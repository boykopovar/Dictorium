#ifndef AVLDICTIONARYGETTERS_TPP
#define AVLDICTIONARYGETTERS_TPP

namespace dtr {

template<typename TKey, typename TValue>
TValue& AvlDictionary<TKey, TValue>::GetValue(const TKey& key) {
    Node* node = _find(_root, key);
    if (!node) throw std::out_of_range("Key not found");
    return node->data.second;
}

template<typename TKey, typename TValue>
const TValue& AvlDictionary<TKey, TValue>::GetValue(const TKey& key) const {
    Node* node = _find(_root, key);
    if (!node) throw std::out_of_range("Key not found");
    return node->data.second;
}

template<typename TKey, typename TValue>
bool AvlDictionary<TKey, TValue>::TryGetValue(const TKey& key, TValue& value) const {
    Node* node = _find(_root, key);
    if (!node) return false;
    value = node->data.second;
    return true;
}

// ── Внутренний поиск ──────────────────────────────────────────────────────────

template<typename TKey, typename TValue>
auto AvlDictionary<TKey, TValue>::_find(Node* node, const TKey& key) const -> Node* {
    while (node) {
        if      (key < node->data.first) node = node->left;
        else if (key > node->data.first) node = node->right;
        else                             return node;
    }
    return nullptr;
}

} // namespace dtr

#endif // AVLDICTIONARYGETTERS_TPP
