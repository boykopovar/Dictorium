#ifndef TREAPDICTIONARY_TPP
#define TREAPDICTIONARY_TPP

namespace dtr {

template <typename TKey, typename TValue>
TreapDictionary<TKey, TValue>::TreapDictionary() {
    std::random_device rd;
    _rngState = (static_cast<uint64_t>(rd()) << 32) ^ static_cast<uint64_t>(rd());
    if (_rngState == 0) {
        _rngState = 0x9e3779b97f4a7c15ULL;
    }
}

template <typename TKey, typename TValue>
template <typename TIter>
TreapDictionary<TKey, TValue>::TreapDictionary(TIter first, TIter last)
    : TreapDictionary() {
    for (; first != last; ++first) {
        InsertOrAssign(first->first, first->second);
    }
}

template <typename TKey, typename TValue>
TreapDictionary<TKey, TValue>::~TreapDictionary() {
    Clear();
}

template <typename TKey, typename TValue>
uint32_t TreapDictionary<TKey, TValue>::_randomPriority() const {
    uint64_t x = _rngState;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    _rngState = x;
    x *= 0x2545F4914F6CDD1DULL;
    return static_cast<uint32_t>(x ^ (x >> 32));
}

template <typename TKey, typename TValue>
typename TreapDictionary<TKey, TValue>::Node* TreapDictionary<TKey, TValue>::_initNode(
    const TKey& key, const TValue& value) {
    return new Node{{key, value}, _randomPriority(), nullptr, nullptr};
}

template <typename TKey, typename TValue>
void TreapDictionary<TKey, TValue>::_clear(Node* node) {
    if (!node) {
        return;
    }
    _clear(node->left);
    _clear(node->right);
    delete node;
}

template <typename TKey, typename TValue>
typename TreapDictionary<TKey, TValue>::Node* TreapDictionary<TKey, TValue>::_rotateLeft(Node* node) {
    Node* right = node->right;
    node->right = right->left;
    right->left = node;
    return right;
}

template <typename TKey, typename TValue>
typename TreapDictionary<TKey, TValue>::Node* TreapDictionary<TKey, TValue>::_rotateRight(Node* node) {
    Node* left = node->left;
    node->left = left->right;
    left->right = node;
    return left;
}

template <typename TKey, typename TValue>
typename TreapDictionary<TKey, TValue>::Node* TreapDictionary<TKey, TValue>::_find(
    Node* node, const TKey& key) const {
    while (node) {
        if (key < node->data.first) {
            node = node->left;
        } else if (node->data.first < key) {
            node = node->right;
        } else {
            return node;
        }
    }
    return nullptr;
}

template <typename TKey, typename TValue>
typename TreapDictionary<TKey, TValue>::Node* TreapDictionary<TKey, TValue>::_insert(
    Node* node, const TKey& key, const TValue& value, bool& inserted) {
    if (!node) {
        inserted = true;
        return _initNode(key, value);
    }

    if (key < node->data.first) {
        node->left = _insert(node->left, key, value, inserted);
        if (inserted && node->left->priority > node->priority) {
            node = _rotateRight(node);
        }
    } else if (node->data.first < key) {
        node->right = _insert(node->right, key, value, inserted);
        if (inserted && node->right->priority > node->priority) {
            node = _rotateLeft(node);
        }
    }

    return node;
}

template <typename TKey, typename TValue>
typename TreapDictionary<TKey, TValue>::Node* TreapDictionary<TKey, TValue>::_remove(
    Node* node, const TKey& key, bool& removed) {
    if (!node) {
        return nullptr;
    }

    if (key < node->data.first) {
        node->left = _remove(node->left, key, removed);
        return node;
    }

    if (node->data.first < key) {
        node->right = _remove(node->right, key, removed);
        return node;
    }

    removed = true;

    if (!node->left) {
        Node* right = node->right;
        delete node;
        return right;
    }

    if (!node->right) {
        Node* left = node->left;
        delete node;
        return left;
    }

    if (node->left->priority > node->right->priority) {
        node = _rotateRight(node);
        node->right = _remove(node->right, key, removed);
    } else {
        node = _rotateLeft(node);
        node->left = _remove(node->left, key, removed);
    }

    return node;
}

template <typename TKey, typename TValue>
bool TreapDictionary<TKey, TValue>::ContainsKey(const TKey& key) const {
    return _find(_root, key) != nullptr;
}

template <typename TKey, typename TValue>
bool TreapDictionary<TKey, TValue>::TryGetValue(const TKey& key, TValue& value) const {
    const Node* node = _find(_root, key);
    if (!node) {
        return false;
    }
    value = node->data.second;
    return true;
}

template <typename TKey, typename TValue>
void TreapDictionary<TKey, TValue>::Add(const TKey& key, const TValue& value) {
    if (_find(_root, key)) {
        throw std::invalid_argument("Element exists");
    }

    bool inserted = false;
    _root = _insert(_root, key, value, inserted);
    if (inserted) {
        ++_count;
    }
}

template <typename TKey, typename TValue>
void TreapDictionary<TKey, TValue>::InsertOrAssign(const TKey& key, const TValue& value) {
    Node* node = _find(_root, key);
    if (node) {
        node->data.second = value;
        return;
    }

    bool inserted = false;
    _root = _insert(_root, key, value, inserted);
    if (inserted) {
        ++_count;
    }
}

template <typename TKey, typename TValue>
bool TreapDictionary<TKey, TValue>::Remove(const TKey& key) {
    bool removed = false;
    _root = _remove(_root, key, removed);
    if (removed) {
        --_count;
    }
    return removed;
}

template <typename TKey, typename TValue>
void TreapDictionary<TKey, TValue>::Clear() {
    _clear(_root);
    _root = nullptr;
    _count = 0;
}

template <typename TKey, typename TValue>
size_t TreapDictionary<TKey, TValue>::Count() const {
    return _count;
}

template <typename TKey, typename TValue>
TValue& TreapDictionary<TKey, TValue>::GetValue(const TKey& key) {
    Node* node = _find(_root, key);
    if (!node) {
        throw std::out_of_range("Key not found");
    }
    return node->data.second;
}

template <typename TKey, typename TValue>
const TValue& TreapDictionary<TKey, TValue>::GetValue(const TKey& key) const {
    const Node* node = _find(_root, key);
    if (!node) {
        throw std::out_of_range("Key not found");
    }
    return node->data.second;
}

}

#endif // TREAPDICTIONARY_TPP
