#ifndef CUCKOOHASHGETTERS_TPP
#define CUCKOOHASHGETTERS_TPP

namespace dtr{

template<CHashable TKey, typename TValue>
bool CuckooHashDictionary<TKey, TValue>::ContainsKey(const TKey &key) const {
    throw std::runtime_error("Not implemented");
}

template<CHashable TKey, typename TValue>
bool CuckooHashDictionary<TKey, TValue>::TryGetValue(const TKey &key, TValue &value) const {
    throw std::runtime_error("Not implemented");
}

template<CHashable TKey, typename TValue>
size_t CuckooHashDictionary<TKey, TValue>::Count() const {
    throw std::runtime_error("Not implemented");
}

template<CHashable TKey, typename TValue>
TValue& CuckooHashDictionary<TKey, TValue>::GetValue(const TKey &key) {
    throw std::runtime_error("Not implemented");
}

template<CHashable TKey, typename TValue>
const TValue & CuckooHashDictionary<TKey, TValue>::GetValue(const TKey &key) const {
    throw std::runtime_error("Not implemented");
}

}
#endif // CUCKOOHASHGETTERS_TPP