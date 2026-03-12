#ifndef LINEARDICTIONARYSETTERS_TPP
#define LINEARDICTIONARYSETTERS_TPP

template <typename TKey, typename TValue>
void LinearDictionary<TKey, TValue>::Add(const TKey& key, const TValue& value) {
    if (ContainsKey(key)) throw std::invalid_argument("Key already exists");
    _dict.emplace_back(key, value);
}

template <typename TKey, typename TValue>
void LinearDictionary<TKey, TValue>::InsertOrAssign(const TKey& key, const TValue& value) {
    for (auto& p : _dict) {
        if (p.first == key) {
            p.second = value;
            return;
        }
    }
    _dict.emplace_back(key, value);
}

#endif // LINEARDICTIONARYSETTERS_TPP
