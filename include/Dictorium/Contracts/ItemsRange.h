#ifndef ITEMSRANGE_H
#define ITEMSRANGE_H

#include <utility>
#include "IEnumerable/Iterator.h"

namespace dtr{

template<typename TKey, typename TValue>
class IDictionary;

template<typename TKey, typename TValue>
class ItemsRange : public IEnumerable<std::pair<TKey, TValue>> {
public:
    ItemsRange(std::unique_ptr<IEnumerator<std::pair<TKey, TValue>>> enumerator)
        : _enumerator(std::move(enumerator)) {}

    Iterator<std::pair<TKey, TValue>> begin() const override {
        return Iterator<std::pair<TKey, TValue>>(std::move(_enumerator));
    }

    Iterator<std::pair<TKey, TValue>> end() const override {
        return Iterator<std::pair<TKey, TValue>>(nullptr);
    }
private:
    mutable std::unique_ptr<IEnumerator<std::pair<TKey, TValue>>> _enumerator;
};

}

#endif //ITEMSRANGE_H
