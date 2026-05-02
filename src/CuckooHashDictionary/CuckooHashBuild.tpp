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

    while (true) {
        _seed1 = (_seed1 ^ (_seed1 >> 30)) * DTR_CUCKOO_REHASH_SALT1;
        _seed2 = (_seed2 ^ (_seed2 >> 27)) * DTR_CUCKOO_REHASH_SALT2;

        _table1.assign(newTableSize, {});
        _table2.assign(newTableSize, {});
        _keysCount = 0;

        bool success =  true;
        for (auto& [k, v] : items) {
            if (!_insert(k, v, false)) {
                success =  false;
                break;
            }
        }
        if (success) { return; }
    }
}


template <CHashable TKey, typename TValue>
bool CuckooHashDictionary<TKey, TValue>::_insert(TKey key, TValue value, const bool allowOverwrite) {
    const auto size = _table1.size();
    uint64_t stdHash = std::hash<TKey>{}(key);

    auto& s1 = _table1[_hash1(stdHash, size)];
    auto& s2 = _table2[_hash2(stdHash, size)];

    if (s1.Exists && s1.Item.first == key) {
        if (allowOverwrite) {
            s1.Item.second = value;
            return true;
        }
        throw std::invalid_argument("Key already exists");
    }

    if (s2.Exists && s2.Item.first == key) {
        if (allowOverwrite) {
            s2.Item.second = value;
            return true;
        }
        throw std::invalid_argument("Key already exists");
    }

    const auto maxKicks = _getMaxKicks();
    std::vector<Displacement<TKey, TValue>> log;
    log.reserve(maxKicks);

    TKey k = key;
    TValue v = value;
    uint64_t hashValue = stdHash;

    for (size_t i = 0; i < maxKicks; i++) {
        size_t index1 = _hash1(hashValue, size);
        auto& slot1 = _table1[index1];
        if (!slot1.Exists) {
            slot1.Exists = true;
            slot1.Item.first = k;
            slot1.Item.second = v;

            ++_keysCount;
            return true;
        }

        log.push_back({1, index1, slot1.Item});
        std::swap(k, slot1.Item.first);
        std::swap(v, slot1.Item.second);
        hashValue = std::hash<TKey>{}(k);

        size_t index2 = _hash2(hashValue, size);
        auto& slot2 = _table2[index2];
        if (!slot2.Exists) {
            slot2.Exists = true;
            slot2.Item.first = k;
            slot2.Item.second = v;

            ++_keysCount;
            return true;
        }

        log.push_back({2, index2, slot2.Item});
        std::swap(k, slot2.Item.first);
        std::swap(v, slot2.Item.second);
        hashValue = std::hash<TKey>{}(k);
    }

    for (long long int i = static_cast<long long int>(log.size()) -1; i>=0; --i) {
        auto& dplace = log[i];
        if (dplace.Table == 1) _table1[dplace.Index].Item = dplace.Old;
        else _table2[dplace.Index].Item = dplace.Old;
    }
    return false;

}

    
}

#endif // CUCKOOHASHBUILD_TPP