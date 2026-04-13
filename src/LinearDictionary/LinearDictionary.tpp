#ifndef LINEARDICTIONARY_TPP
#define LINEARDICTIONARY_TPP


template <typename TKey, typename TValue>
bool LinearDictionary<TKey, TValue>::ContainsKey(const TKey& key) const {
    for (auto& p : _dict) {
        if (p.first == key) return true;
    }
    return false;
}


template<typename TKey, typename TValue>
bool LinearDictionary<TKey, TValue>::Remove(const TKey &key) {
    for (auto i = _dict.begin(), e = _dict.end(); i != e; ++i) {
        if (i->first == key) {
            _dict.erase(i);
            return true;
        }
    }
    return false;
}

template<typename TKey, typename TValue>
void LinearDictionary<TKey, TValue>::Clear() {
    _dict.clear();
}

template<typename TKey, typename TValue>
size_t LinearDictionary<TKey, TValue>::Count() const {
    return _dict.size();
}



#endif // LINEARDICTIONARY_TPP