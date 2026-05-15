#ifndef ROBINHOODHASHDICTIONARY_TPP
#define ROBINHOODHASHDICTIONARY_TPP

namespace dtr
{

template <typename TKey, typename TValue, typename Hash>
RobinHoodHashDictionary<TKey, TValue, Hash>::RobinHoodHashDictionary(size_t capacity) : _table(capacity), _count(0)
{
}

template <typename TKey, typename TValue, typename Hash>
RobinHoodHashDictionary<TKey, TValue, Hash>::RobinHoodHashDictionary(
    std::initializer_list<std::pair<TKey, TValue>> list)
    : _table(8), _count(0)
{
    for (const auto &[k, v] : list)
        Add(k, v);
}

template <typename TKey, typename TValue, typename Hash>
template <typename InputIt>
RobinHoodHashDictionary<TKey, TValue, Hash>::RobinHoodHashDictionary(InputIt first, InputIt last) : _table(8), _count(0)
{
    for (auto it = first; it != last; ++it)
        Add(it->first, it->second);
}

template <typename TKey, typename TValue, typename Hash>
void RobinHoodHashDictionary<TKey, TValue, Hash>::_ensureCapacity()
{
    if ((double)_count / _table.size() >= _maxLoadFactor)
        _rehash();
}

template <typename TKey, typename TValue, typename Hash> void RobinHoodHashDictionary<TKey, TValue, Hash>::_rehash()
{
    std::vector<Entry> old = std::move(_table);

    _table = std::vector<Entry>(old.size() * 2);
    _count = 0;

    for (const auto &e : old)
    {
        if (e.occupied)
            InsertOrAssign(e.key, e.value);
    }
}

template <typename TKey, typename TValue, typename Hash>
size_t RobinHoodHashDictionary<TKey, TValue, Hash>::_findIndex(const TKey &key) const
{
    size_t capacity = _table.size();
    size_t index = _hasher(key) % capacity;
    size_t psl = 0;

    while (true)
    {
        const auto &e = _table[index];

        if (!e.occupied)
            return capacity;

        if (e.psl < psl)
            return capacity;

        if (e.key == key)
            return index;

        index = (index + 1) % capacity;
        ++psl;
    }
}

template <typename TKey, typename TValue, typename Hash>
bool RobinHoodHashDictionary<TKey, TValue, Hash>::ContainsKey(const TKey &key) const
{
    return _findIndex(key) != _table.size();
}

template <typename TKey, typename TValue, typename Hash>
bool RobinHoodHashDictionary<TKey, TValue, Hash>::TryGetValue(const TKey &key, TValue &value) const
{
    size_t pos = _findIndex(key);

    if (pos == _table.size())
        return false;

    value = _table[pos].value;
    return true;
}

template <typename TKey, typename TValue, typename Hash>
void RobinHoodHashDictionary<TKey, TValue, Hash>::Add(const TKey &key, const TValue &value)
{
    if (ContainsKey(key))
        throw std::invalid_argument("Key exists");

    InsertOrAssign(key, value);
}

template <typename TKey, typename TValue, typename Hash>
void RobinHoodHashDictionary<TKey, TValue, Hash>::InsertOrAssign(const TKey &key, const TValue &value)
{
    _ensureCapacity();

    size_t capacity = _table.size();

    Entry current;
    current.key = key;
    current.value = value;
    current.occupied = true;
    current.psl = 0;

    size_t index = _hasher(key) % capacity;

    while (true)
    {
        auto &slot = _table[index];

        if (!slot.occupied)
        {
            slot = current;
            ++_count;
            return;
        }

        if (slot.key == current.key)
        {
            slot.value = current.value;
            return;
        }

        if (slot.psl < current.psl)
        {
            std::swap(slot, current);
        }

        index = (index + 1) % capacity;
        ++current.psl;
    }
}

template <typename TKey, typename TValue, typename Hash>
bool RobinHoodHashDictionary<TKey, TValue, Hash>::Remove(const TKey &key)
{
    size_t pos = _findIndex(key);

    if (pos == _table.size())
        return false;

    size_t capacity = _table.size();

    while (true)
    {
        size_t next = (pos + 1) % capacity;

        if (!_table[next].occupied || _table[next].psl == 0)
        {
            _table[pos].occupied = false;
            break;
        }

        _table[pos] = _table[next];
        --_table[pos].psl;

        pos = next;
    }

    --_count;
    return true;
}

template <typename TKey, typename TValue, typename Hash> void RobinHoodHashDictionary<TKey, TValue, Hash>::Clear()
{
    _table.clear();
    _table.resize(8);
    _count = 0;
}

template <typename TKey, typename TValue, typename Hash>
size_t RobinHoodHashDictionary<TKey, TValue, Hash>::Count() const
{
    return _count;
}

template <typename TKey, typename TValue, typename Hash>
TValue &RobinHoodHashDictionary<TKey, TValue, Hash>::GetValue(const TKey &key)
{
    size_t pos = _findIndex(key);

    if (pos == _table.size())
        throw std::out_of_range("Key not found");

    return _table[pos].value;
}

template <typename TKey, typename TValue, typename Hash>
const TValue &RobinHoodHashDictionary<TKey, TValue, Hash>::GetValue(const TKey &key) const
{
    size_t pos = _findIndex(key);

    if (pos == _table.size())
        throw std::out_of_range("Key not found");

    return _table[pos].value;
}

template <typename TKey, typename TValue, typename Hash>
void RobinHoodHashDictionary<TKey, TValue, Hash>::Iterator::_skip()
{
    while (_index < _table->size() && !(*_table)[_index].occupied)
        ++_index;
}

template <typename TKey, typename TValue, typename Hash>
RobinHoodHashDictionary<TKey, TValue, Hash>::Iterator::Iterator(size_t index, const std::vector<Entry> *table)
    : _index(index), _table(table)
{
    _skip();
}

template <typename TKey, typename TValue, typename Hash>
typename RobinHoodHashDictionary<TKey, TValue, Hash>::Iterator &RobinHoodHashDictionary<TKey, TValue,
                                                                                        Hash>::Iterator::operator++()
{
    ++_index;
    _skip();
    return *this;
}

template <typename TKey, typename TValue, typename Hash>
bool RobinHoodHashDictionary<TKey, TValue, Hash>::Iterator::operator==(const Iterator &other) const
{
    return _index == other._index && _table == other._table;
}

template <typename TKey, typename TValue, typename Hash>
bool RobinHoodHashDictionary<TKey, TValue, Hash>::Iterator::operator!=(const Iterator &other) const
{
    return !(*this == other);
}

template <typename TKey, typename TValue, typename Hash>
typename RobinHoodHashDictionary<TKey, TValue, Hash>::Iterator RobinHoodHashDictionary<TKey, TValue, Hash>::begin()
    const
{
    return Iterator(0, &_table);
}

template <typename TKey, typename TValue, typename Hash>
typename RobinHoodHashDictionary<TKey, TValue, Hash>::Iterator RobinHoodHashDictionary<TKey, TValue, Hash>::end() const
{
    return Iterator(_table.size(), &_table);
}

} // namespace dtr

#endif