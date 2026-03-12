#ifndef IDICTIONARY_H
#define IDICTIONARY_H

#include <vector>

#include "DictProxy.h"

namespace dtr {

template<typename TKey, typename TValue>
class IDictionary {
public:
    IDictionary() = default;
    virtual ~IDictionary() = default;

    virtual bool ContainsKey(const TKey& key) const = 0;
    virtual bool TryGetValue(const TKey& key, TValue& value) const = 0;

    virtual void Add(const TKey& key, const TValue& value) = 0;
    virtual void InsertOrAssign(const TKey& key, const TValue& value) = 0;

    virtual bool Remove(const TKey& key) = 0;
    virtual void Clear() = 0;
    virtual size_t Count() const = 0;

    virtual const std::vector<std::pair<TKey, TValue>>& Items() const = 0;

    DictProxy<TKey, TValue> operator[](const TKey& key) noexcept {
        return DictProxy<TKey, TValue>(this, key);
    }

protected:
    virtual TValue& _get(const TKey& key) = 0;
    virtual const TValue& _get(const TKey& key) const = 0;

    friend class DictProxy<TKey, TValue>;
};

}

#endif //IDICTIONARY_H
