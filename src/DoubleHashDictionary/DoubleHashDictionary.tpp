#ifndef DOUBLEHASHDICTIONARY_TPP
#define DOUBLEHASHDICTIONARY_TPP

namespace dtr
{

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::DoubleHashDictionary(size_t capacity)
    : _table(_nextPrime(capacity)), _count(0)
{
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::DoubleHashDictionary(
    std::initializer_list<std::pair<TKey, TValue>> list)
    : _table(11), _count(0)
{
    for (const auto &[k, v] : list)
    {
        Add(k, v);
    }
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
template <typename InputIt>
DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::DoubleHashDictionary(InputIt first, InputIt last)
    : _table(11), _count(0)
{
    for (auto it = first; it != last; ++it)
    {
        Add(it->first, it->second);
    }
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
size_t DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::_findIndex(const TKey &key) const
{
    size_t capacity = _table.size();

    size_t h1 = _hash1(key);
    size_t h2 = _hash2(key);

    for (size_t i = 0; i < capacity; ++i)
    {
        size_t pos = (h1 + i * h2) % capacity;

        const auto &e = _table[pos];

        if (e.state == EntryState::Empty)
        {
            return capacity;
        }

        if (e.state == EntryState::Occupied && e.key == key)
        {
            return pos;
        }
    }

    return capacity;
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
size_t DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::_findSlotForInsert(const TKey &key)
{
    size_t capacity = _table.size();

    size_t h1 = _hash1(key);
    size_t h2 = _hash2(key);

    size_t firstDeleted = capacity;

    for (size_t i = 0; i < capacity; ++i)
    {
        size_t pos = (h1 + i * h2) % capacity;

        auto &e = _table[pos];

        if (e.state == EntryState::Occupied && e.key == key)
        {
            return pos;
        }

        if (e.state == EntryState::Deleted && firstDeleted == capacity)
        {
            firstDeleted = pos;
        }

        if (e.state == EntryState::Empty)
        {
            return (firstDeleted != capacity) ? firstDeleted : pos;
        }
    }

    return capacity;
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
void DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::_ensureCapacity()
{
    if ((double)_count / _table.size() > _maxLoadFactor)
    {
        _rehash();
    }
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
void DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::_rehash()
{
    std::vector<Entry> old = std::move(_table);

    _table = std::vector<Entry>(_nextPrime(old.size() * 2));

    _count = 0;

    for (const auto &e : old)
    {
        if (e.state == EntryState::Occupied)
        {
            InsertOrAssign(e.key, e.value);
        }
    }
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
bool DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::ContainsKey(const TKey &key) const
{
    return _findIndex(key) != _table.size();
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
bool DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::TryGetValue(const TKey &key, TValue &value) const
{
    size_t pos = _findIndex(key);

    if (pos == _table.size())
    {
        return false;
    }

    value = _table[pos].value;

    return true;
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
void DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::Add(const TKey &key, const TValue &value)
{
    _ensureCapacity();

    size_t pos = _findSlotForInsert(key);

    if (_table[pos].state == EntryState::Occupied)
    {
        throw std::invalid_argument("Key exists");
    }

    _table[pos] = {key, value, EntryState::Occupied};

    ++_count;
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
void DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::InsertOrAssign(const TKey &key, const TValue &value)
{
    _ensureCapacity();

    size_t pos = _findSlotForInsert(key);

    if (_table[pos].state == EntryState::Occupied)
    {
        _table[pos].value = value;
        return;
    }

    _table[pos] = {key, value, EntryState::Occupied};

    ++_count;
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
bool DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::Remove(const TKey &key)
{
    size_t pos = _findIndex(key);

    if (pos == _table.size())
    {
        return false;
    }

    _table[pos].state = EntryState::Deleted;

    --_count;

    return true;
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
void DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::Clear()
{
    _table.clear();
    _table.resize(11);

    _count = 0;
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
size_t DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::Count() const
{
    return _count;
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
TValue &DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::GetValue(const TKey &key)
{
    size_t pos = _findIndex(key);

    if (pos == _table.size())
    {
        throw std::out_of_range("Key not found");
    }

    return _table[pos].value;
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
const TValue &DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::GetValue(const TKey &key) const
{
    size_t pos = _findIndex(key);

    if (pos == _table.size())
    {
        throw std::out_of_range("Key not found");
    }

    return _table[pos].value;
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
void DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::Iterator::_skip()
{
    while (_index < _table->size() && (*_table)[_index].state != EntryState::Occupied)
    {
        ++_index;
    }
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::Iterator::Iterator(size_t index, const std::vector<Entry> *table)
    : _index(index), _table(table)
{
    _skip();
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
typename DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::Iterator &DoubleHashDictionary<TKey, TValue, Hash1,
                                                                                          Hash2>::Iterator::operator++()
{
    ++_index;

    _skip();

    return *this;
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
bool DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::Iterator::operator==(const Iterator &other) const
{
    return _index == other._index && _table == other._table;
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
bool DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::Iterator::operator!=(const Iterator &other) const
{
    return !(*this == other);
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
typename DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::Iterator DoubleHashDictionary<TKey, TValue, Hash1,
                                                                                         Hash2>::begin() const
{
    return Iterator(0, &_table);
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
typename DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::Iterator DoubleHashDictionary<TKey, TValue, Hash1,
                                                                                         Hash2>::end() const
{
    return Iterator(_table.size(), &_table);
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
bool DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::_isPrime(size_t n)
{
    if (n < 2)
        return false;

    for (size_t i = 2; i * i <= n; ++i)
    {
        if (n % i == 0)
            return false;
    }

    return true;
}

template <typename TKey, typename TValue, typename Hash1, typename Hash2>
size_t DoubleHashDictionary<TKey, TValue, Hash1, Hash2>::_nextPrime(size_t n)
{
    if (n <= 2)
        return 2;
    if (n % 2 == 0)
        ++n;

    while (true)
    {
        bool prime = true;
        for (size_t i = 3; i * i <= n; i += 2)
        {
            if (n % i == 0)
            {
                prime = false;
                break;
            }
        }
        if (prime)
            return n;
        n += 2;
    }
}

} // namespace dtr

#endif