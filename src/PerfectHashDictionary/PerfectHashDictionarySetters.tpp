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
    auto flatIndex = _findIndex(key);
    if (flatIndex == -1) return false;

    auto& slot = _values[flatIndex];
    if (!slot.Exists) return false;

    slot.Exists = false;
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