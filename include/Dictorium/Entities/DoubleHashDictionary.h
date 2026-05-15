#ifndef DOUBLEHASHDICTIONARY_H
#define DOUBLEHASHDICTIONARY_H

#include <functional>
#include <initializer_list>
#include <optional>
#include <stdexcept>
#include <vector>

namespace dtr
{

/// <summary>
/// Словарь на основе хэш-таблицы с открытой адресацией
/// и двойным хешированием.
///
/// Формула пробирования:
/// pos = (h1(key) + i * h2(key)) % capacity
///
/// h2(key) никогда не должен возвращать 0.
/// </summary>
template <typename TKey, typename TValue, typename Hash1 = std::hash<TKey>, typename Hash2 = std::hash<TKey>>
class DoubleHashDictionary : public IDictionary<TKey, TValue>
{
  private:
    enum class EntryState
    {
        Empty,
        Occupied,
        Deleted
    };

    struct Entry
    {
        TKey key;
        TValue value;
        EntryState state = EntryState::Empty;
    };

  private:
    std::vector<Entry> _table;
    size_t _count;

    Hash1 _hasher1;
    Hash2 _hasher2;

    static constexpr double _maxLoadFactor = 0.75;

  private:
    /// <summary>
    /// Основной хэш.
    /// </summary>
    size_t _hash1(const TKey &key) const
    {
        return _hasher1(key) % _table.size();
    }

    /// <summary>
    /// Вторичный хэш.
    /// Никогда не возвращает 0.
    /// </summary>
    size_t _hash2(const TKey &key) const
    {
        return 1 + (_hasher2(key) % (_table.size() - 1));
    }

    /// <summary>
    /// Поиск индекса элемента.
    /// </summary>
    size_t _findIndex(const TKey &key) const;

    /// <summary>
    /// Поиск слота для вставки.
    /// </summary>
    size_t _findSlotForInsert(const TKey &key);

    /// <summary>
    /// Увеличение таблицы и повторное хеширование.
    /// </summary>
    void _rehash();

    /// <summary>
    /// Проверка необходимости расширения таблицы.
    /// </summary>
    void _ensureCapacity();

  public:
    std::ostream &WriteToStream(std::ostream &os) const override
    {
        if constexpr (!CStreamWritable<TValue> && !CStreamWritable<TKey>)
        {
            return os << "<class 'DoubleHashDictionary' TKey=" << typeid(TKey).name()
                      << ", TValue=" << typeid(TValue).name() << '>';
        }
        else
        {
            return this->_writeItems(os, *this);
        }
    }

  public:
    /// <summary>
    /// Создаёт пустой словарь.
    /// </summary>
    explicit DoubleHashDictionary(size_t capacity = 11);

    /// <summary>
    /// Инициализация списком.
    /// </summary>
    DoubleHashDictionary(std::initializer_list<std::pair<TKey, TValue>> list);

    template <typename InputIt> DoubleHashDictionary(InputIt first, InputIt last);

  public:
    bool ContainsKey(const TKey &key) const override;

    bool TryGetValue(const TKey &key, TValue &value) const override;

    void Add(const TKey &key, const TValue &value) override;

    void InsertOrAssign(const TKey &key, const TValue &value) override;

    bool Remove(const TKey &key) override;

    void Clear() override;

    size_t Count() const override;

    TValue &GetValue(const TKey &key) override;

    const TValue &GetValue(const TKey &key) const override;

  public:
    class Iterator
    {
      private:
        size_t _index;
        const std::vector<Entry> *_table;

        mutable std::optional<std::pair<const TKey, TValue>> _current;

      private:
        void _skip();

        void _updateCurrent() const
        {
            if (_index < _table->size())
            {
                _current.emplace((*_table)[_index].key, (*_table)[_index].value);
            }
        }

      public:
        Iterator(size_t index, const std::vector<Entry> *table);

        const std::pair<const TKey, TValue> &operator*() const
        {
            _updateCurrent();
            return *_current;
        }

        const std::pair<const TKey, TValue> *operator->() const
        {
            _updateCurrent();
            return &*_current;
        }

        Iterator &operator++();

        bool operator==(const Iterator &other) const;

        bool operator!=(const Iterator &other) const;
    };

  public:
    Iterator begin() const;

    Iterator end() const;

    static bool _isPrime(size_t n);
    static size_t _nextPrime(size_t n);
};

template <CHashable TKey, typename TValue, typename Hash1, typename Hash2>
auto begin(DoubleHashDictionary<TKey, TValue, Hash1, Hash2> &dict) noexcept
{
    return dict.begin();
}

template <CHashable TKey, typename TValue, typename Hash1, typename Hash2>
auto end(DoubleHashDictionary<TKey, TValue, Hash1, Hash2> &dict) noexcept
{
    return dict.end();
}

template <CHashable TKey, typename TValue, typename Hash1, typename Hash2>
auto begin(const DoubleHashDictionary<TKey, TValue, Hash1, Hash2> &dict) noexcept
{
    return dict.begin();
}

template <CHashable TKey, typename TValue, typename Hash1, typename Hash2>
auto end(const DoubleHashDictionary<TKey, TValue, Hash1, Hash2> &dict) noexcept
{
    return dict.end();
}

} // namespace dtr

#include "DoubleHashDictionary/DoubleHashDictionary.tpp"

#endif