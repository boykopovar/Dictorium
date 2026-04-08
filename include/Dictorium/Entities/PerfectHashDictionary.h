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

template <typename TKey, typename TValue>
struct PhBucket {
    std::vector<std::pair<TValue, bool>> Values;
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

private:
    uint64_t _globalSeed;
    uint64_t _count;
    uint64_t _tableSize;

    std::vector<PhBucket<TKey, TValue>> _buckets;

    size_t _hash(const TKey& key, uint64_t seed, size_t tableSize) const;
    [[nodiscard]] uint64_t _randomNum() const;
    uint64_t _findSeed(const std::vector<std::pair<TKey, TValue>>& bucket, size_t tableSize) const;
    [[nodiscard]] size_t _nextPrime(size_t n) const;

    template<PairIterator<TKey, TValue> TIter>
    void _build(TIter begin, TIter end, size_t size);
};

#include "../../../src/PerfectHashDictionary/PerfectHashDictionaryBuild.tpp"
#include "../../../src/PerfectHashDictionary/PerfectHashDictionaryGetters.tpp"
#include "../../../src/PerfectHashDictionary/PerfectHashDictionarySetters.tpp"

}
#endif //PERFECTHASHDICTIONARY_H
