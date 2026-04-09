#ifndef PERFECTDICTIONARYSETTERS_TPP
#define PERFECTDICTIONARYSETTERS_TPP


template <typename TKey, typename TValue>
void PerfectHashDictionary<TKey, TValue>::Add(const TKey& key, const TValue& value) {
    auto stdHash = std::hash<TKey>{}(key);
    const auto globalIndex = _hashRaw(stdHash, _globalSeed, _tableSize);

    auto& bucket = _buckets[globalIndex];
    if (bucket.Size != 0) throw std::invalid_argument("Cannot add this key without rebuild");

    for(size_t i = 0; i < _values.size(); ++i) {
        if (!_values[i].second) {
            bucket.Offset = i;
            bucket.Size = i;
            bucket.Seed = _randomNum();

            _values[i] = {value, true};
            ++_count;
            return;
        }
    }
    throw std::invalid_argument("Cannot add this key without rebuild");
}

template <typename TKey, typename TValue>
void PerfectHashDictionary<TKey, TValue>::InsertOrAssign(const TKey& key, const TValue& value) {
    throw std::logic_error("Not implemented");
}

template<typename TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::Remove(const TKey &key) {
    auto flatIndex = _findIndex(key);
    if (flatIndex == -1) return false;

    auto& slot = _values[flatIndex];
    if (!slot.second) return false;

    slot.second = false;
    return true;
}

template<typename TKey, typename TValue>
void PerfectHashDictionary<TKey, TValue>::Clear() {
    _buckets.clear();
    _values.clear();

    _count = 0;
    _tableSize = 0;
}


#endif // PERFECTDICTIONARYSETTERS_TPP