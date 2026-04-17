#pragma once

#include <functional>
#include <initializer_list>
#include <list>
#include <stdexcept>
#include <utility>
#include <vector>

namespace dtr
{

/// <summary>
/// Словарь на основе хэш-таблицы с разрешением коллизий методом цепочек.
/// Каждый бакет содержит список пар ключ-значение.
/// При превышении коэффициента загрузки выполняется рехеширование.
/// </summary>
/// <typeparam name="TKey">Тип ключа.</typeparam>
/// <typeparam name="TValue">Тип значения.</typeparam>
template <typename TKey, typename TValue> class ChainingHashDictionary : public IDictionary<TKey, TValue>
{
  private:
    using Pair = std::pair<TKey, TValue>;
    using Bucket = std::list<Pair>;

    std::vector<Bucket> _buckets;
    size_t _count;

    static constexpr double _maxLoadFactor = 0.75;

    size_t _getIndex(const TKey &key) const;
    void _rehash();
    void _ensureCapacity();

  public:
    /// <summary>
    /// Создаёт пустой словарь с указанной ёмкостью.
    /// </summary>
    explicit ChainingHashDictionary(size_t capacity = 8);

    /// <summary>
    /// Инициализирует словарь списком пар ключ-значение.
    /// При наличии дублирующихся ключей генерируется исключение.
    /// </summary>
    ChainingHashDictionary(std::initializer_list<Pair> list);

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
        using OuterIt = typename std::vector<Bucket>::iterator;
        using InnerIt = typename Bucket::iterator;

        OuterIt _outer;
        OuterIt _outerEnd;
        InnerIt _inner;

        void _skipEmpty()
        {
            while (_outer != _outerEnd && _inner == _outer->end())
            {
                ++_outer;
                if (_outer != _outerEnd)
                    _inner = _outer->begin();
            }
        }

      public:
        Iterator(OuterIt outer, OuterIt outerEnd) : _outer(outer), _outerEnd(outerEnd)
        {
            if (_outer != _outerEnd)
                _inner = _outer->begin();
            _skipEmpty();
        }

        Pair &operator*()
        {
            return *_inner;
        }

        Iterator &operator++()
        {
            ++_inner;
            _skipEmpty();
            return *this;
        }

        bool operator==(const Iterator &other) const
        {
            return _outer == other._outer && (_outer == _outerEnd || _inner == other._inner);
        }

        bool operator!=(const Iterator &other) const
        {
            return !(*this == other);
        }
    };

    Iterator begin();
    Iterator end();
};

} // namespace dtr

#include "ChainingHashDictionary/ChainingHashDictionary.tpp"