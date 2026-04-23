#ifndef CUCKOOHASHSETTERS_TPP
#define CUCKOOHASHSETTERS_TPP

namespace dtr {

template<CHashable TKey, typename TValue>
void CuckooHashDictionary<TKey, TValue>::Add(const TKey &key, const TValue &value) {
    throw std::runtime_error("Not implemented");
}

template<CHashable TKey, typename TValue>
void CuckooHashDictionary<TKey, TValue>::InsertOrAssign(const TKey &key, const TValue &value) {
    throw std::runtime_error("Not implemented");
}

template<CHashable TKey, typename TValue>
bool CuckooHashDictionary<TKey, TValue>::Remove(const TKey &key) {
    throw std::runtime_error("Not implemented");
}

template<CHashable TKey, typename TValue>
void CuckooHashDictionary<TKey, TValue>::Clear() {
    throw std::runtime_error("Not implemented");
}

}

#endif // CUCKOOHASHSETTERS_TPP