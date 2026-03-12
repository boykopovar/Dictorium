#ifndef DICTPROXY_H
#define DICTPROXY_H

#include <utility>

namespace dtr {

template<typename TKey, typename TValue>
class IDictionary;

template<typename TKey, typename TValue>
class DictProxy {
public:
    DictProxy(IDictionary<TKey, TValue>* dict, const TKey& key) : _dict(dict), _key(key) {}

    DictProxy& operator=(const TValue& value) {
        _dict->InsertOrAssign(_key, value);
        return *this;
    }

    operator TValue&() {
        return this->_dict->_get(_key);
    }

    operator const TValue&() const {
        return this->_dict->_get(_key);
    }

    TValue* operator->() {
        return &(_dict->_get(_key));
    }

    const TValue* operator*() const {
        return &(_dict->_get(_key));
    }

    auto& operator[](size_t i) {
        return _dict->_get(_key)[i];
    }

    const auto& operator[](size_t i) const {
        return _dict->_get(_key)[i];
    }

    operator TValue() const = delete;

    friend std::ostream& operator<<(std::ostream& os, const DictProxy<TKey, TValue>& obj) {
        return os << obj._dict->_get(obj._key);
    }

private:
    IDictionary<TKey, TValue>* _dict;
    TKey _key;
};



}
#endif //DICTPROXY_H
