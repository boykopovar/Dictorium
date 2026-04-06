#ifndef PERFECTDICTIONARYGETTERS_TPP
#define PERFECTDICTIONARYGETTERS_TPP


template <typename TKey, typename TValue>
TValue& PerfectHashDictionary<TKey, TValue>::GetValue(const TKey& key) {
    PhBucket<TKey, TValue>& bucket = _buckets[_hash(key, _globalSeed, _count)];
    auto& [value, exists] = bucket.Values[_hash(key, bucket.Seed, bucket.Values.size())];

    if (!exists) throw std::out_of_range("Key not found");
    return value;
}

template <typename TKey, typename TValue>
const TValue& PerfectHashDictionary<TKey, TValue>::GetValue(const TKey& key) const {
    const PhBucket<TKey, TValue>& bucket = _buckets[_hash(key, _globalSeed, _count)];
    const auto& [value, exists] = bucket.Values[_hash(key, bucket.Seed, bucket.Values.size())];

    if (exists) throw std::out_of_range("Key not found");
    return value;
}

template <typename TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::TryGetValue(const TKey& key, TValue& value) const {
    throw std::logic_error("Not implemented");
}



#endif // PERFECTDICTIONARYGETTERS_TPP