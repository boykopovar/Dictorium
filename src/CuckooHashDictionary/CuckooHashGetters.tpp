#ifndef CUCKOOHASHGETTERS_TPP
#define CUCKOOHASHGETTERS_TPP

namespace dtr{

template<CHashable TKey, typename TValue>
bool CuckooHashDictionary<TKey, TValue>::ContainsKey(const TKey &key) const {
    const uint64_t stdHash = std::hash<TKey>{}(key);
    const auto size = _table1.size();

    const auto& slot1 = _table1[_hash1(stdHash, size)];
    if (slot1.Exists && slot1.Item.first == key) return true;

    const auto& slot2 = _table2[_hash2(stdHash, size)];
    if (slot2.Exists && slot2.Item.first == key) return true;

    return false;
}

template<CHashable TKey, typename TValue>
bool CuckooHashDictionary<TKey, TValue>::TryGetValue(const TKey &key, TValue &value) const {
    const uint64_t stdHash = std::hash<TKey>{}(key);
    const auto size = _table1.size();

    const auto& slot1 = _table1[_hash1(stdHash, size)];
    if (slot1.Exists && slot1.Item.first == key) {
        value = slot1.Item.second;
        return true;
    }

    const auto& slot2 = _table2[_hash2(stdHash, size)];
    if (slot2.Exists && slot2.Item.first == key) {
        value = slot2.Item.second;
        return true;
    }

    return false;
}

template<CHashable TKey, typename TValue>
size_t CuckooHashDictionary<TKey, TValue>::Count() const {
    return _keysCount;
}

template<CHashable TKey, typename TValue>
TValue& CuckooHashDictionary<TKey, TValue>::GetValue(const TKey &key) {
    return const_cast<TValue&>(
        static_cast<const CuckooHashDictionary<TKey, TValue>*>(this)->GetValue(key)
    );
}

template<CHashable TKey, typename TValue>
const TValue& CuckooHashDictionary<TKey, TValue>::GetValue(const TKey &key) const {
    const uint64_t stdHash = std::hash<TKey>{}(key);
    const auto size = _table1.size();

    const auto& slot1 = _table1[_hash1(stdHash, size)];
    if (slot1.Exists && slot1.Item.first == key) return slot1.Item.second;

    const auto& slot2 = _table2[_hash2(stdHash, size)];
    if (slot2.Exists && slot2.Item.first == key) return slot2.Item.second;

    throw std::out_of_range("Key not found");
}

}
#endif // CUCKOOHASHGETTERS_TPP