#ifndef CUCKOOHASHDICTIONARY_H
#define CUCKOOHASHDICTIONARY_H

#define DTR_CUCKOO_MAX_LOAD_FACTOR 0.5
#define DTR_CUCKOO_INIT_CAPACITY 8

#define DTR_CUCKOO_SEED1 0x9e3779b97f4a7c15ULL
#define DTR_CUCKOO_SEED2 0x6c62272e07bb0142ULL

#define DTR_CUCKOO_REHASH_SALT1 0xbf58476d1ce4e5b9ULL
#define DTR_CUCKOO_REHASH_SALT2 0x94d049bb133111ebULL

#include <vector>
#include <stdexcept>
#include "Dictorium/Contracts/Concepts.h"

namespace dtr {

template<typename TKey, typename TValue>
class IDictionary;

template<CHashable TKey, typename TValue>
class CuckooHashDictionary : public IDictionary<TKey, TValue> {
public:

    CuckooHashDictionary(std::initializer_list<std::pair<TKey, TValue>> init) {
        for (auto i = init.begin(), e = init.end(); i != e; ++i) {
            (*this).Add(i->first, i->second);
        }
    }

    bool ContainsKey(const TKey& key) const override;

    bool TryGetValue(const TKey& key, TValue& value) const override;

    void Add(const TKey& key, const TValue& value) override;

    void InsertOrAssign(const TKey& key, const TValue& value) override;

    bool Remove(const TKey& key) override;

    void Clear() override;

    [[nodiscard]] size_t Count() const override;

    TValue& GetValue(const TKey& key) override;

    const TValue& GetValue(const TKey& key) const override;

    std::ostream& WriteToStream(std::ostream& os) const override {
        if constexpr (!CStreamWritable<TValue> && ! CStreamWritable<TKey>) {
            return os << "<class 'LinearDictionary' TKey=" << typeid(TKey).name() << ", TValue=" << typeid(TValue).name() << '>';
        }
        else {
            throw std::runtime_error("Not implemented");
            // return this->_writeItems(os, *this);
        }
    }

private:
    size_t _rehashCount = 0;
    size_t _keysCount = 0;

    size_t _maxLoadFactor = DTR_CUCKOO_MAX_LOAD_FACTOR;
    uint64_t _seed1 = DTR_CUCKOO_SEED1;
    uint64_t _seed2 = DTR_CUCKOO_SEED2;

    template<CPairIterator<TKey, TValue> TIter>
    void _build(TIter begin, TIter end, size_t size);

    void _rehash();

    [[nodiscard]] size_t _hash1(const uint64_t stdHash, const size_t tableSize) const {
        return FastRange(stdHash * _seed1, tableSize);
    }

    [[nodiscard]] size_t _hash2(const uint64_t stdHash, const size_t tableSize) const {
        const uint64_t hash = (stdHash ^ (stdHash >> 30)) * _seed2;
        return FastRange(hash, tableSize);
    }

    std::vector<DictSlot<TKey, TValue>> _table1;
    std::vector<DictSlot<TKey, TValue>> _table2;
};


}

#include "CuckooHashDictionary/CuckooHashBuild.tpp"
#include "CuckooHashDictionary/CuckooHashGetters.tpp"
#include "CuckooHashDictionary/CuckooHashSetters.tpp"

#endif //CUCKOOHASHDICTIONARY_H
