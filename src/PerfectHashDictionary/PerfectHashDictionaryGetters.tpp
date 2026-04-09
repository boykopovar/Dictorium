#ifndef PERFECTDICTIONARYGETTERS_TPP
#define PERFECTDICTIONARYGETTERS_TPP

template <typename TKey, typename TValue>
uint64_t PerfectHashDictionary<TKey, TValue>::_findIndex(const TKey &key) const {
    auto stdHash = std::hash<TKey>{}(key);
    const auto& bucket = _buckets[_hashRaw(stdHash, _globalSeed, _tableSize)];

    if (bucket.Size == 0) return -1;
    __builtin_prefetch(&_values[bucket.Offset], 0, 1);
    return bucket.Offset + _hashRaw(stdHash, bucket.Seed, bucket.Size);
}

template <typename TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::ContainsKey(const TKey& key) const {
    auto flatIndex = _findIndex(key);
    if (flatIndex == -1) return false;
    return true;
}

template<typename TKey, typename TValue>
size_t PerfectHashDictionary<TKey, TValue>::Count() const {
    return _count;
}

template <typename TKey, typename TValue>
TValue& PerfectHashDictionary<TKey, TValue>::GetValue(const TKey& key) {
    return const_cast<TValue&>(
        static_cast<const PerfectHashDictionary<TKey, TValue>*>(this)->GetValue(key)
    );
}

template <typename TKey, typename TValue>
const TValue& PerfectHashDictionary<TKey, TValue>::GetValue(const TKey& key) const {
    auto flatIndex = _findIndex(key);
    if (flatIndex == -1) throw std::out_of_range("Key not found");

    auto& [value, exists] = _values[flatIndex];
    if (!exists) throw std::out_of_range("Key not found");
    return value;
}

template <typename TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::TryGetValue(const TKey& key, TValue& value) const {
    auto flatIndex = _findIndex(key);
    if (flatIndex == -1) return false;

    auto& [dictValue, exists] = _values[flatIndex];
    if (!exists) return false;
    value = dictValue;
    return true;
}

#endif // PERFECTDICTIONARYGETTERS_TPP