#ifndef AVLDICTIONARYSETTERS_TPP
#define AVLDICTIONARYSETTERS_TPP

namespace dtr {

template<typename TKey, typename TValue>
void AvlDictionary<TKey, TValue>::Add(const TKey& key, const TValue& value) {
    bool inserted = false;
    _root = _insert(_root, key, value, /*replace=*/false, inserted);
    if (!inserted) throw std::invalid_argument("Key already exists");
    ++_count;
}

template<typename TKey, typename TValue>
void AvlDictionary<TKey, TValue>::InsertOrAssign(const TKey& key, const TValue& value) {
    bool inserted = false;
    _root = _insert(_root, key, value, /*replace=*/true, inserted);
    if (inserted) ++_count;
}

// ── Внутренняя вставка с балансировкой ───────────────────────────────────────

/// Рекурсивно вставляет пару (key, value) в поддерево node.
///
/// Если replace == true и ключ уже существует, значение обновляется на месте,
/// структура дерева не меняется, балансировка не нужна.
/// Если replace == false и ключ уже существует, inserted остаётся false,
/// дерево не изменяется.
///
/// После вставки в один из потомков пересчитывает высоту узла и,
/// при необходимости, выполняет балансировку.
template<typename TKey, typename TValue>
auto AvlDictionary<TKey, TValue>::_insert(
    Node*        node,
    const TKey&  key,
    const TValue& value,
    bool         replace,
    bool&        inserted
) -> Node* {
    if (!node) {
        inserted = true;
        return new Node(key, value);
    }

    if (key < node->data.first) {
        node->left  = _insert(node->left,  key, value, replace, inserted);
    } else if (key > node->data.first) {
        node->right = _insert(node->right, key, value, replace, inserted);
    } else {
        // Ключ уже существует — структура дерева не меняется.
        inserted = false;
        if (replace) node->data.second = value;
        return node;
    }

    _updateHeight(node);
    return _balance(node);
}

} // namespace dtr

#endif // AVLDICTIONARYSETTERS_TPP
