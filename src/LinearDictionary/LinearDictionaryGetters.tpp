#ifndef LINEARDICTIONARYGETTERS_TPP
#define LINEARDICTIONARYGETTERS_TPP

template <class TKey, class TValue>
TValue& LinearDictionary<TKey, TValue>::GetValue(const TKey& key) {
    for (auto& p : _dict) {
        if (p.first == key) return p.second;
    }
    throw std::out_of_range("Key not found");
}

template <class TKey, class TValue>
const TValue& LinearDictionary<TKey, TValue>::GetValue(const TKey& key) const {
    for (auto& p : _dict) {
        if (p.first == key) return p.second;
    }
    throw std::out_of_range("Key not found");
}

template <class TKey, class TValue>
bool LinearDictionary<TKey, TValue>::TryGetValue(const TKey& key, TValue& value) const {
    for (auto& p : _dict) {
        if (p.first == key) {
            value = p.second;
            return true;
        }
    }
    return false;
}

#endif //LINEARDICTIONARYGETTERS_TPP
