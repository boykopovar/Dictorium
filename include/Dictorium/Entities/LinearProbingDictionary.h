#ifndef LINEARPROBINGDICTIONARY_H
#define LINEARPROBINGDICTIONARY_H

#include <functional>
#include <initializer_list>
#include <optional>
#include <stdexcept>
#include <vector>

namespace dtr
{

/// <summary>
/// Словарь на основе хэш-таблицы с открытой адресацией и линейным пробированием.
/// При коллизии используется последовательный поиск следующего слота.
/// Удаление реализовано через tombstone-маркеры.
/// </summary>
template <typename TKey, typename TValue, typename Hash = std::hash<TKey>>
class LinearProbingDictionary : public IDictionary<TKey, TValue>
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

    std::vector<Entry> _table;
    size_t _count;
    Hash _hasher;

    static constexpr double _maxLoadFactor = 0.75;

    size_t _findIndex(const TKey &key) const;
    size_t _findSlotForInsert(const TKey &key);
    void _rehash();
    void _ensureCapacity();

public:
    std::ostream& WriteToStream(std::ostream& os) const override {
    if constexpr (!CStreamWritable<TValue> && ! CStreamWritable<TKey>) {
      return os << "<class 'LinearProbingDictionary' TKey=" << typeid(TKey).name() << ", TValue=" << typeid(TValue).name() << '>';
    }
    else {
      return this->_writeItems(os, *this);
    }
    }

    /// <summary>
    /// Создаёт пустой словарь.
    /// </summary>
    explicit LinearProbingDictionary(size_t capacity = 8);

    /// <summary>
    /// Инициализация списком.
    /// </summary>
    LinearProbingDictionary(std::initializer_list<std::pair<TKey, TValue>> list);

    template <typename InputIt> LinearProbingDictionary(InputIt first, InputIt last);

    bool ContainsKey(const TKey &key) const override;
    bool TryGetValue(const TKey &key, TValue &value) const override;
    void Add(const TKey &key, const TValue &value) override;
    void InsertOrAssign(const TKey &key, const TValue &value) override;
    bool Remove(const TKey &key) override;
    void Clear() override;
    size_t Count() const override;

    TValue &GetValue(const TKey &key) override;
    const TValue &GetValue(const TKey &key) const override;

    class Iterator
    {
    private:
        size_t _index;
        const std::vector<Entry> *_table;
        mutable std::optional<std::pair<const TKey, TValue>> _current;

        void _skip();
        void _updateCurrent() const {
            if (_index < _table->size()) {
                _current.emplace((*_table)[_index].key, (*_table)[_index].value);
            }
        }


    public:
        Iterator(size_t index, const std::vector<Entry> *table);


        const std::pair<const TKey, TValue>& operator*() const {
            _updateCurrent();
            return *_current;
        }

        const std::pair<const TKey, TValue>* operator->() const {
            _updateCurrent();
            return &*_current;
        }

        Iterator &operator++();

        bool operator==(const Iterator &other) const;
        bool operator!=(const Iterator &other) const;
    };

    Iterator begin() const;
    Iterator end() const;
};

template<CHashable TKey, typename TValue, typename Hash>
    auto begin(LinearProbingDictionary<TKey, TValue, Hash>& dict) noexcept {
  return dict.begin();
}

template<CHashable TKey, typename TValue, typename Hash>
auto end(LinearProbingDictionary<TKey, TValue, Hash>& dict) noexcept {
  return dict.end();
}

template<CHashable TKey, typename TValue, typename Hash>
auto begin(const LinearProbingDictionary<TKey, TValue, Hash>& dict) noexcept {
  return dict.begin();
}

template<CHashable TKey, typename TValue, typename Hash>
auto end(const LinearProbingDictionary<TKey, TValue, Hash>& dict) noexcept {
  return dict.end();
}

} // namespace dtr

#include "LinearProbingDictionary/LinearProbingDictionary.tpp"

#endif