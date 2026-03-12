#ifndef IENUMERABLE_H
#define IENUMERABLE_H

#include <memory>


namespace dtr{

template<typename T>
class Iterator;

template<typename T>
class IEnumerator{
public:
      virtual ~IEnumerator() = default;
      virtual T* MoveNext() = 0;
};


template<typename T>
class IEnumerable {
public:
      virtual ~IEnumerable() = default;

      virtual Iterator<T> begin() const = 0;
      virtual Iterator<T> end() const = 0;
};

}

#endif //IENUMERABLE_H
