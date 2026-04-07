#ifndef PERFECTDICTIONARYGETTERS_TPP
#define PERFECTDICTIONARYGETTERS_TPP

template <typename TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::ContainsKey(const TKey& key) const {
    if (_buckets.size() == 0) return false;
    const PhBucket<TKey, TValue>& bucket = _buckets[_hash(key, _globalSeed, _tableSize)];

    if (bucket.Values.size() == 0) return false;
    const auto& [value, exists] = bucket.Values[_hash(key, bucket.Seed, bucket.Values.size())];
    return exists;
}

template<typename TKey, typename TValue>
size_t PerfectHashDictionary<TKey, TValue>::Count() const {
    return _count;
}

template <typename TKey, typename TValue>
TValue& PerfectHashDictionary<TKey, TValue>::GetValue(const TKey& key) {
    if (_buckets.size() == 0) throw std::out_of_range("Key not found");
    PhBucket<TKey, TValue>& bucket = _buckets[_hash(key, _globalSeed, _tableSize)];

    if (bucket.Values.size() == 0) throw std::out_of_range("Key not found");
    auto& [value, exists] = bucket.Values[_hash(key, bucket.Seed, bucket.Values.size())];

    if (!exists) throw std::out_of_range("Key not found");
    return value;
}

template <typename TKey, typename TValue>
const TValue& PerfectHashDictionary<TKey, TValue>::GetValue(const TKey& key) const {
    if (_buckets.size() == 0) throw std::out_of_range("Key not found");
    const PhBucket<TKey, TValue>& bucket = _buckets[_hash(key, _globalSeed, _tableSize)];

    if (bucket.Values.size() == 0) throw std::out_of_range("Key not found");
    const auto& [value, exists] = bucket.Values[_hash(key, bucket.Seed, bucket.Values.size())];

    if (!exists) throw std::out_of_range("Key not found");
    return value;
}

template <typename TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::TryGetValue(const TKey& key, TValue& value) const {
    if (_buckets.size() == 0) return false;
    const PhBucket<TKey, TValue>& bucket = _buckets[_hash(key, _globalSeed, _tableSize)];

    if (bucket.Values.size() == 0) return false;
    const auto& [dictValue, exists] = bucket.Values[_hash(key, bucket.Seed, bucket.Values.size())];

    if (exists) value = dictValue;
    return exists;
}

#endif // PERFECTDICTIONARYGETTERS_TPP