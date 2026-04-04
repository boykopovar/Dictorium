#ifndef PERFECTHASHDICTIONARY_H
#define PERFECTHASHDICTIONARY_H

#include <random>
#include <vector>
#include <utility>
#include <stdexcept>
#include "../Contracts/IEnumerable/IEnumerable.h"
#define PERFECTHASH_SALT 2654435761ULL

namespace dtr{

template<typename TKey, typename TValue>
class IDictionary;

template<typename TKey, typename TValue>
class PerfectHashDictionary : public IDictionary<TKey, TValue> {
public:
    PerfectHashDictionary() = default;
    PerfectHashDictionary(std::initializer_list<std::pair<TKey, TValue>> init);

    bool ContainsKey(const TKey& key) const override;
    bool TryGetValue(const TKey& key, TValue& value) const override;

    [[deprecated("PerfectHashDictionary::Add triggers full rebuild O(n). "
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
};

#include "../../../src/PerfectHashDictionary/PerfectHashDictionary.tpp"
#include "../../../src/PerfectHashDictionary/PerfectHashDictionaryGetters.tpp"
#include "../../../src/PerfectHashDictionary/PerfectHashDictionarySetters.tpp"

}
#endif //PERFECTHASHDICTIONARY_H
