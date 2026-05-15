#ifndef SKIPLISTDICTIONARY_TPP
#define SKIPLISTDICTIONARY_TPP

namespace dtr {

template <typename TKey, typename TValue>
SkipListDictionary<TKey, TValue>::SkipListDictionary() {
    std::random_device rd;
    _rngState = (static_cast<uint64_t>(rd()) << 32) ^ static_cast<uint64_t>(rd());
    if (_rngState == 0) {
        _rngState = 0x9e3779b97f4a7c15ULL;
    }
}

template <typename TKey, typename TValue>
template <typename TIter>
SkipListDictionary<TKey, TValue>::SkipListDictionary(TIter first, TIter last)
    : SkipListDictionary() {
    for (; first != last; ++first) {
        InsertOrAssign(first->first, first->second);
    }
}

template <typename TKey, typename TValue>
uint64_t SkipListDictionary<TKey, TValue>::_nextRandom() const {
    uint64_t x = _rngState;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    _rngState = x;
    return x * 0x2545F4914F6CDD1DULL;
}

template <typename TKey, typename TValue>
uint8_t SkipListDictionary<TKey, TValue>::_randomLevel() const {
    const uint64_t bits = _nextRandom();
    int level = 0;
    uint64_t mask = 1ULL;

    while (level < DTR_SKIPLIST_MAX_LEVEL && (bits & mask)) {
        ++level;
        mask <<= 1;
    }
    return static_cast<uint8_t>(level);
}

template <typename TKey, typename TValue>
typename SkipListDictionary<TKey, TValue>::Node* SkipListDictionary<TKey, TValue>::_allocNode(
    const TKey& key, const TValue& value, uint8_t nodeLevel) {
    Node* node = nullptr;

    if (_freeList) {
        node = _freeList;
        _freeList = _freeList->forward[0];
    } else {
        _pool.emplace_back(key, value, nodeLevel);
        node = &_pool.back();
    }

    node->data = {key, value};
    node->level = nodeLevel;
    node->forward.fill(nullptr);
    return node;
}

template <typename TKey, typename TValue>
void SkipListDictionary<TKey, TValue>::_freeNode(Node* node) {
    node->forward[0] = _freeList;
    _freeList = node;
}

template <typename TKey, typename TValue>
typename SkipListDictionary<TKey, TValue>::Node* SkipListDictionary<TKey, TValue>::_search(
    const TKey& key, UpdatePath& update) const {
    update.fill(nullptr);

    if (_level < 0) {
        return nullptr;
    }

    Node* predecessor = nullptr;

    for (int level = _level; level >= 0; --level) {
        Node* next = predecessor ? predecessor->forward[level] : _head[level];

        while (next) {
            DTR_SKIPLIST_PREFETCH(next->forward[level]);
            if (next->data.first < key) {
                predecessor = next;
                next = predecessor->forward[level];
            } else {
                break;
            }
        }

        update[static_cast<size_t>(level)] = predecessor;
    }

    Node* candidate = predecessor ? predecessor->forward[0] : _head[0];
    if (candidate && candidate->data.first == key) {
        return candidate;
    }
    return nullptr;
}

template <typename TKey, typename TValue>
void SkipListDictionary<TKey, TValue>::_linkNode(
    Node* node, const UpdatePath& update, uint8_t nodeLevel) {
    for (uint8_t level = 0; level <= nodeLevel; ++level) {
        Node* pred = update[level];
        node->forward[level] = pred ? pred->forward[level] : _head[level];
        if (pred) {
            pred->forward[level] = node;
        } else {
            _head[level] = node;
        }
    }
}

template <typename TKey, typename TValue>
void SkipListDictionary<TKey, TValue>::_unlinkNode(Node* node, const UpdatePath& update) {
    for (uint8_t level = 0; level <= node->level; ++level) {
        Node* pred = update[level];
        if (pred) {
            pred->forward[level] = node->forward[level];
        } else {
            _head[level] = node->forward[level];
        }
    }
}

template <typename TKey, typename TValue>
void SkipListDictionary<TKey, TValue>::_shrinkLevel() {
    while (_level > 0 && _head[_level] == nullptr) {
        --_level;
    }
    if (_level >= 0 && _head[0] == nullptr) {
        _level = -1;
    }
}

template <typename TKey, typename TValue>
bool SkipListDictionary<TKey, TValue>::ContainsKey(const TKey& key) const {
    UpdatePath update;
    return _search(key, update) != nullptr;
}

template <typename TKey, typename TValue>
bool SkipListDictionary<TKey, TValue>::TryGetValue(const TKey& key, TValue& value) const {
    UpdatePath update;
    const Node* node = _search(key, update);
    if (!node) {
        return false;
    }
    value = node->data.second;
    return true;
}

template <typename TKey, typename TValue>
void SkipListDictionary<TKey, TValue>::Add(const TKey& key, const TValue& value) {
    UpdatePath update;
    if (_search(key, update)) {
        throw std::invalid_argument("Element exists");
    }

    const uint8_t nodeLevel = _randomLevel();
    if (static_cast<int>(nodeLevel) > _level) {
        for (int level = _level + 1; level <= static_cast<int>(nodeLevel); ++level) {
            update[static_cast<size_t>(level)] = nullptr;
        }
        _level = static_cast<int>(nodeLevel);
    }

    Node* node = _allocNode(key, value, nodeLevel);
    _linkNode(node, update, nodeLevel);
    ++_count;
}

template <typename TKey, typename TValue>
void SkipListDictionary<TKey, TValue>::InsertOrAssign(const TKey& key, const TValue& value) {
    UpdatePath update;
    Node* found = _search(key, update);

    if (found) {
        found->data.second = value;
        return;
    }

    const uint8_t nodeLevel = _randomLevel();
    if (static_cast<int>(nodeLevel) > _level) {
        for (int level = _level + 1; level <= static_cast<int>(nodeLevel); ++level) {
            update[static_cast<size_t>(level)] = nullptr;
        }
        _level = static_cast<int>(nodeLevel);
    }

    Node* node = _allocNode(key, value, nodeLevel);
    _linkNode(node, update, nodeLevel);
    ++_count;
}

template <typename TKey, typename TValue>
bool SkipListDictionary<TKey, TValue>::Remove(const TKey& key) {
    if (_level < 0) {
        return false;
    }

    UpdatePath update;
    Node* node = _search(key, update);
    if (!node) {
        return false;
    }

    _unlinkNode(node, update);
    _freeNode(node);
    _shrinkLevel();
    --_count;
    return true;
}

template <typename TKey, typename TValue>
void SkipListDictionary<TKey, TValue>::Clear() {
    _head.fill(nullptr);
    _level = -1;
    _count = 0;
    _pool.clear();
    _pool.shrink_to_fit();
    _freeList = nullptr;
}

template <typename TKey, typename TValue>
size_t SkipListDictionary<TKey, TValue>::Count() const {
    return _count;
}

template <typename TKey, typename TValue>
TValue& SkipListDictionary<TKey, TValue>::GetValue(const TKey& key) {
    UpdatePath update;
    Node* node = _search(key, update);
    if (!node) {
        throw std::out_of_range("Key not found");
    }
    return node->data.second;
}

template <typename TKey, typename TValue>
const TValue& SkipListDictionary<TKey, TValue>::GetValue(const TKey& key) const {
    UpdatePath update;
    const Node* node = _search(key, update);
    if (!node) {
        throw std::out_of_range("Key not found");
    }
    return node->data.second;
}

} 

#endif // SKIPLISTDICTIONARY_TPP
