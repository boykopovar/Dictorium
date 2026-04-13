#ifndef IDICTIONARY_H
#define IDICTIONARY_H

#include <vector>
#include <type_traits>


#include "DictProxy.h"
#include "IFormattable.h"
#include "Dictorium/Entities/Console.h"


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
    std::ostream& WriteToStream(std::ostream& os) const override {
        return os << "<class 'IDictionary' TKey=" << typeid(TKey).name() << ", TValue=" << typeid(TValue).name() << '>';
    }

    /// <summary>
    /// Предоставляет доступ к значению по ключу. Ссылается на `GetValue`.
    /// </summary>
    /// <param name="key">Ключ.</param>
    /// <returns>Ссылка для доступа к значению.</returns>
    TValue& operator[](const TKey& key) {
        return GetValue(key);
    }


protected:
    friend class DictProxy<TKey, TValue>;

    template<typename TRange>
    requires CPairIterator<decltype(std::begin(std::declval<TRange>())), TKey, TValue>
    std::ostream& _writeItems(std::ostream& os, const TRange& range) const  {
        bool first = true;
        os << '{' << '\n';

        for (auto& [key, value] : range) {
            if (!first) os << ',' << '\n';
            first = false;
            os << "    ";

            if constexpr (CStreamWritable<TKey>) os << '"' << key << '"';
            else os << "<TKey=" << typeid(TKey).name() << '>';

            os << ": ";

            if constexpr (std::is_same_v<TValue, std::string>) os << '"' << value << '"';
            else if constexpr (CStreamWritable<TValue>) os << value;
            else os << "<TValue=" << typeid(value).name() << '>';
        }
        return os << '\n' << '}';
    }

    template<typename TRange>
    requires CValuesIterator<decltype(std::begin(std::declval<TRange>())), TValue>
    std::ostream& _writeValues(std::ostream& os, const TRange& range) const {
        if constexpr (!CStreamWritable<TValue>) {
            return os << " TValue=" << typeid(TValue).name();
        }
        else {
            os << '[';
            bool first = true;
            for (auto& value : range) {
                if (!first) {
                    os << ',' << ' ';
                    first = false;
                }
                os << value;
            }

            return os << ']';
        }
    }

};

}

#endif //IDICTIONARY_H
