#pragma once

#include "Dictorium/Entities/ChainingHashDictionary.h"

namespace dtr
{

template <typename TKey, typename TValue>
ChainingHashDictionary<TKey, TValue>::ChainingHashDictionary(size_t capacity) : _buckets(capacity), _count(0)
{
}

template <typename TKey, typename TValue>
ChainingHashDictionary<TKey, TValue>::ChainingHashDictionary(std::initializer_list<Pair> list) : _buckets(8), _count(0)
{
    for (const auto &[key, value] : list)
    {
        if (ContainsKey(key))
        {
            throw std::invalid_argument("Duplicate key");
        }
        Add(key, value);
    }
}

template <typename TKey, typename TValue> size_t ChainingHashDictionary<TKey, TValue>::_getIndex(const TKey &key) const
{
    return std::hash<TKey>{}(key) % _buckets.size();
}

template <typename TKey, typename TValue> void ChainingHashDictionary<TKey, TValue>::_rehash()
{
    size_t newSize = _buckets.size() * 2;
    std::vector<Bucket> newBuckets(newSize);

    for (const auto &bucket : _buckets)
    {
        for (const auto &[key, value] : bucket)
        {
            size_t index = std::hash<TKey>{}(key) % newSize;
            newBuckets[index].emplace_back(key, value);
        }
    }

    _buckets = std::move(newBuckets);
}

template <typename TKey, typename TValue> void ChainingHashDictionary<TKey, TValue>::_ensureCapacity()
{
    if ((double)_count / _buckets.size() > _maxLoadFactor)
    {
        _rehash();
    }
}

// ===== Методы =====

template <typename TKey, typename TValue> bool ChainingHashDictionary<TKey, TValue>::ContainsKey(const TKey &key) const
{
    const auto &bucket = _buckets[_getIndex(key)];
    for (const auto &[k, _] : bucket)
    {
        if (k == key)
            return true;
    }
    return false;
}

template <typename TKey, typename TValue>
bool ChainingHashDictionary<TKey, TValue>::TryGetValue(const TKey &key, TValue &value) const
{
    const auto &bucket = _buckets[_getIndex(key)];
    for (const auto &[k, v] : bucket)
    {
        if (k == key)
        {
            value = v;
            return true;
        }
    }
    return false;
}

template <typename TKey, typename TValue>
void ChainingHashDictionary<TKey, TValue>::Add(const TKey &key, const TValue &value)
{
    auto &bucket = _buckets[_getIndex(key)];

    for (const auto &[k, _] : bucket)
    {
        if (k == key)
        {
            throw std::invalid_argument("Key exists");
        }
    }

    bucket.emplace_back(key, value);
    _count++;
    _ensureCapacity();
}

template <typename TKey, typename TValue>
void ChainingHashDictionary<TKey, TValue>::InsertOrAssign(const TKey &key, const TValue &value)
{
    auto &bucket = _buckets[_getIndex(key)];

    for (auto &[k, v] : bucket)
    {
        if (k == key)
        {
            v = value;
            return;
        }
    }

    bucket.emplace_back(key, value);
    _count++;
    _ensureCapacity();
}

template <typename TKey, typename TValue> bool ChainingHashDictionary<TKey, TValue>::Remove(const TKey &key)
{
    auto &bucket = _buckets[_getIndex(key)];

    for (auto it = bucket.begin(); it != bucket.end(); ++it)
    {
        if (it->first == key)
        {
            bucket.erase(it);
            _count--;
            return true;
        }
    }
    return false;
}

template <typename TKey, typename TValue> void ChainingHashDictionary<TKey, TValue>::Clear()
{
    _buckets.clear();
    _buckets.resize(8);
    _count = 0;
}

template <typename TKey, typename TValue> size_t ChainingHashDictionary<TKey, TValue>::Count() const
{
    return _count;
}

template <typename TKey, typename TValue> TValue &ChainingHashDictionary<TKey, TValue>::GetValue(const TKey &key)
{
    auto &bucket = _buckets[_getIndex(key)];

    for (auto &[k, v] : bucket)
    {
        if (k == key)
            return v;
    }

    throw std::out_of_range("Key not found");
}

template <typename TKey, typename TValue>
const TValue &ChainingHashDictionary<TKey, TValue>::GetValue(const TKey &key) const
{
    const auto &bucket = _buckets[_getIndex(key)];

    for (const auto &[k, v] : bucket)
    {
        if (k == key)
            return v;
    }

    throw std::out_of_range("Key not found");
}

template <typename TKey, typename TValue>
typename ChainingHashDictionary<TKey, TValue>::Iterator ChainingHashDictionary<TKey, TValue>::begin()
{
    return Iterator(_buckets.begin(), _buckets.end());
}

template <typename TKey, typename TValue>
typename ChainingHashDictionary<TKey, TValue>::Iterator ChainingHashDictionary<TKey, TValue>::end()
{
    return Iterator(_buckets.end(), _buckets.end());
}

} // namespace dtr