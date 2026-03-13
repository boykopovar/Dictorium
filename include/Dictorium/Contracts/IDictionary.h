#ifndef IDICTIONARY_H
#define IDICTIONARY_H

#include <vector>
#include <type_traits>
#include <concepts>
#include <typeinfo>

#include "DictProxy.h"
#include "IEnumerable/IEnumerable.h"
#include "IEnumerable/Iterator.h"
#include "ItemsRange.h"
#include "IFormattable.h"


namespace dtr {

template<typename T>
concept StreamWritable =
    requires(std::ostream& os, T value) {
        os << value;
    };

template<typename TKey, typename TValue>
class IDictionary : public IFormattable {
public:
    IDictionary() = default;
    virtual ~IDictionary() = default;

    virtual bool ContainsKey(const TKey& key) const = 0;
    virtual bool TryGetValue(const TKey& key, TValue& value) const = 0;

    virtual void Add(const TKey& key, const TValue& value) = 0;
    virtual void InsertOrAssign(const TKey& key, const TValue& value) = 0;

    virtual bool Remove(const TKey& key) = 0;
    virtual void Clear() = 0;
    [[nodiscard]] virtual size_t Count() const = 0;

    virtual TValue& GetValue(const TKey& key) = 0;
    virtual const TValue& GetValue(const TKey& key) const = 0;

    void WriteToStream(std::ostream& os) const override {
        if constexpr (!StreamWritable<TKey> && !StreamWritable<TValue>){
            os << "<class 'IDictionary': TKey=" << typeid(TKey).name() << ", TValue=" << typeid(TValue).name() << '>';
        }
        else
        {
            bool first = true;
            os << '{' << '\n';

            for (auto& [key, value] : Items()) {
                if (!first) os << ',' << '\n';
                first = false;
                os << "    ";

                if constexpr (StreamWritable<TKey>) os << '"' << key << '"';
                else os << "<TKey=" << typeid(TKey).name() << '>';

                os << ": ";

                if constexpr (std::is_same_v<TValue, std::string>) os << '"' << value << '"';
                else if constexpr (StreamWritable<TValue>) os << value;
                else os << "<TValue=" << typeid(value).name() << '>';
            }
            os << '\n' << '}';
        }
    }

    ItemsRange<TKey, TValue> Items() const {
        return ItemsRange(_getItemsEnumerator());
    }
    
    DictProxy<TKey, TValue> operator[](const TKey& key) noexcept {
        return DictProxy<TKey, TValue>(this, key);
    }


protected:
    virtual std::unique_ptr<IEnumerator<std::pair<TKey, TValue>>> _getItemsEnumerator() const = 0;
    friend class DictProxy<TKey, TValue>;
};

}

#endif //IDICTIONARY_H
