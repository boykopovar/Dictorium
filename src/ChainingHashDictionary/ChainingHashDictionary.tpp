#ifndef CHAININGHASHDICTIONARY_TPP
#define CHAININGHASHDICTIONARY_TPP

namespace dtr
{

template <typename TKey, typename TValue, typename Hash>
ChainingHashDictionary<TKey, TValue, Hash>::ChainingHashDictionary(size_t capacity) : _buckets(capacity), _count(0)
{
}

template <typename TKey, typename TValue, typename Hash>
ChainingHashDictionary<TKey, TValue, Hash>::ChainingHashDictionary(std::initializer_list<Pair> list)
    : _buckets(8), _count(0)
{
    for (const auto &[key, value] : list)
    {
        try
        {
            Add(key, value);
        }
        catch (const std::invalid_argument &)
        {
            throw std::invalid_argument("Duplicate key");
        }
    }
}

template <typename TKey, typename TValue, typename Hash>
template <typename InputIt>
ChainingHashDictionary<TKey, TValue, Hash>::ChainingHashDictionary(InputIt first, InputIt last) : _buckets(8), _count(0)
{
    for (auto it = first; it != last; ++it)
    {
        const auto &[key, value] = *it;

        try
        {
            Add(key, value);
        }
        catch (const std::invalid_argument &)
        {
            throw std::invalid_argument("Duplicate key");
        }
    }
}

template <typename TKey, typename TValue, typename Hash>
size_t ChainingHashDictionary<TKey, TValue, Hash>::_getIndex(const TKey &key) const
{
    return _hasher(key) % _buckets.size();
}

template <typename TKey, typename TValue, typename Hash> void ChainingHashDictionary<TKey, TValue, Hash>::_rehash()
{
    size_t newSize = _buckets.size() * 2;
    std::vector<Bucket> newBuckets(newSize);

    for (const auto &bucket : _buckets)
    {
        for (const auto &[key, value] : bucket)
        {
            size_t index = _hasher(key) % newSize;
            newBuckets[index].emplace_back(key, value);
        }
    }

    _buckets = std::move(newBuckets);
}

template <typename TKey, typename TValue, typename Hash>
void ChainingHashDictionary<TKey, TValue, Hash>::_ensureCapacity()
{
    if ((double)_count / _buckets.size() > _maxLoadFactor)
    {
        _rehash();
    }
}

// ===== Методы =====

template <typename TKey, typename TValue, typename Hash>
bool ChainingHashDictionary<TKey, TValue, Hash>::ContainsKey(const TKey &key) const
{
    const auto &bucket = _buckets[_getIndex(key)];
    for (const auto &[k, _] : bucket)
    {
        if (k == key)
            return true;
    }
    return false;
}

template <typename TKey, typename TValue, typename Hash>
bool ChainingHashDictionary<TKey, TValue, Hash>::TryGetValue(const TKey &key, TValue &value) const
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

template <typename TKey, typename TValue, typename Hash>
void ChainingHashDictionary<TKey, TValue, Hash>::Add(const TKey &key, const TValue &value)
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

template <typename TKey, typename TValue, typename Hash>
void ChainingHashDictionary<TKey, TValue, Hash>::InsertOrAssign(const TKey &key, const TValue &value)
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

template <typename TKey, typename TValue, typename Hash>
bool ChainingHashDictionary<TKey, TValue, Hash>::Remove(const TKey &key)
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

template <typename TKey, typename TValue, typename Hash> void ChainingHashDictionary<TKey, TValue, Hash>::Clear()
{
    _buckets.clear();
    _buckets.resize(8);
    _count = 0;
}

template <typename TKey, typename TValue, typename Hash>
size_t ChainingHashDictionary<TKey, TValue, Hash>::Count() const
{
    return _count;
}

template <typename TKey, typename TValue, typename Hash>
TValue &ChainingHashDictionary<TKey, TValue, Hash>::GetValue(const TKey &key)
{
    auto &bucket = _buckets[_getIndex(key)];

    for (auto &[k, v] : bucket)
    {
        if (k == key)
            return v;
    }

    throw std::out_of_range("Key not found");
}

template <typename TKey, typename TValue, typename Hash>
const TValue &ChainingHashDictionary<TKey, TValue, Hash>::GetValue(const TKey &key) const
{
    const auto &bucket = _buckets[_getIndex(key)];

    for (const auto &[k, v] : bucket)
    {
        if (k == key)
            return v;
    }

    throw std::out_of_range("Key not found");
}

template <typename TKey, typename TValue, typename Hash>
typename ChainingHashDictionary<TKey, TValue, Hash>::Iterator ChainingHashDictionary<TKey, TValue, Hash>::begin() const
{
    return Iterator(_buckets.begin(), _buckets.end());
}

template <typename TKey, typename TValue, typename Hash>
typename ChainingHashDictionary<TKey, TValue, Hash>::Iterator ChainingHashDictionary<TKey, TValue, Hash>::end() const
{
    return Iterator(_buckets.end(), _buckets.end());
}

} // namespace dtr

#endif // CHAININGHASHDICTIONARY_TPP