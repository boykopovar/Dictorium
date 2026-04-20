#ifndef AVLDICTIONARY_TPP
#define AVLDICTIONARY_TPP

namespace dtr {

template<typename TKey, typename TValue>
bool AvlDictionary<TKey, TValue>::ContainsKey(const TKey& key) const {
    return _find(_root, key) != nullptr;
}

template<typename TKey, typename TValue>
bool AvlDictionary<TKey, TValue>::Remove(const TKey& key) {
    bool removed = false;
    _root = _remove(_root, key, removed);
    if (removed) --_count;
    return removed;
}

template<typename TKey, typename TValue>
void AvlDictionary<TKey, TValue>::Clear() {
    _destroy(_root);
    _root  = nullptr;
    _count = 0;
}

template<typename TKey, typename TValue>
size_t AvlDictionary<TKey, TValue>::Count() const {
    return _count;
}

template<typename TKey, typename TValue>
void AvlDictionary<TKey, TValue>::_destroy(Node* node) {
    if (!node) return;
    _destroy(node->left);
    _destroy(node->right);
    delete node;
}

} // namespace dtr

#endif // AVLDICTIONARY_TPP
