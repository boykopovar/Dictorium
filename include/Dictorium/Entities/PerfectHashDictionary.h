#ifndef PERFECTHASHDICTIONARY_H
#define PERFECTHASHDICTIONARY_H

#include <list>
#include <random>
#include <vector>
#include <utility>
#include <stdexcept>
#include"../Contracts/Concepts.h"

#define PERFECTHASH_SALT 2654435761ULL
#define PERFECTHASH_DEPRECATED_POSTFIX "This method of PerfectHashDictionary possibly triggers full hard rebuild. Prefer initializer_list constructor."
#define PERFECTHASH_DICT_NAME "PerfectHashDictionary"

namespace dtr{

struct PhBucket {
    uint64_t Offset;
    uint64_t Size;
    uint64_t Seed;
};

template<typename TKey, typename TValue>
class IDictionary;

template<typename TKey, typename TValue>
class PerfectHashDictionary : public IDictionary<TKey, TValue> {
public:
    PerfectHashDictionary() = default;
    PerfectHashDictionary(std::initializer_list<std::pair<TKey, TValue>> init) {
        _build(init.begin(), init.end(), init.size());
    }
    PerfectHashDictionary(std::vector<std::pair<TKey, TValue>> init){
        _build(init.begin(), init.end(), init.size());
    }

    template<PairIterator<TKey, TValue> TIter>
    PerfectHashDictionary(TIter begin, TIter end){
        _build(begin, end, std::distance(begin, end));
    }

    bool ContainsKey(const TKey& key) const override;
    bool TryGetValue(const TKey& key, TValue& value) const override;

    [[deprecated(PERFECTHASH_DEPRECATED_POSTFIX)]]
    void Add(const TKey& key, const TValue& value) override;

    [[deprecated(PERFECTHASH_DEPRECATED_POSTFIX)]]
    void InsertOrAssign(const TKey& key, const TValue& value) override;

    [[deprecated("Lazy removal (tombstone). Element is not phisically removed.")]]
    bool Remove(const TKey& key) override;

    void Clear() override;
    [[nodiscard]] size_t Count() const override;

    TValue& GetValue(const TKey& key) override;
    const TValue& GetValue(const TKey& key) const override;

    std::ostream& WriteToStream(std::ostream& os) const override {
        if constexpr (!StreamWritable<TValue> && ! StreamWritable<TKey>) {
            return os << "<class 'PerfectHashDictionary' TKey=" << typeid(TKey).name() << ", TValue=" << typeid(TValue).name() << '>';
        }
        else {
            // return this->_writeValues(os, *this);
            return os << "<class 'PerfectHashDictionary' TKey=" << typeid(TKey).name() << ", TValue=" << typeid(TValue).name() << '>';
        }
    }

private:

    static inline size_t _fastRange(const uint64_t hash, const size_t count) {
        return ((__uint128_t)hash * count) >> 64;
    }

    static inline size_t _hashRaw(const uint64_t stdHash, const uint64_t seed, const size_t tableSize) {
        return _fastRange((seed * PERFECTHASH_SALT + 1) * stdHash + seed, tableSize);
    }

    size_t _hash(const TKey& key, const uint64_t seed, const size_t tableSize) const {
        return _hashRaw(std::hash<TKey>{}(key), seed, tableSize);
    }

    uint64_t _globalSeed;
    uint64_t _count;
    uint64_t _tableSize;

    std::vector<PhBucket> _buckets;
    std::vector<std::pair<TValue, bool>> _values;

    [[nodiscard]] uint64_t _randomNum() const;
    uint64_t _findSeed(const std::vector<std::pair<TKey, TValue>>& bucket, size_t tableSize) const;
    [[nodiscard]] size_t _nextPrime(size_t n) const;

    uint64_t _findIndex(const TKey& key) const;

    template<PairIterator<TKey, TValue> TIter>
    void _build(TIter begin, TIter end, size_t size);
};

#include "PerfectHashDictionary/PerfectHashDictionaryBuild.tpp"
#include "PerfectHashDictionary/PerfectHashDictionaryGetters.tpp"
#include "PerfectHashDictionary/PerfectHashDictionarySetters.tpp"

}
#endif //PERFECTHASHDICTIONARY_H
