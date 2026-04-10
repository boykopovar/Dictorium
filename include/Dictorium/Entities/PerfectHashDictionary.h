#ifndef PERFECTHASHDICTIONARY_H
#define PERFECTHASHDICTIONARY_H

#include <random>
#include <vector>
#include <utility>
#include <stdexcept>

#include"Dictorium/Contracts/Concepts.h"
#include "PerfectHashDictionary/PerfectHashIterator.tpp"

#define PERFECTHASH_SALT 2654435761ULL
#define PERFECTHASH_DEPRECATED_POSTFIX "This method of PerfectHashDictionary possibly triggers full hard rebuild. Prefer initializer_list constructor."
#define PERFECTHASH_DICT_NAME "PerfectHashDictionary"
#define PERFECTHASH_DEPRECATED_KEYS "Valid only for keys from the original set"

namespace dtr{

struct PhBucket {
    uint64_t Offset;
    uint64_t Size;
    uint64_t Seed;
};

template<typename TKey, typename TValue>
struct PhSlot {
    std::pair<TKey, TValue> Item;
    bool Exists;
};

template<typename TKey, typename TValue>
class IDictionary;

template<CHashable TKey, typename TValue>
class PerfectHashDictionary : public IDictionary<TKey, TValue> {
public:
    PerfectHashDictionary() = delete;

    /// <summary>
    /// Создаёт словарь из initializer_list.
    /// Выполняет построение идеального хеша.
    /// Сложность: O(n^1.5) с очень большой константой
    /// </summary>
    PerfectHashDictionary(std::initializer_list<std::pair<TKey, TValue>> init) {
        _build(init.begin(), init.end(), init.size());
    }

    /// <summary>
    /// Создаёт словарь из вектора пар ключ-значение.
    /// Выполняет построение идеального хеша.
    /// Сложность: O(n^1.5) с очень большой константой
    /// </summary>
    PerfectHashDictionary(std::vector<std::pair<TKey, TValue>> init){
        _build(init.begin(), init.end(), init.size());
    }

    /// <summary>
    /// Создаёт словарь из диапазона итераторов.
    /// Выполняет построение идеального хеша.
    /// Сложность: O(n^1.5) с очень большой константой
    /// </summary>
    template<CPairIterator<TKey, TValue> TIter>
    PerfectHashDictionary(TIter begin, TIter end){
        _build(begin, end, std::distance(begin, end));
    }

    /// <summary>
    /// Проверяет наличие ключа в словаре.
    /// Сложность: O(1)
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <returns>true, если ключ существует; иначе false.</returns>
    bool ContainsKey(const TKey& key) const override;

    /// <summary>
    /// Пытается получить значение по ключу.
    /// Не выполняет полной проверки совпадения ключа. Ожидается ключ из исходного множества.
    /// Сложность: O(1)
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <param name="value">Выходное значение.</param>
    /// <returns>true, если ключ найден; иначе false.</returns>
    bool TryGetValue(const TKey& key, TValue& value) const override;


    /// <summary>
    /// Пытается получить значение по ключу c полной проверкой ключа.
    /// Сложность: O(1)
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <param name="value">Выходное значение.</param>
    /// <returns>true, если ключ найден; иначе false.</returns>
    bool TryGetValidatedValue(const TKey& key, TValue& value) const;

    /// <summary>
    /// Добавляет элемент в словарь.
    /// Почти всегда приводит к полной перестройке словаря. Если ключ был ранее удален, то O(1).
    /// Сложность: O(n^1.5) с очень большой константой
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <param name="value">Значение.</param>
    [[deprecated(PERFECTHASH_DEPRECATED_POSTFIX)]]
    void Add(const TKey& key, const TValue& value) override;

    /// <summary>
    /// Вставляет элемент или обновляет существующий.
    /// Может привести к полной перестройке структуры. Если ключ сущетсвует, то O(1).
    /// Сложность: O(n^1.5) с очень большой константой, если ключа не было в словаре.
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <param name="value">Значение.</param>
    [[deprecated(PERFECTHASH_DEPRECATED_POSTFIX)]]
    void InsertOrAssign(const TKey& key, const TValue& value) override;

    /// <summary>
    /// Логически удаляет элемент (tombstone).
    /// Элемент не удаляется физически.
    /// Сложность: O(1)
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <returns>true, если элемент был найден и помечен удалённым.</returns>
    [[deprecated("Lazy removal (tombstone). Element is not phisically removed.")]]
    bool Remove(const TKey& key) override;

    /// <summary>
    /// Очищает словарь.
    /// Сложность: O(n)
    /// </summary>
    void Clear() override;

    /// <summary>
    /// Возвращает количество элементов.
    /// Сложность: O(1)
    /// </summary>
    [[nodiscard]] size_t Count() const override;

    /// <summary>
    /// Возвращает ссылку на значение по ключу.
    /// Не выполняет полной проверки совпадения ключа. Ожидается ключ из исходного множества.
    /// Сложность: O(1)
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <returns>Ссылка на значение.</returns>
    /// <exception cref="std::out_of_range">Если слот не существует.</exception>
    TValue& GetValue(const TKey& key) override;

    /// <summary>
    /// Возвращает ссылку на значение по ключу с полной проверкой ключа.
    /// Сложность: O(1)
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <returns>Ссылка на значение.</returns>
    /// <exception cref="std::out_of_range">Если ключ не найден.</exception>
    TValue& GetValidatedValue(const TKey& key);

    /// <summary>
    /// Возвращает ссылку на значение по ключу.
    /// Не выполняет полной проверки совпадения ключа. Ожидается ключ из исходного множества.
    /// Сложность: O(1)
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <returns>Ссылка на значение.</returns>
    /// <exception cref="std::out_of_range">Если слот не существует.</exception>
    const TValue& GetValue(const TKey& key) const override;

    /// <summary>
    /// Возвращает ссылку на значение по ключу с полной проверкой ключа.
    /// Сложность: O(1)
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <returns>Ссылка на значение.</returns>
    /// <exception cref="std::out_of_range">Если ключ не найден.</exception>
    const TValue& GetValidatedValue(const TKey& key) const;

    std::ostream& WriteToStream(std::ostream& os) const override {
        if constexpr (!CStreamWritable<TValue> && ! CStreamWritable<TKey>) {
            return os << "<class 'PerfectHashDictionary' TKey=" << typeid(TKey).name() << ", TValue=" << typeid(TValue).name() << '>';
        }
        else {
            return this->_writeItems(os, *this);
        }
    }

    PerfectHashIterator<TKey, TValue> begin(){ return {_values, 0}; }
    PerfectHashIterator<TKey, TValue> end(){ return {_values, _values.size()}; }

    PerfectHashIterator<TKey, TValue> begin() const { return {_values, 0}; }
    PerfectHashIterator<TKey, TValue> end() const { return {_values, _values.size()}; }

private:

    static inline size_t _fastRange(const uint64_t hash, const size_t count) {
        return ((__uint128_t)hash * count) >> 64;
    }

    static inline size_t _hashRaw(const uint64_t stdHash, const uint64_t seed, const size_t tableSize) {
        return _fastRange((seed * PERFECTHASH_SALT + 1) * stdHash + seed, tableSize);
    }

    size_t _hash(const TKey& key, const uint64_t seed, const size_t tableSize) const {
        return _hashRaw(std::hash<TKey>{}(key), seed, tableSize);
    }

    uint64_t _globalSeed;
    uint64_t _count;
    uint64_t _tableSize;

    std::vector<PhBucket> _buckets;
    std::vector<PhSlot<TKey, TValue>> _values;

    [[nodiscard]] uint64_t _randomNum() const;
    uint64_t _findSeed(const std::vector<std::pair<TKey, TValue>>& bucket, size_t tableSize) const;
    [[nodiscard]] size_t _nextPrime(size_t n) const;

    uint64_t _findIndex(const TKey& key) const;

    template<CPairIterator<TKey, TValue> TIter>
    void _build(TIter begin, TIter end, size_t size);

    const PhSlot<TKey, TValue>& _getExistedSlot(const TKey& key) const {
        auto flatIndex = _findIndex(key);
        if (flatIndex == -1) throw std::out_of_range("Key not found");

        auto& slot = _values[flatIndex];
        if (!slot.Exists) throw std::out_of_range("Key not found");
        return slot;
    }
};

#include "PerfectHashDictionary/PerfectHashDictionaryBuild.tpp"
#include "PerfectHashDictionary/PerfectHashDictionaryGetters.tpp"
#include "PerfectHashDictionary/PerfectHashDictionarySetters.tpp"

}
#endif //PERFECTHASHDICTIONARY_H
