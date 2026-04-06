#ifndef DICTORIUMCONCEPTS_H
#define DICTORIUMCONCEPTS_H

#include <utility>


template<typename T>
concept SizedIterable = requires(T c) {
    c.begin();
    c.end();
    c.size();
};

template <typename TIter, typename TKey, typename TValue>
concept PairIterator = requires(TIter iter) {
    ++iter;
    *iter;
    iter != iter;
    {iter->first} -> std::convertible_to<TKey>;
    {iter->second} -> std::convertible_to<TValue>;
};

#endif //DICTORIUMCONCEPTS_H
