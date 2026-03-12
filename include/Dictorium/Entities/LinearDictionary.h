#ifndef LINEARDICTIONARY_H
#define LINEARDICTIONARY_H

#include <vector>

namespace dtr{
template<typename TKey, typename TValue>
class IDictionary;

template<typename TKey, typename TValue>
class LinearDictionary : public IDictionary<TKey, TValue> {
public:
    LinearDictionary(std::initializer_list<std::pair<TKey, TValue>> init) {
        for (auto i = init.begin(), e = init.end(); i != e; ++i) {
            (*this)[i->first] = i->second;
        }
    }

    bool ContainsKey(const TKey& key) const override;
    bool TryGetValue(const TKey& key, TValue& value) const override;

    void Add(const TKey& key, const TValue& value) override;
    void InsertOrAssign(const TKey& key, const TValue& value) override;

    bool Remove(const TKey& key) override;
    void Clear() override;
    [[nodiscard]] size_t Count() const override;

    const std::vector<std::pair<TKey, TValue>>& Items() const override {
        return _dict;
    }

private:
    TValue& _get(const TKey& key) override;
    const TValue& _get(const TKey& key) const override;

    std::vector<std::pair<TKey, TValue>> _dict;
};

#include "../src/LinearDictionary/LinearDictionary.tpp"
#include "../src/LinearDictionary/LinearDictionarySetters.tpp"
#include "../src/LinearDictionary/LinearDictionaryGetters.tpp"

}

#endif //LINEARDICTIONARY_H
