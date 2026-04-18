#ifndef IBALANCEDTREEDICTIONARY_H
#define IBALANCEDTREEDICTIONARY_H

#include <concepts>

#include "IDictionary.h"

namespace dtr {

/// <summary>
/// Базовый контракт для словарей на основе самобалансирующихся деревьев.
///
/// Расширяет IDictionary, гарантируя реализацию:
///   - левого и правого поворотов узлов дерева;
///   - получения текущей высоты дерева.
///
/// Тип узла (TNode) является параметром шаблона.
///
/// Итерация по элементам осуществляется через begin() / end(),
/// которые должны быть реализованы в конкретном классе-наследнике.
/// </summary>
/// <typeparam name="TKey">
///   Тип ключа. Должен поддерживать полный порядок сравнения (operator&lt;, operator&gt; и др.),
///   так как дерево поиска опирается на упорядоченность ключей.
/// </typeparam>
/// <typeparam name="TValue">Тип хранимого значения.</typeparam>
/// <typeparam name="TNode">Тип внутреннего узла дерева.</typeparam>
template<typename TKey, typename TValue, typename TNode>
requires std::totally_ordered<TKey>
class IBalancedTreeDictionary : public IDictionary<TKey, TValue> {
public:
    IBalancedTreeDictionary() = default;
    ~IBalancedTreeDictionary() override = default;

    /// <summary>
    /// Возвращает высоту дерева.
    /// Сложность: определяется реализацией (O(1) при хранении в узле, O(n) при обходе).
    /// </summary>
    /// <returns>Высота дерева в уровнях.</returns>
    [[nodiscard]] virtual size_t Height() const = 0;

protected:
    /// <summary>
    /// Выполняет левый поворот вокруг указанного узла.
    /// </summary>
    /// <param name="node">Узел, вокруг которого выполняется поворот. Не должен быть nullptr.</param>
    /// <returns>Новый корень поддерева.</returns>
    virtual TNode* _rotateLeft(TNode* node) = 0;

    /// <summary>
    /// Выполняет правый поворот вокруг указанного узла.
    /// </summary>
    /// <param name="node">Узел, вокруг которого выполняется поворот. Не должен быть nullptr.</param>
    /// <returns>Новый корень поддерева.</returns>
    virtual TNode* _rotateRight(TNode* node) = 0;
};

}

#endif //IBALANCEDTREEDICTIONARY_H