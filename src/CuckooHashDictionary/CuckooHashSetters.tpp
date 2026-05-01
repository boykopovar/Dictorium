#ifndef CUCKOOHASHSETTERS_TPP
#define CUCKOOHASHSETTERS_TPP

namespace dtr {

template<CHashable TKey, typename TValue>
void CuckooHashDictionary<TKey, TValue>::Add(const TKey &key, const TValue &value) {
    if (_table1.empty()) {
        _table1.assign(DTR_CUCKOO_INIT_CAPACITY, {});
        _table2.assign(DTR_CUCKOO_INIT_CAPACITY, {});
    }
    if (static_cast<double>(_keysCount + 1) / (_table1.size() * 2) > _maxLoadFactor) {
        Rehash();
    }
    while (!_insert(key, value, false)) {
        Rehash();
    }
}

template<CHashable TKey, typename TValue>
void CuckooHashDictionary<TKey, TValue>::InsertOrAssign(const TKey &key, const TValue &value) {
    if (_table1.empty()) {
        _table1.assign(DTR_CUCKOO_INIT_CAPACITY, {});
        _table2.assign(DTR_CUCKOO_INIT_CAPACITY, {});
    }

    if (static_cast<double>(_keysCount + 1) / (_table1.size() * 2) > _maxLoadFactor) {
        Rehash();
    }

    while (!_insert(key, value, true)) {
        Rehash();
    }
}

template<CHashable TKey, typename TValue>
bool CuckooHashDictionary<TKey, TValue>::Remove(const TKey &key) {
    const uint64_t stdHash = std::hash<TKey>{}(key);
    const auto size = _table1.size();

    auto& slot1 = _table1[_hash1(stdHash, size)];
    if (slot1.Exists && slot1.Item.first == key) {
        slot1.Exists = false;
        --_keysCount;
        return true;
    }

    auto& slot2 = _table2[_hash2(stdHash, size)];
    if (slot2.Exists && slot2.Item.first == key) {
        slot2.Exists = false;
        --_keysCount;
        return true;
    }

    return false;
}

template<CHashable TKey, typename TValue>
void CuckooHashDictionary<TKey, TValue>::Clear() {
    _keysCount = 0;

    _seed1 = DTR_CUCKOO_SEED1;
    _seed2 = DTR_CUCKOO_SEED2;

    _table1.clear();
    _table2.clear();
}

}

#endif // CUCKOOHASHSETTERS_TPP