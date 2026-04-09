#ifndef PERFECTDICTIONARYGETTERS_TPP
#define PERFECTDICTIONARYGETTERS_TPP

template <typename TKey, typename TValue>
const std::pair<TValue, bool>* PerfectHashDictionary<TKey, TValue>::_findSlot(const TKey &key) const {
    if (_buckets.size() == 0) return nullptr;
    const PhBucket<TKey, TValue>& bucket = _buckets[_hash(key, _globalSeed, _tableSize)];

    if (bucket.Values.size() == 0) return nullptr;
    return &bucket.Values[_hash(key, bucket.Seed, bucket.Values.size())];
}

template <typename TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::ContainsKey(const TKey& key) const {
    auto* slot = _findSlot(key);
    return slot && slot->second;
}

template<typename TKey, typename TValue>
size_t PerfectHashDictionary<TKey, TValue>::Count() const {
    return _count;
}

template <typename TKey, typename TValue>
TValue& PerfectHashDictionary<TKey, TValue>::GetValue(const TKey& key) {
    auto* slot = const_cast<std::pair<TValue, bool>*>(_findSlot(key));
    if (slot == nullptr || slot->second == false) throw std::out_of_range("Key not found");
    return slot->first;
}

template <typename TKey, typename TValue>
const TValue& PerfectHashDictionary<TKey, TValue>::GetValue(const TKey& key) const {
    auto* slot = _findSlot(key);
    if (slot == nullptr || slot->second == false) throw std::out_of_range("Key not found");
    return slot->first;
}

template <typename TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::TryGetValue(const TKey& key, TValue& value) const {
    auto* slot = _findSlot(key);
    if (!slot || !slot->second) return false;

    value = slot->first;
    return true;
}

#endif // PERFECTDICTIONARYGETTERS_TPP