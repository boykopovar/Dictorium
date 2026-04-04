#ifndef PERFECTDICTIONARY_TPP
#define PERFECTDICTIONARY_TPP


template <typename TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::ContainsKey(const TKey& key) const {
    throw std::logic_error("Not implemented");
}


template<typename TKey, typename TValue>
bool PerfectHashDictionary<TKey, TValue>::Remove(const TKey &key) {
    throw std::logic_error("Not implemented");
}

template<typename TKey, typename TValue>
void PerfectHashDictionary<TKey, TValue>::Clear() {
    throw std::logic_error("Not implemented");
}

template<typename TKey, typename TValue>
size_t PerfectHashDictionary<TKey, TValue>::Count() const {
    throw std::logic_error("Not implemented");
}



#endif // PERFECTDICTIONARY_TPP