#ifndef LINEARDICTIONARY_H
#define LINEARDICTIONARY_H

#include <vector>
#include <stdexcept>
#include "../Contracts/IEnumerable/LinearEnumerator.h"

namespace dtr{


template<typename TKey, typename TValue>
class IDictionary;

template<typename TKey, typename TValue>
class LinearDictionary : public IDictionary<TKey, TValue> {
public:
    LinearDictionary(std::initializer_list<std::pair<TKey, TValue>> init) {
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
    

private:
    TValue& GetValue(const TKey& key) override;
    const TValue& GetValue(const TKey& key) const override;

    std::unique_ptr<IEnumerator<std::pair<TKey, TValue>>> _getItemsEnumerator() const override {
        return std::make_unique<LinearEnumerator<TKey, TValue>>(_dict);
    }

    std::vector<std::pair<TKey, TValue>> _dict;
};

#include "../src/LinearDictionary/LinearDictionary.tpp"
#include "../src/LinearDictionary/LinearDictionarySetters.tpp"
#include "../src/LinearDictionary/LinearDictionaryGetters.tpp"

}

#endif //LINEARDICTIONARY_H
