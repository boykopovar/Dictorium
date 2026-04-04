#ifndef PERFECTDICTIONARYGETTERS_TPP
#define PERFECTDICTIONARYGETTERS_TPP


template <typename TKey, typename TValue>
TValue& PerfectHashDictionary<TKey, TValue>::GetValue(const TKey& key) {
    throw std::logic_error("Not implemented");
}

template <typename TKey, typename TValue>
const TValue& PerfectHashDictionary<TKey, TValue>::GetValue(const TKey& key) const {
    throw std::logic_error("Not implemented");
}

template <typename TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::TryGetValue(const TKey& key, TValue& value) const {
    throw std::logic_error("Not implemented");
}



#endif // PERFECTDICTIONARYGETTERS_TPP