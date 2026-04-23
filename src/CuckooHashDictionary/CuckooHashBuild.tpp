#ifndef CUCKOOHASHBUILD_TPP
#define CUCKOOHASHBUILD_TPP

namespace dtr {

template <CHashable TKey, typename TValue>
template<CPairIterator<TKey, TValue> TIter>
void CuckooHashDictionary<TKey, TValue>::_build(TIter begin, TIter end, size_t size) {
    throw std::runtime_error("Not implemented");
}
    
}

#endif // CUCKOOHASHBUILD_TPP