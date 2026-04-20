#ifndef AVLDICTIONARY_H
#define AVLDICTIONARY_H

#include <stack>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <utility>
#include <initializer_list>

#include "Dictorium/Contracts/IBalancedTreeDictionary.h"

namespace dtr {

namespace detail {

/// <summary>
/// Внутренний узел AVL-дерева.
/// Хранит пару ключ-значение, высоту и указатели на потомков.
/// </summary>
template<typename TKey, typename TValue>
struct AvlNode {
    std::pair<TKey, TValue> data;
    AvlNode* left;
    AvlNode* right;
    int height;

    AvlNode(const TKey& key, const TValue& value)
        : data{key, value}, left{nullptr}, right{nullptr}, height{1} {}
};

} // namespace detail


/// <summary>
/// Словарь на основе AVL-дерева.
///
/// AVL-дерево — самобалансирующееся бинарное дерево поиска, в котором
/// высоты левого и правого поддеревьев любого узла различаются не более чем на 1.
/// Баланс поддерживается однократными или двукратными поворотами после каждой
/// операции вставки или удаления.
///
/// Итерация выполняется в порядке возрастания ключей (in-order обход).
/// </summary>
/// <typeparam name="TKey">
///   Тип ключа. Должен полностью упорядочиваться (std::totally_ordered).
/// </typeparam>
/// <typeparam name="TValue">Тип хранимого значения.</typeparam>
template<typename TKey, typename TValue>
class AvlDictionary final
    : public IBalancedTreeDictionary<TKey, TValue, detail::AvlNode<TKey, TValue>>
{
public:
    using Node = detail::AvlNode<TKey, TValue>;

    // ── In-order iterator ────────────────────────────────────────────────────

    /// <summary>
    /// Однонаправленный итератор для обхода пар ключ-значение
    /// в порядке возрастания ключей.
    /// </summary>
    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = std::pair<TKey, TValue>;
        using reference         = std::pair<TKey, TValue>&;
        using pointer           = std::pair<TKey, TValue>*;
        using difference_type   = std::ptrdiff_t;

        /// Конструктор начала обхода: принимает корень дерева.
        explicit Iterator(Node* root) : _current{nullptr} {
            _pushLeft(root);
            _advance();
        }

        /// Конструктор конца обхода (sentinel).
        Iterator() : _current{nullptr} {}

        reference operator*()  const { return  _current->data; }
        pointer   operator->() const { return &_current->data; }

        Iterator& operator++()    { _advance(); return *this; }
        Iterator  operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

        bool operator==(const Iterator& other) const { return _current == other._current; }
        bool operator!=(const Iterator& other) const { return _current != other._current; }

    private:
        std::stack<Node*> _stack;
        Node*             _current;

        /// Проталкивает в стек все узлы по левой ветке от node.
        void _pushLeft(Node* node) {
            while (node) { _stack.push(node); node = node->left; }
        }

        /// Переходит к следующему узлу in-order обхода.
        void _advance() {
            if (_stack.empty()) { _current = nullptr; return; }
            _current = _stack.top();
            _stack.pop();
            _pushLeft(_current->right);
        }
    };

    // ── Конструкторы / Деструктор ─────────────────────────────────────────

    AvlDictionary() : _root{nullptr}, _count{0} {}

    /// <summary>
    /// Инициализирует словарь списком пар ключ-значение.
    /// Использует Add, поэтому запрещает дубликаты ключей.
    /// Сложность: O(n log n)
    /// </summary>
    /// <param name="init">Список пар ключ-значение.</param>
    /// <exception cref="std::invalid_argument">
    ///   Выбрасывается при повторяющихся ключах.
    /// </exception>
    /// <summary>
    /// Инициализирует словарь из диапазона итераторов [first, last).
    /// Ожидает итератор, разыменование которого даёт pair-like тип.
    /// Сложность: O(n log n)
    /// </summary>
    /// <exception cref="std::invalid_argument">При повторяющихся ключах.</exception>
    template<typename TIter>
    AvlDictionary(TIter first, TIter last) : _root{nullptr}, _count{0} {
        for (; first != last; ++first)
            Add(first->first, first->second);
    }

    AvlDictionary(std::initializer_list<std::pair<TKey, TValue>> init)
        : _root{nullptr}, _count{0}
    {
        for (const auto& [key, value] : init)
            Add(key, value);
    }

    ~AvlDictionary() override { _destroy(_root); }

    // ── IDictionary ───────────────────────────────────────────────────────

    /// <summary>
    /// Проверяет наличие ключа.
    /// Сложность: O(log n)
    /// </summary>
    bool ContainsKey(const TKey& key) const override;

    /// <summary>
    /// Пытается получить значение по ключу.
    /// Сложность: O(log n)
    /// </summary>
    bool TryGetValue(const TKey& key, TValue& value) const override;

    /// <summary>
    /// Добавляет новую пару ключ-значение.
    /// Сложность: O(log n)
    /// </summary>
    /// <exception cref="std::invalid_argument">
    ///   Выбрасывается, если ключ уже существует.
    /// </exception>
    void Add(const TKey& key, const TValue& value) override;

    /// <summary>
    /// Вставляет новый элемент или обновляет значение существующего.
    /// Сложность: O(log n)
    /// </summary>
    void InsertOrAssign(const TKey& key, const TValue& value) override;

    /// <summary>
    /// Удаляет элемент по ключу.
    /// Сложность: O(log n)
    /// </summary>
    /// <returns>true, если элемент был найден и удалён; иначе false.</returns>
    bool Remove(const TKey& key) override;

    /// <summary>
    /// Очищает словарь, освобождая всю память.
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
    /// Сложность: O(log n)
    /// </summary>
    /// <exception cref="std::out_of_range">Выбрасывается, если ключ не найден.</exception>
    TValue& GetValue(const TKey& key) override;

    /// <summary>
    /// Возвращает константную ссылку на значение по ключу.
    /// Сложность: O(log n)
    /// </summary>
    /// <exception cref="std::out_of_range">Выбрасывается, если ключ не найден.</exception>
    const TValue& GetValue(const TKey& key) const override;

    // ── IBalancedTreeDictionary ───────────────────────────────────────────

    /// <summary>
    /// Возвращает высоту дерева.
    /// Берётся из поля height корневого узла, сложность O(1).
    /// Высота пустого дерева — 0, дерева из одного узла — 1.
    /// </summary>
    [[nodiscard]] size_t Height() const override;

    // ── Итерация ─────────────────────────────────────────────────────────

    Iterator begin() const { return Iterator{_root}; }
    Iterator end()   const { return Iterator{}; }

    // ── IFormattable ──────────────────────────────────────────────────────

    std::ostream& WriteToStream(std::ostream& os) const override {
        if constexpr (!CStreamWritable<TKey> && !CStreamWritable<TValue>) {
            return os << "<class 'AvlDictionary' TKey="
                      << typeid(TKey).name()
                      << ", TValue=" << typeid(TValue).name() << '>';
        } else {
            return this->_writeItems(os, *this);
        }
    }

protected:
    // ── IBalancedTreeDictionary: повороты ─────────────────────────────────

    /// <summary>
    /// Левый поворот вокруг node. Обновляет высоты затронутых узлов.
    /// Сложность: O(1)
    /// </summary>
    /// <param name="node">Узел, вокруг которого выполняется поворот.</param>
    /// <returns>Новый корень поддерева.</returns>
    Node* _rotateLeft(Node* node) override;

    /// <summary>
    /// Правый поворот вокруг node. Обновляет высоты затронутых узлов.
    /// Сложность: O(1)
    /// </summary>
    /// <param name="node">Узел, вокруг которого выполняется поворот.</param>
    /// <returns>Новый корень поддерева.</returns>
    Node* _rotateRight(Node* node) override;

private:
    Node*  _root;
    size_t _count;

    // ── Вспомогательные методы ────────────────────────────────────────────

    /// Возвращает высоту узла; 0, если node == nullptr.
    static int _height(Node* node);

    /// Пересчитывает и записывает в node->height актуальное значение.
    static void _updateHeight(Node* node);

    /// Возвращает баланс-фактор узла: height(left) - height(right).
    static int _getBalance(Node* node);

    /// Выполняет необходимые повороты и возвращает новый корень поддерева.
    Node* _balance(Node* node);

    /// Итеративный поиск узла с заданным ключом; nullptr, если не найден.
    Node* _find(Node* node, const TKey& key) const;

    /// Рекурсивная вставка с балансировкой.
    /// replace=true — обновить значение при совпадении ключа.
    /// inserted=true — ключ был новым.
    Node* _insert(Node* node, const TKey& key, const TValue& value,
                  bool replace, bool& inserted);

    /// Возвращает узел с минимальным ключом в поддереве.
    static Node* _min(Node* node);

    /// Рекурсивное удаление узла с заданным ключом с балансировкой.
    /// removed=true, если узел был найден и удалён.
    Node* _remove(Node* node, const TKey& key, bool& removed);

    /// Рекурсивно освобождает память всего поддерева.
    static void _destroy(Node* node);
};

} // namespace dtr

#include "AvlDictionary/AvlDictionary.tpp"
#include "AvlDictionary/AvlDictionaryGetters.tpp"
#include "AvlDictionary/AvlDictionarySetters.tpp"
#include "AvlDictionary/AvlDictionaryBalance.tpp"

#endif // AVLDICTIONARY_H