#ifndef CUCKOOHASHBUILD_TPP
#define CUCKOOHASHBUILD_TPP

namespace dtr {

template <CHashable TKey, typename TValue>
template<CPairIterator<TKey, TValue> TIter>
void CuckooHashDictionary<TKey, TValue>::_build(TIter begin, TIter end, size_t size) {
    this->_table1.assign(size, {});
    this->_table2.assign(size, {});

    _keysCount = 0;
    for (auto it = begin; it != end; ++it) {
        Add(it->first, it->second);
    }
}

template <CHashable TKey, typename TValue>
void CuckooHashDictionary<TKey, TValue>::_rehash() {
    ++_rehashCount;
    _seed1 = (_seed1 ^ (_seed1 >> 30)) * DTR_CUCKOO_REHASH_SALT1;
    _seed2 = (_seed2 ^ (_seed2 >> 27)) * DTR_CUCKOO_REHASH_SALT2;
}
    
}

#endif // CUCKOOHASHBUILD_TPP