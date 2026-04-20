#ifndef AVLDICTIONARYBALANCE_TPP
#define AVLDICTIONARYBALANCE_TPP

namespace dtr {

// ── Height (публичный) ────────────────────────────────────────────────────────

template<typename TKey, typename TValue>
size_t AvlDictionary<TKey, TValue>::Height() const {
    return static_cast<size_t>(_height(_root));
}

// ── Вспомогательные методы балансировки ──────────────────────────────────────

template<typename TKey, typename TValue>
int AvlDictionary<TKey, TValue>::_height(Node* node) {
    return node ? node->height : 0;
}

template<typename TKey, typename TValue>
void AvlDictionary<TKey, TValue>::_updateHeight(Node* node) {
    node->height = 1 + std::max(_height(node->left), _height(node->right));
}

template<typename TKey, typename TValue>
int AvlDictionary<TKey, TValue>::_getBalance(Node* node) {
    return node ? _height(node->left) - _height(node->right) : 0;
}

/// Проверяет баланс-фактор узла и выполняет нужные повороты:
///
///  bf > +1  — левое поддерево тяжелее:
///    bf(left) < 0  → Left-Right: сначала _rotateLeft(left), затем _rotateRight
///    bf(left) >= 0 → Left-Left:  _rotateRight
///
///  bf < -1  — правое поддерево тяжелее:
///    bf(right) > 0 → Right-Left:  сначала _rotateRight(right), затем _rotateLeft
///    bf(right) <= 0→ Right-Right: _rotateLeft
template<typename TKey, typename TValue>
auto AvlDictionary<TKey, TValue>::_balance(Node* node) -> Node* {
    const int bf = _getBalance(node);

    if (bf > 1) {
        if (_getBalance(node->left) < 0)             // Left-Right
            node->left = _rotateLeft(node->left);
        return _rotateRight(node);                   // Left-Left
    }

    if (bf < -1) {
        if (_getBalance(node->right) > 0)            // Right-Left
            node->right = _rotateRight(node->right);
        return _rotateLeft(node);                    // Right-Right
    }

    return node;
}

// ── Повороты (реализация контракта IBalancedTreeDictionary) ──────────────────

///  До:               После:
///    node              right
///    /  \              /   \
///   A   right  →   node    C
///       /  \        /  \
///      B    C      A    B
template<typename TKey, typename TValue>
auto AvlDictionary<TKey, TValue>::_rotateLeft(Node* node) -> Node* {
    Node* right      = node->right;
    Node* rightLeft  = right->left;

    right->left  = node;
    node->right  = rightLeft;

    _updateHeight(node);   // node опустился — обновляем первым
    _updateHeight(right);  // right стал новым корнем

    return right;
}

///  До:               После:
///      node            left
///      /  \            /  \
///    left  C  →       A   node
///    /  \                 /  \
///   A    B               B    C
template<typename TKey, typename TValue>
auto AvlDictionary<TKey, TValue>::_rotateRight(Node* node) -> Node* {
    Node* left       = node->left;
    Node* leftRight  = left->right;

    left->right = node;
    node->left  = leftRight;

    _updateHeight(node);   // node опустился — обновляем первым
    _updateHeight(left);   // left стал новым корнем

    return left;
}

// ── Вспомогательные методы удаления ──────────────────────────────────────────

template<typename TKey, typename TValue>
auto AvlDictionary<TKey, TValue>::_min(Node* node) -> Node* {
    while (node->left) node = node->left;
    return node;
}

/// Рекурсивно удаляет узел с ключом key из поддерева node.
///
/// Три случая при нахождении целевого узла:
///   1. Нет потомков или один потомок — удаляем узел, возвращаем единственного потомка.
///   2. Два потомка — находим in-order преемника (минимум правого поддерева),
///      копируем его данные в текущий узел, затем рекурсивно удаляем преемника
///      из правого поддерева. Структура дерева сохраняется корректной.
///
/// После рекурсивного вызова пересчитываем высоту и применяем балансировку.
template<typename TKey, typename TValue>
auto AvlDictionary<TKey, TValue>::_remove(Node* node, const TKey& key, bool& removed) -> Node* {
    if (!node) {
        removed = false;
        return nullptr;
    }

    if (key < node->data.first) {
        node->left  = _remove(node->left,  key, removed);
    } else if (key > node->data.first) {
        node->right = _remove(node->right, key, removed);
    } else {
        removed = true;

        if (!node->left || !node->right) {
            // Случай 1: не более одного потомка.
            Node* child = node->left ? node->left : node->right;
            delete node;
            return child;
        }

        // Случай 2: два потомка — заменяем данные in-order преемником.
        Node* successor = _min(node->right);
        node->data = successor->data;

        // Удаляем преемника из правого поддерева. removed уже true —
        // передаём dummy, чтобы не сбросить флаг.
        bool dummy = false;
        node->right = _remove(node->right, successor->data.first, dummy);
    }

    _updateHeight(node);
    return _balance(node);
}

} // namespace dtr

#endif // AVLDICTIONARYBALANCE_TPP
