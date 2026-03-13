#ifndef ITERATOR_H
#define ITERATOR_H

#include <memory>
#include "IEnumerable.h"

namespace dtr{

template<typename T>
class Iterator {
public:
    explicit Iterator(std::unique_ptr<IEnumerator<T>> e)
    : _e(std::move(e)), _current(nullptr) {
        if(_e) _current = _e->MoveNext();
    }

    bool operator!=(const Iterator&) const{
        return _current != nullptr;
    }

    void operator++(){
        if (!_e) return;
        _current = _e->MoveNext();
    }

    T& operator*() const {
        return *_current;
    }

private:
    std::unique_ptr<IEnumerator<T>> _e;
    T* _current;
};



}

#endif //ITERATOR_H
