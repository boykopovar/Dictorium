#ifndef PERFECTDICTIONARYSETTERS_TPP
#define PERFECTDICTIONARYSETTERS_TPP


template <typename TKey, typename TValue>
void PerfectHashDictionary<TKey, TValue>::Add(const TKey& key, const TValue& value) {
    throw std::logic_error("Not implemented");
}

template <typename TKey, typename TValue>
void PerfectHashDictionary<TKey, TValue>::InsertOrAssign(const TKey& key, const TValue& value) {
    throw std::logic_error("Not implemented");
}

template<typename TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::Remove(const TKey &key) {
    if (_buckets.size() == 0) return false;
    PhBucket<TKey, TValue>& bucket = _buckets[_hash(key, _globalSeed, _tableSize)];

    if (bucket.Values.size() == 0) return false;
    auto& [value, exists] = bucket.Values[_hash(key, bucket.Seed, bucket.Values.size())];

    if (!exists) return false;
    exists = false;

    --_count;
    return true;
}

template<typename TKey, typename TValue>
void PerfectHashDictionary<TKey, TValue>::Clear() {
    for (auto& bucket : _buckets) {
        bucket.Values.clear();
    }
    _buckets.clear();
    _count = 0;
    _tableSize = 0;
}


#endif // PERFECTDICTIONARYSETTERS_TPP