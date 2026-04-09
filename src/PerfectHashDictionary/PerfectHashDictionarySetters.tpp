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
    auto* slot = const_cast<std::pair<TValue, bool>*>(_findSlot(key));;
    if (!slot || !slot->second) return false;

    slot->second = false;
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