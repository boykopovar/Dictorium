#ifndef IDICTIONARY_H
#define IDICTIONARY_H

#include <vector>
#include <type_traits>


#include "DictProxy.h"
#include "IEnumerable/IEnumerable.h"
#include "IEnumerable/Iterator.h"
#include "ItemsRange.h"
#include "IFormattable.h"
#include "../Entities/Console.h"

namespace dtr {


template<typename TKey, typename TValue>
class IDictionary : public IFormattable {
public:
    IDictionary() = default;
    ~IDictionary() override = default;

    /// <summary>
    /// Проверяет наличие ключа в словаре.
    /// </summary>
    /// <param name="key">Ключ для поиска.</param>
    /// <returns>true, если ключ найден; иначе false.</returns>
    virtual bool ContainsKey(const TKey& key) const = 0;

    /// <summary>
    /// Пытается получить значение по ключу.
    /// При успехе записывает значение в value.
    /// </summary>
    /// <param name="key">Ключ для поиска.</param>
    /// <param name="value">Переменная для записи значения.</param>
    /// <returns>true, если ключ найден; иначе false.</returns>
    virtual bool TryGetValue(const TKey& key, TValue& value) const = 0;

    /// <summary>
    /// Добавляет пару ключ-значение в словарь.
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <param name="value">Значение.</param>
    virtual void Add(const TKey& key, const TValue& value) = 0;

    /// <summary>
    /// Вставляет новую пару или обновляет существующую.
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <param name="value">Значение.</param>
    virtual void InsertOrAssign(const TKey& key, const TValue& value) = 0;

    /// <summary>
    /// Удаляет элемент по ключу.
    /// </summary>
    /// <param name="key">Ключ для удаления.</param>
    /// <returns>true, если элемент был удалён; иначе false.</returns>
    virtual bool Remove(const TKey& key) = 0;

    /// <summary>
    /// Очищает словарь.
    /// </summary>
    virtual void Clear() = 0;

    /// <summary>
    /// Возвращает количество элементов.
    /// </summary>
    /// <returns>Количество элементов.</returns>
    [[nodiscard]] virtual size_t Count() const = 0;

    /// <summary>
    /// Возвращает ссылку на значение по ключу.
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <returns>Ссылка на значение.</returns>
    virtual TValue& GetValue(const TKey& key) = 0;

    virtual const TValue& GetValue(const TKey& key) const = 0;

    /// <summary>
    /// Выводит содержимое словаря в поток.
    /// </summary>
    /// <param name="os">Выходной поток.</param>
    void WriteToStream(std::ostream& os) const override {
        if constexpr (!StreamWritable<TKey> && !StreamWritable<TValue>){
            os << "<class 'IDictionary': TKey=" << typeid(TKey).name() << ", TValue=" << typeid(TValue).name() << '>';
        }
        else
        {
            bool first = true;
            os << '{' << '\n';

            for (auto& [key, value] : Items()) {
                if (!first) os << ',' << '\n';
                first = false;
                os << "    ";

                if constexpr (StreamWritable<TKey>) os << '"' << key << '"';
                else os << "<TKey=" << typeid(TKey).name() << '>';

                os << ": ";

                if constexpr (std::is_same_v<TValue, std::string>) os << '"' << value << '"';
                else if constexpr (StreamWritable<TValue>) os << value;
                else os << "<TValue=" << typeid(value).name() << '>';
            }
            os << '\n' << '}';
        }
    }

    /// <summary>
    /// Возвращает диапазон для ленивой итерации по элементам словаря.
    /// </summary>
    /// <returns>Диапазон элементов.</returns>
    ItemsRange<TKey, TValue> Items() const {
        return ItemsRange(_getItemsEnumerator());
    }

    /// <summary>
    /// Предоставляет доступ к значению по ключу через прокси. Ссылается на `GetValue`.
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <returns>Прокси-объект для доступа к значению.</returns>
    DictProxy<TKey, TValue> operator[](const TKey& key) noexcept {
        return DictProxy<TKey, TValue>(this, key);
    }


protected:
    /// <summary>
    /// Возвращает IEnumerator элементов словаря.
    /// </summary>
    /// <returns>Указатель на IEnumerator.</returns>
    virtual std::unique_ptr<IEnumerator<std::pair<TKey, TValue>>> _getItemsEnumerator() const = 0;
    friend class DictProxy<TKey, TValue>;
};

}

#endif //IDICTIONARY_H
