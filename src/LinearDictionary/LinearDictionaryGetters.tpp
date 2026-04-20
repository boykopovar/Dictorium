#ifndef LINEARDICTIONARYGETTERS_TPP
#define LINEARDICTIONARYGETTERS_TPP

namespace dtr
{

template <typename TKey, typename TValue>
TValue& LinearDictionary<TKey, TValue>::GetValue(const TKey& key) {
    for (auto& p : _dict) {
        if (p.first == key) return p.second;
    }
    throw std::out_of_range("Key not found");
}

template <typename TKey, typename TValue>
const TValue& LinearDictionary<TKey, TValue>::GetValue(const TKey& key) const {
    for (auto& p : _dict) {
        if (p.first == key) return p.second;
    }
    throw std::out_of_range("Key not found");
}

template <typename TKey, typename TValue>
bool LinearDictionary<TKey, TValue>::TryGetValue(const TKey& key, TValue& value) const {
    for (auto& p : _dict) {
        if (p.first == key) {
            value = p.second;
            return true;
        }
    }
    return false;
}

}
#endif //LINEARDICTIONARYGETTERS_TPP
