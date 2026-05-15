#ifndef DICTORIUM_REDBLACKTREE_H
#define DICTORIUM_REDBLACKTREE_H

#include "Dictorium/Contracts/Contracts.h"
#include "Dictorium/Contracts/IBalancedTreeDictionary.h"

namespace dtr {
    namespace detail {
        template<typename TKey, typename TValue>
        struct RedBlackNode {
            std::pair<TKey, TValue> data;
            RedBlackNode* parent;
            RedBlackNode* left;
            RedBlackNode* right;
            bool color = true;
        };
    }

    template<typename TKey, typename TValue>
    class IDictionary;

    template<typename TKey, typename TValue, typename TNode>
    class IBalancedTreeDictionary;

    template<typename TKey, typename TValue>
    class RedBlackTree : public IDictionary<TKey, TValue>
            , IBalancedTreeDictionary<TKey, TValue, detail::RedBlackNode<TKey, TValue>> {
    public:
        using Node = detail::RedBlackNode<TKey, TValue>;

        class Iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::pair<TKey, TValue>;
            using reference = std::pair<TKey, TValue>&;
            using pointer = std::pair<TKey, TValue>*;
            using difference_type = std::ptrdiff_t;

            explicit Iterator(Node* root, Node* nil) : _current(nullptr), _nil(nil) {
                _pushLeft(root);
                _advance();
            }
            Iterator() : _current(nullptr), _nil(nullptr) {}

            reference operator*() const { return _current->data; }
            pointer operator->() const { return &_current->data; }

            Iterator& operator++() { _advance(); return *this; }
            Iterator operator++(int) { Iterator tmp = *this; _advance(); return tmp; }

            bool operator==(const Iterator& o) const { return _current == o._current; }
            bool operator!=(const Iterator& o) const { return _current != o._current; }

        private:
            std::stack<Node*> _stack;
            Node* _current;
            Node* _nil;

            void _pushLeft(Node* node) {
                while (node && node != _nil) { _stack.push(node); node = node->left; }
            }
            void _advance() {
                if (_stack.empty()) { _current = nullptr; return; }
                _current = _stack.top(); _stack.pop();
                _pushLeft(_current->right);
            }
        };

        RedBlackTree() {
            _nil = new Node();
            _nil->color = false;
            _nil->left = nullptr;
            _nil->right = nullptr;
            _nil->parent = nullptr;
            _root= _nil;
            _count = 0;
        }

        ~RedBlackTree() {
            _clear(_root);
            delete _nil;
        }

        Iterator begin() const { return Iterator(_root, _nil); }
        Iterator end() const { return Iterator(); }

        std::ostream& WriteToStream(std::ostream& os) const override {
            return this->_writeItems(os, *this);
        }

        bool ContainsKey(const TKey& key) const override {
            return _find(_root, key) != nullptr;
        }

        bool TryGetValue(const TKey& key, TValue& value) const override {
            auto* node = _find(_root, key);
            if (!node) return false;
            value = node->data.second;
            return true;
        }

        void Add(const TKey& key, const TValue& value) override {
            if (_find(_root, key))
                throw std::invalid_argument("Key already exists");
            _bstInsert(key, value);
            ++_count;
        }

        void InsertOrAssign(const TKey& key, const TValue& value) override {
            auto* node = _find(_root, key);
            if (node) {
                node->data.second = value;
            } else {
                _bstInsert(key, value);
                ++_count;
            }
        }

        bool Remove(const TKey& key) override {
            bool removed = _bstRemove(key);
            if (removed) --_count;
            return removed;
        }

        void Clear() override {
            _clear(_root);
            _root = _nil;
            _count = 0;
        }

        [[nodiscard]] size_t Count() const override {
            return _count;
        }

        [[nodiscard]] unsigned char Height() const override {
            return _height(_root);
        }

        TValue& GetValue(const TKey& key) override {
            auto* node = _find(_root, key);
            if (!node) throw std::out_of_range("Key not found");
            return node->data.second;
        }

        const TValue& GetValue(const TKey& key) const override {
            auto* node = _find(_root, key);
            if (!node) throw std::out_of_range("Key not found");
            return node->data.second;
        }

    protected:

        Node* RotationLeft(Node* x) override {
            Node* y = x->right;
            x->right = y->left;

            if (y->left != _nil)
                y->left->parent = x;

            y->parent = x->parent;

            if (x->parent == nullptr)
                _root = y;
            else if (x == x->parent->left)
                x->parent->left = y;
            else
                x->parent->right = y;

            y->left = x;
            x->parent = y;
            return y;
        }

        Node* RotationRight(Node* y) override {
            Node* x = y->left;
            y->left = x->right;

            if (x->right != _nil)
                x->right->parent = y;

            x->parent = y->parent;

            if (y->parent == nullptr)
                _root = x;
            else if (y == y->parent->right)
                y->parent->right = x;
            else
                y->parent->left = x;

            x->right = y;
            y->parent = x;
            return x;
        }

    private:
        Node* _root;
        Node* _nil;
        size_t _count;

        Node* _find(Node* node, const TKey& key) const {
            if (node == _nil) return nullptr;
            if (key < node->data.first) return _find(node->left, key);
            else if (key > node->data.first) return _find(node->right, key);
            else return node;
        }

        void _bstInsert(const TKey& key, const TValue& value) {
            Node* z = new Node();
            z->data = {key, value};
            z->color = true;
            z->left = _nil;
            z->right = _nil;
            z->parent = nullptr;

            Node* y = nullptr;
            Node* x = _root;
            while (x != _nil) {
                y = x;
                if (z->data.first < x->data.first) x = x->left;
                else x = x->right;
            }

            z->parent = y;
            if (y == nullptr)
                _root = z;
            else if (z->data.first < y->data.first)
                y->left = z;
            else
                y->right = z;

            if (z->parent == nullptr) { z->color = false; return; }
            if (!z->parent->color) return;

            _fixInsert(z);
        }

        void _fixInsert(Node* z) {
            while (z->parent && z->parent->color) {
                if (z->parent == z->parent->parent->left) {
                    Node* uncle = z->parent->parent->right;

                    if (uncle->color) {
                        z->parent->color = false;
                        uncle->color = false;
                        z->parent->parent->color = true;
                        z = z->parent->parent;
                    } else {
                        if (z == z->parent->right) {
                            z = z->parent;
                            RotationLeft(z);
                        }
                        z->parent->color = false;
                        z->parent->parent->color = true;
                        RotationRight(z->parent->parent);
                    }
                } else {
                    Node* uncle = z->parent->parent->left;

                    if (uncle->color) {
                        z->parent->color = false;
                        uncle->color = false;
                        z->parent->parent->color = true;
                        z = z->parent->parent;
                    } else {
                        if (z == z->parent->left) {
                            z = z->parent;
                            RotationRight(z);
                        }
                        z->parent->color = false;
                        z->parent->parent->color = true;
                        RotationLeft(z->parent->parent);
                    }
                }
            }
            _root->color = false;
        }

        bool _bstRemove(const TKey& key) {
            Node* z = _find(_root, key);
            if (!z) return false;

            Node* y = z;
            Node* x;
            bool yOriginalColor = y->color;

            if (z->left == _nil) {
                x = z->right;
                _transplant(z, z->right);
            } else if (z->right == _nil) {
                x = z->left;
                _transplant(z, z->left);
            } else {
                y = _minimum(z->right);
                yOriginalColor = y->color;
                x = y->right;

                if (y->parent == z) {
                    x->parent = y;
                } else {
                    _transplant(y, y->right);
                    y->right = z->right;
                    y->right->parent = y;
                }

                _transplant(z, y);
                y->left = z->left;
                y->left->parent = y;
                y->color= z->color;
            }

            delete z;

            if (!yOriginalColor)
                _fixRemove(x);

            return true;
        }

        void _transplant(Node* u, Node* v) {
            if (u->parent == nullptr)
                _root = v;
            else if (u == u->parent->left)
                u->parent->left = v;
            else
                u->parent->right = v;
            v->parent = u->parent;
        }

        Node* _minimum(Node* node) {
            while (node->left != _nil) node = node->left;
            return node;
        }

        void _fixRemove(Node* x) {
            while (x != _root && !x->color) {
                if (x == x->parent->left) {
                    Node* s = x->parent->right;

                    if (s->color) {
                        s->color = false;
                        x->parent->color = true;
                        RotationLeft(x->parent);
                        s = x->parent->right;
                    }

                    if (!s->left->color && !s->right->color) {
                        s->color = true;
                        x = x->parent;
                    } else {
                        if (!s->right->color) {
                            s->left->color = false;
                            s->color = true;
                            RotationRight(s);
                            s = x->parent->right;
                        }
                        s->color = x->parent->color;
                        x->parent->color = false;
                        s->right->color = false;
                        RotationLeft(x->parent);
                        x = _root;
                    }
                } else {
                    Node* s = x->parent->left;

                    if (s->color) {
                        s->color = false;
                        x->parent->color = true;
                        RotationRight(x->parent);
                        s = x->parent->left;
                    }

                    if (!s->right->color && !s->left->color) {
                        s->color = true;
                        x = x->parent;
                    } else {
                        if (!s->left->color) {
                            s->right->color = false;
                            s->color= true;
                            RotationLeft(s);
                            s = x->parent->left;
                        }
                        s->color = x->parent->color;
                        x->parent->color = false;
                        s->left->color = false;
                        RotationRight(x->parent);
                        x = _root;
                    }
                }
            }
            x->color = false;
        }

        void _clear(Node* node) {
            if (node == _nil) return;
            _clear(node->left);
            _clear(node->right);
            delete node;
        }

        unsigned char _height(Node* node) const {
            if (node == _nil) return 0;
            auto l = _height(node->left);
            auto r = _height(node->right);
            return 1 + (l > r ? l : r);
        }
    };
}

#endif //DICTORIUM_REDBLACKTREE_H