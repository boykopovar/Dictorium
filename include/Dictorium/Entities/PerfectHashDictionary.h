#ifndef PERFECTHASHDICTIONARY_H
#define PERFECTHASHDICTIONARY_H

#include <list>
#include <random>
#include <vector>
#include <utility>
#include <stdexcept>
#include "../Contracts/IEnumerable/IEnumerable.h"

#define PERFECTHASH_SALT 2654435761ULL

namespace dtr{

template <typename TKey, typename TValue>
struct PhBucket {
    std::vector<std::pair<TValue, bool>> Values;
    uint64_t Seed;
};

template<typename T>
concept SizedIterable = requires(T c) {
    c.begin();
    c.end();
    c.size();
};

template<typename TKey, typename TValue>
class IDictionary;

template<typename TKey, typename TValue>
class PerfectHashDictionary : public IDictionary<TKey, TValue> {
public:
    PerfectHashDictionary() = default;
    PerfectHashDictionary(std::initializer_list<std::pair<TKey, TValue>> init) {
        _build(init);
    }
    PerfectHashDictionary(std::vector<std::pair<TKey, TValue>> init){
        _build(init);
    }

    bool ContainsKey(const TKey& key) const override;
    bool TryGetValue(const TKey& key, TValue& value) const override;

    [[deprecated("PerfectHashDictionary::Add triggers full rebuild. "
        "Prefer initializer_list constructor.")]]
    void Add(const TKey& key, const TValue& value) override;
    void InsertOrAssign(const TKey& key, const TValue& value) override;

    bool Remove(const TKey& key) override;
    void Clear() override;
    [[nodiscard]] size_t Count() const override;

    std::unique_ptr<IEnumerator<std::pair<TKey, TValue>>> _getItemsEnumerator() const override {
        throw std::logic_error("Not implemented");
    }

    TValue& GetValue(const TKey& key) override;
    const TValue& GetValue(const TKey& key) const override;

private:
    uint64_t _globalSeed;
    uint64_t _count;
    std::vector<PhBucket<TKey, TValue>> _buckets;

    size_t _hash(const TKey& key, const uint64_t seed, const size_t tableSize) const {
        auto keyHash = std::hash<TKey>{}(key);

        uint64_t a = seed * PERFECTHASH_SALT + 1;
        uint64_t b = seed * PERFECTHASH_SALT;

        return (a * keyHash + b) % tableSize;
    }

    [[nodiscard]] uint64_t _randomNum() const {
        static std::mt19937_64 generator(std::random_device{}());
        return generator();
    }

    uint64_t _findSeed(const std::vector<std::pair<TKey, TValue>>& bucket, size_t tableSize) const {
        constexpr size_t maxAttempts = 1000;
        std::vector<bool> occupied(tableSize, false);
        for (size_t i = 0; i < maxAttempts; ++i) {
            std::fill(occupied.begin(), occupied.end(), false);
            auto seed = _randomNum();
            bool collision = false;

            for (auto& [key, value] : bucket) {
                auto slot = _hash(key, seed, tableSize);
                if (occupied[slot]){
                    collision = true;
                    break;
                }
                occupied[slot] = true;
            }
            if (!collision) return seed;
        }
        return 0;
    }

    size_t _nextPrime(size_t n) {
        if (n<2) return 2;
        while (true) {
            bool prime = true;
            for (size_t i = 2; i * i <= n; ++i) {
                if (n%i == 0) {
                    prime = false;
                    break;
                }
            }
            if (prime) return n;
            ++n;
        }
    }

    template<SizedIterable TContainer>
    void _build(TContainer& init);
};

#include "../../../src/PerfectHashDictionary/PerfectHashDictionary.tpp"
#include "../../../src/PerfectHashDictionary/PerfectHashDictionaryGetters.tpp"
#include "../../../src/PerfectHashDictionary/PerfectHashDictionarySetters.tpp"

}
#endif //PERFECTHASHDICTIONARY_H
