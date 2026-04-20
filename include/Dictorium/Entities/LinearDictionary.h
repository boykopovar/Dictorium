#ifndef LINEARDICTIONARY_H
#define LINEARDICTIONARY_H

#include <vector>
#include <stdexcept>

namespace dtr{


template<typename TKey, typename TValue>
class IDictionary;

template<typename TKey, typename TValue>
class LinearDictionary : public IDictionary<TKey, TValue> {
public:
    /// <summary>
    /// Инициализирует словарь списком пар ключ-значение.
    /// Использует Add, поэтому запрещает дубликаты ключей.
    /// Сложность: O(n^2)
    /// </summary>
    /// <param name="init">Список пар ключ-значение.</param>
    /// <exception cref="std::invalid_argument">Выбрасывается, если встречается дубликат ключа.</exception>
    LinearDictionary(std::initializer_list<std::pair<TKey, TValue>> init) {
        for (auto i = init.begin(), e = init.end(); i != e; ++i) {
            (*this).Add(i->first, i->second);
        }
    }

    /// <summary>
    /// Проверяет наличие ключа линейным поиском.
    /// Сравнение выполняется через operator==.
    /// Сложность: O(n)
    /// </summary>
    /// <param name="key">Ключ для поиска.</param>
    /// <returns>true, если ключ найден; иначе false.</returns>
    bool ContainsKey(const TKey& key) const override;

    /// <summary>
    /// Пытается получить значение по ключу.
    /// При успехе копирует значение в value.
    /// Сложность: O(n)
    /// </summary>
    /// <param name="key">Ключ для поиска.</param>
    /// <param name="value">Переменная для записи найденного значения.</param>
    /// <returns>true, если ключ найден; иначе false.</returns>
    bool TryGetValue(const TKey& key, TValue& value) const override;

    /// <summary>
    /// Добавляет пару ключ-значение.
    /// Выполняет проверку через ContainsKey.
    /// Использует emplace_back.
    /// Сложность: O(n)
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <param name="value">Значение.</param>
    /// <exception cref="std::invalid_argument">Выбрасывается, если ключ уже существует.</exception>
    void Add(const TKey& key, const TValue& value) override;

    /// <summary>
    /// Вставляет новый элемент или обновляет существующий.
    /// При наличии ключа изменяет значение, иначе добавляет в конец.
    /// Сохраняет порядок элементов.
    /// Сложность: O(n)
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <param name="value">Значение.</param>
    void InsertOrAssign(const TKey& key, const TValue& value) override;

    /// <summary>
    /// Удаляет элемент по ключу.
    /// Использует erase, что приводит к сдвигу элементов.
    /// Сохраняет порядок.
    /// Сложность: O(n)
    /// </summary>
    /// <param name="key">Ключ для удаления.</param>
    /// <returns>true, если элемент был удалён; иначе false.</returns>
    bool Remove(const TKey& key) override;

    /// <summary>
    /// Очищает словарь.
    /// Использует vector::clear.
    /// Сложность: O(n)
    /// </summary>
    void Clear() override;

    /// <summary>
    /// Возвращает количество элементов.
    /// Сложность: O(1)
    /// </summary>
    /// <returns>Количество элементов в словаре.</returns>
    [[nodiscard]] size_t Count() const override;


    /// <summary>
    /// Возвращает ссылку на значение по ключу.
    /// Выполняет линейный поиск.
    /// Сложность: O(n)
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <returns>Ссылка на значение.</returns>
    /// <exception cref="std::out_of_range">Выбрасывается, если ключ не найден.</exception>
    TValue& GetValue(const TKey& key) override;

    /// <summary>
    /// Возвращает константную ссылку на значение по ключу.
    /// Выполняет линейный поиск.
    /// Сложность: O(n)
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <returns>Константная ссылка на значение.</returns>
    /// <exception cref="std::out_of_range">Выбрасывается, если ключ не найден.</exception>
    const TValue& GetValue(const TKey& key) const override;

    auto begin() const { return _dict.begin();}
    auto end() const { return _dict.end();}

    std::ostream& WriteToStream(std::ostream& os) const override {
        if constexpr (!CStreamWritable<TValue> && ! CStreamWritable<TKey>) {
            return os << "<class 'LinearDictionary' TKey=" << typeid(TKey).name() << ", TValue=" << typeid(TValue).name() << '>';
        }
        else {
            return this->_writeItems(os, *this);
        }
    }

private:
    std::vector<std::pair<TKey, TValue>> _dict;
};


}

#include "LinearDictionary/LinearDictionary.tpp"
#include "LinearDictionary/LinearDictionarySetters.tpp"
#include "LinearDictionary/LinearDictionaryGetters.tpp"

#endif //LINEARDICTIONARY_H
