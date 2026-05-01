#ifndef CUCKOOHASHBUILD_TPP
#define CUCKOOHASHBUILD_TPP

namespace dtr {

template <CHashable TKey, typename TValue>
template<CPairIterator<TKey, TValue> TIter>
void CuckooHashDictionary<TKey, TValue>::_build(TIter begin, TIter end, size_t size) {
    const auto initSize = std::max(DTR_CUCKOO_INIT_CAPACITY, size);
    this->_table1.assign(initSize, {});
    this->_table2.assign(initSize , {});

    _keysCount = 0;
    for (auto it = begin; it != end; ++it) {
        Add(it->first, it->second);
    }
}

template <CHashable TKey, typename TValue>
void CuckooHashDictionary<TKey, TValue>::Rehash() {
    Rehash(_table1.size() * 2);
}

template <CHashable TKey, typename TValue>
void CuckooHashDictionary<TKey, TValue>::Rehash(size_t newTableSize) {
    std::vector<std::pair<TKey, TValue>> items;
    items.reserve(_keysCount);

    for (auto& slot : _table1) {
        if (slot.Exists) items.push_back(slot.Item);
    }

    for (auto& slot : _table2) {
        if (slot.Exists) items.push_back(slot.Item);
    }

    _seed1 = (_seed1 ^ (_seed1 >> 30)) * DTR_CUCKOO_REHASH_SALT1;
    _seed2 = (_seed2 ^ (_seed2 >> 27)) * DTR_CUCKOO_REHASH_SALT2;

    _table1.assign(newTableSize, {});
    _table2.assign(newTableSize, {});

    bool done = false;
    for (auto& [k, v] : items) {
        if (done) return;
        while (!_insert(k, v, false)) {
            _seed1 = (_seed1 ^ (_seed1 >> 30)) * DTR_CUCKOO_REHASH_SALT1;
            _seed2 = (_seed2 ^ (_seed2 >> 27)) * DTR_CUCKOO_REHASH_SALT2;

            _table1.assign(newTableSize, {});
            _table2.assign(newTableSize, {});

            _keysCount = 0;
            for (auto& [k2, v2] : items) _insert(k2, v2, false);
            done = true;
            break;
        }
    }
}


template <CHashable TKey, typename TValue>
bool CuckooHashDictionary<TKey, TValue>::_insert(TKey key, TValue value, const bool allowOverwrite) {
    const auto maxKicks = _getMaxKicks();
    const auto size = _table1.size();
    uint64_t stdHash = std::hash<TKey>{}(key);

    for (size_t i = 0; i < maxKicks; ++i) {
        auto index1 = _hash1(stdHash, size);
        auto& slot1 = _table1[index1];

        if (slot1.Exists && slot1.Item.first == key) {
            if (allowOverwrite) {
                slot1.Item.second = value;
                return true;
            }
            throw std::runtime_error("Key already exists");
        }
        if (!slot1.Exists) {
            slot1.Item.first = key;
            slot1.Item.second = value;

            slot1.Exists = true;
            ++_keysCount;
            return true;
        }

        std::swap(key, slot1.Item.first);
        std::swap(value, slot1.Item.second);
        stdHash = std::hash<TKey>{}(key);

        auto index2 = _hash2(stdHash, size);
        auto& slot2 = _table2[index2];

        if (slot2.Exists && slot2.Item.first == key) {
            if (allowOverwrite) {
                slot2.Item.second = value;
                return true;
            }
            throw std::runtime_error("Key already exists");
        }
        if (!slot2.Exists) {
            slot2.Item.first = key;
            slot2.Item.second = value;

            slot2.Exists = true;
            ++_keysCount;
            return true;
        }

        std::swap(key, slot2.Item.first);
        std::swap(value, slot2.Item.second);
        stdHash = std::hash<TKey>{}(key);
    }
    return false;
}

    
}

#endif // CUCKOOHASHBUILD_TPP