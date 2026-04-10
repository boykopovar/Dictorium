#ifndef DICTORIUMCONCEPTS_H
#define DICTORIUMCONCEPTS_H

#include <utility>

namespace dtr
{

template<typename T>
concept CHashable =
requires(const T& key) {
    {key == key} -> std::convertible_to<bool>;
    {std::hash<T>{}(key)} -> std::convertible_to<size_t>;
};

template<typename T>
concept CStreamWritable =
requires(std::ostream& os, T value) {
    os << value;
};

template<typename T>
concept CSizedIterable = requires(T c) {
    c.begin();
    c.end();
    c.size();
};

template <typename TIter, typename TKey, typename TValue>
concept CPairIterator = requires(TIter iter) {
    ++iter;
    *iter;
    iter != iter;
    {iter->first} -> std::convertible_to<TKey>;
    {iter->second} -> std::convertible_to<TValue>;
};

template <typename TIter, typename TValue>
concept CValuesIterator = requires(TIter iter) {
    ++iter;
    *iter;
    iter != iter;
    {* iter} -> std::convertible_to<TValue>;
};

template <typename TCollection, typename TKey, typename TValue>
concept CItemsCollection = requires(const TCollection c) {
    c.Items();

    requires CSizedIterable<decltype(c.Items())>;

    requires CPairIterator<
        decltype(std::begin(c.Items())),
        TKey,
        TValue
    >;
};

template <typename TCollection, typename TValue>
concept CValuesCollection = requires(const TCollection c) {
    c.Values();

    requires CSizedIterable<decltype(c.Values())>;

    requires CValuesIterator<
        decltype(std::begin(c.Values())),
        TValue
    >;
};

}
#endif //DICTORIUMCONCEPTS_H
