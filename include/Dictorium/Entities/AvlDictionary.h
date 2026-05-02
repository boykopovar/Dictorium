#ifndef DICTORIUM_AVLDICTIONARY_H
#define DICTORIUM_AVLDICTIONARY_H

#include <utility>
#include <stdexcept>
#include <initializer_list>
#include <stack>

#include "Dictorium/Contracts/Contracts.h"
#include "Dictorium/Contracts/IBalancedTreeDictionary.h"

namespace dtr {
namespace detail {
    template<typename TKey, typename TValue>
    struct AvlNode {
        std::pair<TKey, TValue> data;
        AvlNode *left;
        AvlNode *right;
        unsigned char height;
    };
}


template<typename TKey, typename TValue>
class IDictionary;

template<typename TKey, typename TValue, typename TNode>
class IBalancedTreeDictionary;

template<typename TKey, typename TValue>
class AvlDictionary : public IDictionary<TKey, TValue>
        , IBalancedTreeDictionary<TKey, TValue, detail::AvlNode<TKey, TValue>> {

public:

    using Node = detail::AvlNode<TKey, TValue>;

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

        Iterator& operator++() { _advance(); return *this; }
        Iterator operator++(int) { Iterator tmp = *this; _advance(); return tmp; }

        bool operator==(const Iterator& o) const { return _current == o._current; }
        bool operator!=(const Iterator& o) const { return _current != o._current; }

    private:
        std::stack<Node*> _stack;
        Node* _current;

        void _pushLeft(Node* node) {
            while (node) { _stack.push(node); node = node->left; }
        }
        void _advance() {
            if (_stack.empty()) { _current = nullptr; return; }
            _current = _stack.top(); _stack.pop();
            _pushLeft(_current->right);
        }
    };

    AvlDictionary() : _root(nullptr), _count(0) {}

    AvlDictionary(std::initializer_list<std::pair<TKey, TValue>> init)
            : _root(nullptr), _count(0)
    {
        for (const auto& [k, v] : init)
            Add(k, v);
    }

    template<typename TIter>
    AvlDictionary(TIter first, TIter last) : _root(nullptr), _count(0) {
        for (; first != last; ++first) Add(first->first, first->second);
    }

    ~AvlDictionary() override { _clear(_root); }

    bool ContainsKey(const TKey& key) const override
    {
        return _find(_root, key) != nullptr;
    };

    bool TryGetValue(const TKey& key, TValue& value) const override{
        auto node = _find(_root, key);
        if (!node){
            return false;
        } else{
            value = node->data.second;
        }
        return true;
    };

    void Add(const TKey& key, const TValue& value) override{
        if (_find(_root, key))
            throw std::invalid_argument("Element exists");
        bool inserted = false;
        _root = _insert(_root, key, value, inserted);
        if (inserted) ++_count;
    };

    void InsertOrAssign(const TKey& key, const TValue& value) override{
        auto node = _find(_root, key);
        if (!node) {
            bool inserted = false;
            _root = _insert(_root, key, value, inserted);
            if (inserted) ++_count;
        } else {
            node->data.second = value;
        }
    };

    bool Remove(const TKey& key) override{
        bool removed = false;
        _root = _remove(_root, key, removed);
        if (removed) --_count;
        return removed;
    };

    void Clear() override{
        _clear(_root);
        _root = nullptr;
        _count = 0;
    };

    [[nodiscard]] size_t Count() const override{
        return _count;
    };

    TValue& GetValue(const TKey& key) override{
        auto node = _find(_root, key);
        if (!node){
            throw std::out_of_range("Key not found");
        }
        return node->data.second;
    };

    const TValue& GetValue(const TKey& key) const override{
        auto node = _find(_root, key);
        if (!node){
            throw std::out_of_range("Key not found");
        }
        return node->data.second;
    };

    //std::vector<std::pair<TKey, TValue>> LowerBound(const TKey& key) override;
    //std::vector<std::pair<TKey, TValue>> UpperBound(const TKey& key) override;

    [[nodiscard]] unsigned char Height() const override{
        return _root? _root->height : 0;
    };

    Iterator begin() const { return Iterator(_root); }
    Iterator end() const { return Iterator(); }

    std::ostream& WriteToStream(std::ostream& os) const override {
        return this->_writeItems(os, *this);
    }

protected:

    Node* RotationRight(Node* node) override{
        Node* newNode = node->left;
        node->left = newNode->right;
        newNode->right = node;
        _fixHeight(node);
        _fixHeight(newNode);
        return newNode;
    };

    Node* RotationLeft(Node* node) override{
        Node* newNode = node->right;
        node->right = newNode->left;
        newNode->left = node;
        _fixHeight(node);
        _fixHeight(newNode);
        return newNode;
    };

private:

    Node* _root = nullptr;
    unsigned int _count = 0;

    static unsigned char _height(Node* node){
        return node ? node->height : 0;
    }

    void _clear(Node* node){
        if (!node) return;
        _clear(node->left);
        _clear(node->right);
        delete node;
    }

    Node* _initNode(TKey key, TValue value){
        return new Node{
                {key, value},
                nullptr,
                nullptr,
                1
        };
    }
    Node* _find (Node* node, const TKey& key) const{
        if (!node) return 0;
        if (node->data.first > key){
            return _find(node->left, key);
        } else if (node->data.first < key){
            return _find(node->right, key);
        } else{
            return node;
        }
        return nullptr;
    }
    Node* _insert(Node* node, const TKey& key, const TValue& value, bool& inserted){
        if (!node) {
            inserted = true;
            return new Node{
                {key, value},
                nullptr,
                nullptr,
                1
            };
        }
        if (key < node->data.first)
            node->left = _insert(node->left, key, value, inserted);
        else if (key > node->data.first)
            node->right = _insert(node->right, key, value, inserted);
        return _balance(node);
    };

    Node* _findMin(Node* node){
        return node->left? _findMin(node->left):node;
    }

    Node* _removeMin(Node* node){
        if (!node->left){
            return node->right;
        }
        node->left = _removeMin(node->left);
        return node;
    }

    Node* _remove(Node* node, const TKey& key, bool& removed){
        if (!node) { removed = false; return nullptr; }

        if (key < node->data.first) {
            node->left = _remove(node->left, key, removed);
        } else if (key > node->data.first) {
            node->right = _remove(node->right, key, removed);
        } else {
            removed = true;
            Node* left = node->left;
            Node* right = node->right;
            delete node;

            if (!right) return left;

            Node* min = _findMin(right);
            min->right = _removeMin(right);
            min->left = left;
            return _balance(min);
        }
        return _balance(node);
    }

    static int _balanceFactor(Node* node){
        return static_cast<int>(_height(node->left)) - static_cast<int>(_height(node->right));
    }

    static void _fixHeight(Node* node){
        unsigned char heightLeft = _height(node->left);
        unsigned char heightRight = _height(node->right);
        node->height = static_cast<unsigned char>(((heightLeft > heightRight) ? heightLeft : heightRight) + 1);
    }

    Node* _balance(Node* node){
        _fixHeight(node);
        if (_balanceFactor(node) == 2) {
            if (_balanceFactor(node->left) < 0){
                node->left = RotationLeft(node->left);
            }
            return RotationRight(node);
        }
        if (_balanceFactor(node) == -2){
            if (_balanceFactor(node->right) > 0){
                node->right = RotationRight(node->right);
            }
            return RotationLeft(node);
        }
        return node;
    }
};
}

#endif //DICTORIUM_AVLDICTIONARY_H
