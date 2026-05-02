#ifndef PERFECTDICTIONARYGETTERS_TPP
#define PERFECTDICTIONARYGETTERS_TPP

namespace dtr
{

template <CHashable TKey, typename TValue>
uint64_t PerfectHashDictionary<TKey, TValue>::_findIndex(const TKey &key) const {
    auto stdHash = std::hash<TKey>{}(key);
    const auto& bucket = _buckets[_hashRaw(stdHash, _globalSeed, _tableSize)];

    if (bucket.Size == 0) return -1;
    DTR_PREFETCH(&_values[bucket.Offset]);
    return bucket.Offset + _hashRaw(stdHash, bucket.Seed, bucket.Size);
}

template <CHashable TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::ContainsKey(const TKey& key) const {
    auto flatIndex = _findIndex(key);
    if (flatIndex == -1) return false;

    auto& slot = _values[flatIndex];
    if (!slot.Exists) return false;
    return slot.Item.first == key;
}

template<CHashable TKey, typename TValue>
size_t PerfectHashDictionary<TKey, TValue>::Count() const {
    return _count;
}

template <CHashable TKey, typename TValue>
TValue& PerfectHashDictionary<TKey, TValue>::GetValue(const TKey& key) {
    return const_cast<TValue&>(
        static_cast<const PerfectHashDictionary<TKey, TValue>*>(this)->GetValue(key)
    );
}

template <CHashable TKey, typename TValue>
const TValue& PerfectHashDictionary<TKey, TValue>::GetValue(const TKey& key) const {
    auto& slot = _getExistedSlot(key);
    if (!slot.Exists) throw std::out_of_range("Key not found");
    return slot.Item.second;
}

template <CHashable TKey, typename TValue>
const TValue& PerfectHashDictionary<TKey, TValue>::GetValidatedValue(const TKey& key) const {
    auto& slot = _getExistedSlot(key);
    if(slot.Item.first!=key) throw std::out_of_range("Key not found");
    return slot.Item.second;
}

template <CHashable TKey, typename TValue>
TValue& PerfectHashDictionary<TKey, TValue>::GetValidatedValue(const TKey& key) {
    return const_cast<TValue&>(
        static_cast<const PerfectHashDictionary<TKey, TValue>*>(this)->GetValidatedValue(key)
    );
}

template <CHashable TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::TryGetValue(const TKey& key, TValue& value) const {
    auto flatIndex = _findIndex(key);
    if (flatIndex == -1) return false;

    auto& slot = _values[flatIndex];
    if (!slot.Exists) return false;
    value = slot.Item.second;
    return true;
}

template <CHashable TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::TryGetValidatedValue(const TKey& key, TValue& value) const {
    auto flatIndex = _findIndex(key);
    if (flatIndex == -1) return false;

    auto& slot = _values[flatIndex];
    if (!slot.Exists || !(slot.Item.first == key)) return false;
    value = slot.Item.second;
    return true;
}

}
#endif // PERFECTDICTIONARYGETTERS_TPP