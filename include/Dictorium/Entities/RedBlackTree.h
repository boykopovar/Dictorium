#ifndef DICTORIUM_REDBLACKTREE_H
#define DICTORIUM_REDBLACKTREE_H


#include "Dictorium/Contracts/Contracts.h"
#include "Dictorium/Contracts/IBalancedTreeDictionary.h"

namespace dtr {
namespace detail {
    template<typename TKey, typename TValue>
    struct RedBlackNode {
        std::pair<TKey, TValue> data;
        RedBlackNode *parent;
        RedBlackNode *left;
        RedBlackNode *right;
        //unsigned char blackHeight;
        //unsigned char height;
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

        RedBlackTree(){
            _nil = new Node();
            _nil->color = false;
            _nil->right = nullptr;
            _nil->left = nullptr;
            _root = _nil;
        };

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
//            if (_find(_root, key))
//                throw std::invalid_argument("Element exists");
//            bool inserted = false;
//            _root = _insert(_root, key, value, inserted);
//            if (inserted) ++_count;
        };

        void InsertOrAssign(const TKey& key, const TValue& value) override{
//            auto node = _find(_root, key);
//            if (!node) {
//                bool inserted = false;
//                _root = _insert(_root, key, value, inserted);
//                if (inserted) ++_count;
//            } else {
//                node->data.second = value;
//            }
        };

        bool Remove(const TKey& key) override{
//            bool removed = false;
//            _root = _remove(_root, key, removed);
//            if (removed) --_count;
//            return removed;
        };

        void Clear() override{
//            _clear(_root);
//            _root = nullptr;
//            _count = 0;
        };

        [[nodiscard]] size_t Count() const override{
            //return _count;
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

    protected:

        Node* RotationRight(Node* node) override{
            Node* newNode = node->left;
            node->left = newNode->right;
            newNode->right = node;
            return newNode;
        };

        Node* RotationLeft(Node* node) override{
            Node* newNode = node->right;
            node->right = newNode->left;
            newNode->left = node;
            return newNode;
        };

    private:
        Node* _root;
        Node* _nil;

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

        Node* _insert(Node* node, const TKey& key, const TValue& value){
            while (node->parent && node->parent->color)
            {
                if (node->parent == node->parent->parent->left){

                    auto* uncle = node->parent->parent->right;

                    if(uncle->color){
                        node->parent->color = false;
                        uncle->color = false;
                        node->parent->parent->color = true;
                        node = node->parent->parent;
                    }else{
                        if(node == node->parent->right){
                            node = node->parent;
                            RotationLeft(node);
                        }
                        node->parent->color = false;
                        node->parent->parent->color = true;
                        RotationRight(node->parent->parent);
                    }
                }else{
                    auto* uncle = node->parent->parent->left;

                    if(uncle->color)
                    {
                        node->parent->color = false;
                        uncle->color = false;
                        node->parent->parent->color = true;
                        node = node->parent->parent;
                    }else{
                        if(node == node->parent->left)
                        {
                            node = node->parent;
                            RotationRight(node);
                        }

                        node->parent->color = false;
                        node->parent->parent->color = true;
                        RotationLeft(node->parent->parent);
                    }
                }
            }
            _root->color = false;
        };

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

        Node* _remove(Node* node, const TKey& key){
            while (node != _root && !node->color){
                if (node == node->parent->left) {
                    Node* sibling = node->parent->right;

                    if (sibling->color) {
                        sibling->color = false;
                        node->parent->color = true;
                        RotationLeft(node->parent);
                        sibling = node->parent->right;
                    }

                    if (!sibling->left->color && !sibling->right->color) {
                        sibling->color = true;
                        node = node->parent;
                    }else {
                        if (!sibling->right->color) {
                            sibling->left->color = false;
                            sibling->color = true;
                            RotationRight(sibling);
                            sibling = node->parent->right;
                        }

                        sibling->color = node->parent->color;
                        node->parent->color = false;
                        sibling->right->color = false;
                        rotateLeft(node->parent);
                        node = _root;
                    }
                }else {
                    Node* sibling = node->parent->left;

                    if (sibling->color) {
                        sibling->color = false;
                        node->parent->color = true;
                        RotationRight(node->parent);
                        sibling = node->parent->left;
                    }

                    if (!sibling->right->color && !sibling->left->color) {
                        sibling->color = true;
                        node = node->parent;
                    }else {
                        if (!sibling->left->color) {
                            sibling->right->color = false;
                            sibling->color = true;
                            RotationLeft(sibling);
                            sibling = node->parent->left;
                        }

                        sibling->color = node->parent->color;
                        node->parent->color = false;
                        sibling->left->color = false;
                        RotationRight(node->parent);
                        node = _root;
                    }
                }
            }
            node->color = false;
        }
    };
}

#endif //DICTORIUM_REDBLACKTREE_H
