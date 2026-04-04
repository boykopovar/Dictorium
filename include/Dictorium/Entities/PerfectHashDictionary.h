#ifndef PERFECTHASHDICTIONARY_H
#define PERFECTHASHDICTIONARY_H

#include <vector>
#include <utility>
#include <stdexcept>
#include "../Contracts/IEnumerable/IEnumerable.h"

namespace dtr{

template<typename TKey, typename TValue>
class IDictionary;

template<typename TKey, typename TValue>
class PerfectHashDictionary : public IDictionary<TKey, TValue> {
public:
    PerfectHashDictionary() = default;
    PerfectHashDictionary(std::initializer_list<std::pair<TKey, TValue>> init) {
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

    std::unique_ptr<IEnumerator<std::pair<TKey, TValue>>> _getItemsEnumerator() const override {
        throw std::logic_error("Not implemented");
    }

    TValue& GetValue(const TKey& key) override;
    const TValue& GetValue(const TKey& key) const override;
};

#include "../../../src/PerfectHashDictionary/PerfectHashDictionary.tpp"
#include "../../../src/PerfectHashDictionary/PerfectHashDictionaryGetters.tpp"
#include "../../../src/PerfectHashDictionary/PerfectHashDictionarySetters.tpp"

}
#endif //PERFECTHASHDICTIONARY_H
