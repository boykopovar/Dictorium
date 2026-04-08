#ifndef DICTORIUMCONCEPTS_H
#define DICTORIUMCONCEPTS_H

#include <utility>

namespace dtr
{
    
template<typename T>
concept StreamWritable =
requires(std::ostream& os, T value) {
    os << value;
};

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

template <typename TIter, typename TValue>
concept ValuesIterator = requires(TIter iter) {
    ++iter;
    *iter;
    iter != iter;
    {* iter} -> std::convertible_to<TValue>;
};

template <typename TCollection, typename TKey, typename TValue>
concept ItemsCollection = requires(const TCollection c) {
    c.Items();

    requires SizedIterable<decltype(c.Items())>;

    requires PairIterator<
        decltype(std::begin(c.Items())),
        TKey,
        TValue
    >;
};

template <typename TCollection, typename TValue>
concept ValuesCollection = requires(const TCollection c) {
    c.Items();

    requires SizedIterable<decltype(c.Values())>;

    requires ValuesIterator<
        decltype(std::begin(c.Values())),
        TValue
    >;
};
    
}
#endif //DICTORIUMCONCEPTS_H
