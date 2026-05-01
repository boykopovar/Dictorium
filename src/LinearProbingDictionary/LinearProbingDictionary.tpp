#ifndef LINEARPROBINGDICTIONARY_TPP
#define LINEARPROBINGDICTIONARY_TPP

namespace dtr
{

template <typename TKey, typename TValue, typename Hash>
LinearProbingDictionary<TKey, TValue, Hash>::LinearProbingDictionary(size_t capacity) : _table(capacity), _count(0)
{
}

template <typename TKey, typename TValue, typename Hash>
LinearProbingDictionary<TKey, TValue, Hash>::LinearProbingDictionary(
    std::initializer_list<std::pair<TKey, TValue>> list)
    : _table(8), _count(0)
{
    for (const auto &[k, v] : list)
        Add(k, v);
}

template <typename TKey, typename TValue, typename Hash>
template <typename InputIt>
LinearProbingDictionary<TKey, TValue, Hash>::LinearProbingDictionary(InputIt first, InputIt last) : _table(8), _count(0)
{
    for (auto it = first; it != last; ++it)
        Add(it->first, it->second);
}

template <typename TKey, typename TValue, typename Hash>
size_t LinearProbingDictionary<TKey, TValue, Hash>::_findIndex(const TKey &key) const
{
    size_t capacity = _table.size();
    size_t index = _hasher(key) % capacity;

    for (size_t i = 0; i < capacity; i++)
    {
        size_t pos = (index + i) % capacity;
        const auto &e = _table[pos];

        if (e.state == EntryState::Empty)
            return capacity;

        if (e.state == EntryState::Occupied && e.key == key)
            return pos;
    }
    return capacity;
}

template <typename TKey, typename TValue, typename Hash>
size_t LinearProbingDictionary<TKey, TValue, Hash>::_findSlotForInsert(const TKey &key)
{
    size_t capacity = _table.size();
    size_t index = _hasher(key) % capacity;

    size_t firstDeleted = capacity;

    for (size_t i = 0; i < capacity; i++)
    {
        size_t pos = (index + i) % capacity;
        auto &e = _table[pos];

        if (e.state == EntryState::Occupied && e.key == key)
            return pos;

        if (e.state == EntryState::Deleted && firstDeleted == capacity)
            firstDeleted = pos;

        if (e.state == EntryState::Empty)
            return (firstDeleted != capacity) ? firstDeleted : pos;
    }
    return capacity;
}

template <typename TKey, typename TValue, typename Hash>
void LinearProbingDictionary<TKey, TValue, Hash>::_ensureCapacity()
{
    if ((double)_count / _table.size() > _maxLoadFactor)
        _rehash();
}

template <typename TKey, typename TValue, typename Hash> void LinearProbingDictionary<TKey, TValue, Hash>::_rehash()
{
    std::vector<Entry> old = std::move(_table);
    _table = std::vector<Entry>(old.size() * 2);
    _count = 0;

    for (const auto &e : old)
    {
        if (e.state == EntryState::Occupied)
            InsertOrAssign(e.key, e.value);
    }
}

template <typename TKey, typename TValue, typename Hash>
bool LinearProbingDictionary<TKey, TValue, Hash>::ContainsKey(const TKey &key) const
{
    return _findIndex(key) != _table.size();
}

template <typename TKey, typename TValue, typename Hash>
bool LinearProbingDictionary<TKey, TValue, Hash>::TryGetValue(const TKey &key, TValue &value) const
{
    size_t pos = _findIndex(key);
    if (pos == _table.size())
        return false;

    value = _table[pos].value;
    return true;
}

template <typename TKey, typename TValue, typename Hash>
void LinearProbingDictionary<TKey, TValue, Hash>::Add(const TKey &key, const TValue &value)
{
    _ensureCapacity();

    size_t pos = _findSlotForInsert(key);

    if (_table[pos].state == EntryState::Occupied)
        throw std::invalid_argument("Key exists");

    _table[pos] = {key, value, EntryState::Occupied};
    _count++;
}

template <typename TKey, typename TValue, typename Hash>
void LinearProbingDictionary<TKey, TValue, Hash>::InsertOrAssign(const TKey &key, const TValue &value)
{
    _ensureCapacity();

    size_t pos = _findSlotForInsert(key);

    if (_table[pos].state == EntryState::Occupied)
    {
        _table[pos].value = value;
        return;
    }

    _table[pos] = {key, value, EntryState::Occupied};
    _count++;
}

template <typename TKey, typename TValue, typename Hash>
bool LinearProbingDictionary<TKey, TValue, Hash>::Remove(const TKey &key)
{
    size_t pos = _findIndex(key);
    if (pos == _table.size())
        return false;

    _table[pos].state = EntryState::Deleted;
    _count--;
    return true;
}

template <typename TKey, typename TValue, typename Hash> void LinearProbingDictionary<TKey, TValue, Hash>::Clear()
{
    _table.clear();
    _table.resize(8);
    _count = 0;
}

template <typename TKey, typename TValue, typename Hash>
size_t LinearProbingDictionary<TKey, TValue, Hash>::Count() const
{
    return _count;
}

template <typename TKey, typename TValue, typename Hash>
TValue &LinearProbingDictionary<TKey, TValue, Hash>::GetValue(const TKey &key)
{
    size_t pos = _findIndex(key);
    if (pos == _table.size())
        throw std::out_of_range("Key not found");

    return _table[pos].value;
}

template <typename TKey, typename TValue, typename Hash>
const TValue &LinearProbingDictionary<TKey, TValue, Hash>::GetValue(const TKey &key) const
{
    size_t pos = _findIndex(key);
    if (pos == _table.size())
        throw std::out_of_range("Key not found");

    return _table[pos].value;
}

template <typename TKey, typename TValue, typename Hash>
void LinearProbingDictionary<TKey, TValue, Hash>::Iterator::_skip()
{
    while (_index < _table->size() && (*_table)[_index].state != EntryState::Occupied)
        _index++;
}

template <typename TKey, typename TValue, typename Hash>
LinearProbingDictionary<TKey, TValue, Hash>::Iterator::Iterator(size_t index, const std::vector<Entry> *table)
    : _index(index), _table(table)
{
    _skip();
}

template <typename TKey, typename TValue, typename Hash>
const std::pair<const TKey &, const TValue &> LinearProbingDictionary<TKey, TValue, Hash>::Iterator::operator*() const
{
    const auto &e = (*_table)[_index];
    return {e.key, e.value};
}

template <typename TKey, typename TValue, typename Hash>
typename LinearProbingDictionary<TKey, TValue, Hash>::Iterator &LinearProbingDictionary<TKey, TValue,
                                                                                        Hash>::Iterator::operator++()
{
    _index++;
    _skip();
    return *this;
}

template <typename TKey, typename TValue, typename Hash>
bool LinearProbingDictionary<TKey, TValue, Hash>::Iterator::operator==(const Iterator &other) const
{
    return _index == other._index && _table == other._table;
}

template <typename TKey, typename TValue, typename Hash>
bool LinearProbingDictionary<TKey, TValue, Hash>::Iterator::operator!=(const Iterator &other) const
{
    return !(*this == other);
}

template <typename TKey, typename TValue, typename Hash>
typename LinearProbingDictionary<TKey, TValue, Hash>::Iterator LinearProbingDictionary<TKey, TValue, Hash>::begin()
    const
{
    return Iterator(0, &_table);
}

template <typename TKey, typename TValue, typename Hash>
typename LinearProbingDictionary<TKey, TValue, Hash>::Iterator LinearProbingDictionary<TKey, TValue, Hash>::end() const
{
    return Iterator(_table.size(), &_table);
}

} // namespace dtr

#endif