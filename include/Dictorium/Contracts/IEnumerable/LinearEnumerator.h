#ifndef LINEARENUMERATOR_H
#define LINEARENUMERATOR_H

#include <utility>
#include <vector>
#include "IEnumerable.h"

namespace dtr{


template<typename TKey, typename TValue>
class LinearEnumerator : public IEnumerator<std::pair<TKey, TValue>> {
public:
    explicit LinearEnumerator(const std::vector<std::pair<TKey, TValue>>& data)
      : _data(data), _index(0) {}

    std::pair<TKey, TValue>* MoveNext() override{
        if(_index >= _data.size()) {
            return nullptr;
        }
        return const_cast<std::pair<TKey, TValue>*>(&_data[_index++]);
    }

private:
    const std::vector<std::pair<TKey, TValue>>& _data;
    size_t _index;
};

}

#endif //LINEARENUMERATOR_H
