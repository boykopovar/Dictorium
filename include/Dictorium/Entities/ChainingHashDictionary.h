#ifndef CHAININGHASHDICTIONARY_H
#define CHAININGHASHDICTIONARY_H

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
template <typename TKey, typename TValue, typename Hash = std::hash<TKey>>
class ChainingHashDictionary : public IDictionary<TKey, TValue>
{
  private:
    using Pair = std::pair<TKey, TValue>;
    using Bucket = std::list<Pair>;

    Hash _hasher;

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

    template <typename InputIt> ChainingHashDictionary(InputIt first, InputIt last);

    /// <summary>
    /// Проверяет наличие ключа в словаре.
    /// </summary>
    /// <param name="key">Ключ для поиска.</param>
    /// <returns>true, если ключ найден; иначе false.</returns>
    bool ContainsKey(const TKey &key) const override;

    /// <summary>
    /// Пытается получить значение по ключу.
    /// При успехе записывает значение в value.
    /// </summary>
    /// <param name="key">Ключ для поиска.</param>
    /// <param name="value">Переменная для записи значения.</param>
    /// <returns>true, если ключ найден; иначе false.</returns>
    bool TryGetValue(const TKey &key, TValue &value) const override;

    /// <summary>
    /// Добавляет пару ключ-значение в словарь.
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <param name="value">Значение.</param>
    void Add(const TKey &key, const TValue &value) override;

    /// <summary>
    /// Вставляет новую пару или обновляет существующую.
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <param name="value">Значение.</param>
    void InsertOrAssign(const TKey &key, const TValue &value) override;

    /// <summary>
    /// Удаляет элемент по ключу.
    /// </summary>
    /// <param name="key">Ключ для удаления.</param>
    /// <returns>true, если элемент был удалён; иначе false.</returns>
    bool Remove(const TKey &key) override;

    /// <summary>
    /// Очищает словарь.
    /// </summary>
    void Clear() override;

    /// <summary>
    /// Возвращает количество элементов.
    /// </summary>
    /// <returns>Количество элементов.</returns>
    size_t Count() const override;

    /// <summary>
    /// Возвращает ссылку на значение по ключу.
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <returns>Ссылка на значение.</returns>
    TValue &GetValue(const TKey &key) override;

    const TValue &GetValue(const TKey &key) const override;

    // Вернуть кол-во ведер
    int BucketCount() const
    {
        return _buckets.size();
    }

    auto BucketBegin(size_t bucket_index) const
    {
        return _buckets[bucket_index].begin();
    }

    auto BucketEnd(size_t bucket_index) const
    {
        return _buckets[bucket_index].end();
    }

  public:
    class Iterator
    {
      private:
        using OuterIt = typename std::vector<Bucket>::const_iterator;
        using InnerIt = typename Bucket::const_iterator;

        OuterIt _outer;
        OuterIt _outerEnd;
        InnerIt _inner;

        void _skipEmpty()
        {
            while (_outer != _outerEnd && _outer->empty())
                ++_outer;

            if (_outer != _outerEnd)
                _inner = _outer->begin();
        }

      public:
        Iterator(OuterIt outer, OuterIt outerEnd) : _outer(outer), _outerEnd(outerEnd)
        {
            if (_outer != _outerEnd)
                _inner = _outer->begin();
            _skipEmpty();
        }

        const Pair &operator*() const
        {
            return *_inner;
        }

        Iterator &operator++()
        {
            if (_outer == _outerEnd)
                return *this;

            ++_inner;

            while (_outer != _outerEnd && _inner == _outer->end())
            {
                ++_outer;
                if (_outer != _outerEnd)
                    _inner = _outer->begin();
            }

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

    Iterator begin() const;
    Iterator end() const;
};

} // namespace dtr

#include "ChainingHashDictionary/ChainingHashDictionary.tpp"

#endif // CHAININGHASHDICTIONARY_H