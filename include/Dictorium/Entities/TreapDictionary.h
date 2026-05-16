#ifndef DICTORIUM_TREAPDICTIONARY_H
#define DICTORIUM_TREAPDICTIONARY_H

#include <cstdint>
#include <initializer_list>
#include <ostream>
#include <random>
#include <stack>
#include <stdexcept>
#include <utility>

#include "Dictorium/Contracts/Contracts.h"

namespace dtr {
namespace detail {

template <typename TKey, typename TValue>
struct TreapNode {
    std::pair<TKey, TValue> data;
    uint32_t priority = 0;
    TreapNode* left = nullptr;
    TreapNode* right = nullptr;
};

} // namespace detail

template <typename TKey, typename TValue>
class IDictionary;

template <typename TKey, typename TValue>
class TreapDictionary : public IDictionary<TKey, TValue> {
public:
    using Node = detail::TreapNode<TKey, TValue>;

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<TKey, TValue>;
        using reference = std::pair<TKey, TValue>&;
        using pointer = std::pair<TKey, TValue>*;
        using difference_type = std::ptrdiff_t;

        explicit Iterator(Node* root) : _current(nullptr) {
            _pushLeft(root);
            _advance();
        }

        Iterator() : _current(nullptr) {}

        reference operator*() const { return _current->data; }
        pointer operator->() const { return &_current->data; }

        Iterator& operator++() {
            _advance();
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            _advance();
            return tmp;
        }

        bool operator==(const Iterator& other) const { return _current == other._current; }
        bool operator!=(const Iterator& other) const { return _current != other._current; }

    private:
        std::stack<Node*> _stack;
        Node* _current = nullptr;

        void _pushLeft(Node* node) {
            while (node) {
                _stack.push(node);
                node = node->left;
            }
        }

        void _advance() {
            if (_stack.empty()) {
                _current = nullptr;
                return;
            }
            _current = _stack.top();
            _stack.pop();
            _pushLeft(_current->right);
        }
    };

    TreapDictionary();
    ~TreapDictionary() override;

    TreapDictionary(std::initializer_list<std::pair<TKey, TValue>> init)
        : TreapDictionary() {
        for (const auto& [key, value] : init) {
            Add(key, value);
        }
    }

    template <typename TIter>
    TreapDictionary(TIter first, TIter last);

    TreapDictionary(const TreapDictionary&) = delete;
    TreapDictionary& operator=(const TreapDictionary&) = delete;

    bool ContainsKey(const TKey& key) const override;
    bool TryGetValue(const TKey& key, TValue& value) const override;
    void Add(const TKey& key, const TValue& value) override;
    void InsertOrAssign(const TKey& key, const TValue& value) override;
    bool Remove(const TKey& key) override;
    void Clear() override;
    [[nodiscard]] size_t Count() const override;
    TValue& GetValue(const TKey& key) override;
    const TValue& GetValue(const TKey& key) const override;

    Iterator begin() const { return Iterator(_root); }
    Iterator end() const { return Iterator(); }

    std::ostream& WriteToStream(std::ostream& os) const override {
        return this->_writeItems(os, *this);
    }

private:
    Node* _root = nullptr;
    size_t _count = 0;
    mutable uint64_t _rngState = 0;

    [[nodiscard]] uint32_t _randomPriority() const;
    [[nodiscard]] Node* _find(Node* node, const TKey& key) const;
    [[nodiscard]] Node* _initNode(const TKey& key, const TValue& value);
    void _clear(Node* node);

    static Node* _rotateLeft(Node* node);
    static Node* _rotateRight(Node* node);

    Node* _insert(Node* node, const TKey& key, const TValue& value, bool& inserted);
    Node* _remove(Node* node, const TKey& key, bool& removed);
};

}

#include "TreapDictionary/TreapDictionary.tpp"

#endif // DICTORIUM_TREAPDICTIONARY_H
