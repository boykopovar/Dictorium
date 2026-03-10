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

    operator TValue() const {
        return _dict->_get(_key);
    }

    friend std::ostream& operator<<(std::ostream& os, const DictProxy<TKey, TValue>& obj) {
        return os << obj._key;
    }

private:
    IDictionary<TKey, TValue>* _dict;
    TKey _key;
};



}
#endif //DICTPROXY_H
