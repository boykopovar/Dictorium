#ifndef DICTORIUM_AVLDICTIONARY_H
#define DICTORIUM_AVLDICTIONARY_H

#include <utility>
#include <stdexcept>
#include <vector>

namespace dtr {
namespace detail {
    template<typename TKey, typename TValue>
    struct Node {
        std::pair<TKey, TValue> data;
        Node *left;
        Node *right;
        unsigned char height;
    };
}


template<typename TKey, typename TValue>
class IDictionary;

template<typename TKey, typename TValue, typename TNode>
class IBalancedTreeDictionary;

template<typename TKey, typename TValue>
class AvlDictionary : public IDictionary<TKey, TValue>, IBalancedTreeDictionary<TKey, TValue, detail::Node<TKey, TValue>> {

public:

    using Node = detail::Node<TKey, TValue>;
    bool ContainsKey(const TKey& key) const override
    {
        auto node = _find(_root, key);
        if (node == 0){
            return false;
        } else{
            return true;
        }
    };
    bool TryGetValue(const TKey& key, TValue& value) const override{
        auto node = _find(_root, key);
        if (node == 0){
            return false;
        } else{
            node->data.second = value;
        }
        return true;
    };

    void Add(const TKey& key, const TValue& value) override{
        auto newNode = _initNode(key, value);
        if (_find(_root, newNode->data.first) != 0)
            throw std::invalid_argument("Element exists");
        _root = _insert(_root, newNode);
        _count++;
    };

    void InsertOrAssign(const TKey& key, const TValue& value) override{
        auto updateNode = _find(_root, key);
        if (updateNode == 0){
            Add(key, value);
        } else{
            updateNode->data.second = value;
        }
    };
    bool Remove(const TKey& key) override{
        auto removeNode = _remove(_root, _initNode(key, nullptr));
        if (removeNode == 0){
            return false;
        } else{
            _count--;
            return true;
        }
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
        return _find(_root, key)->data.second;
    };
    const TValue& GetValue(const TKey& key) const override{
        return _find(_root, key)->data.second;
    };
    std::vector<std::pair<TKey, TValue>> LowerBound(const TKey& key) override;
    std::vector<std::pair<TKey, TValue>> UpperBound(const TKey& key) override;
    [[nodiscard]] virtual unsigned char Height()
    {
        return _root? _root->height : 0;
    };

private:

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
    Node* _find (Node* node, TKey key){
        if (!node) return 0;
        if (node->data.first > key){
            return _find(node->left, key);
        } else if (node->data.first < key){
            return _find(node->right, key);
        } else{
            return node;
        }
        return 0;
    }
    Node* _insert(Node* node, Node* newNode){
        if (!node)
            return newNode;
        if (node->data.first > newNode->data.first)
            node->left = _insert(node->left, newNode);
        if (node->data.first < newNode->data.first)
            node->right = _insert(node->right, newNode);
        return _balance(node);
    };

    Node* _findMin(Node* node){
        return node->left? _findMin(node->left):node;
    }

    Node* _removeMin(Node* node){
        if (node->left == 0){
            return node->right;
        }
        node->left = _removeMin(node->left);
        return _balance(node);
    }

    Node* _remove(Node* node, Node* removeNode){
        if (!node) return 0;
        if (node->data.first > removeNode->data.first){
            node->left = _remove(node->left, removeNode);
        } else if (node->data.first < removeNode->data.first){
            node->right = _remove(node->right, removeNode);
        } else {
            auto leftSubtree = node->left;
            auto rightSubtree = node->right;
            delete node;
            if (!rightSubtree) return leftSubtree;
            auto min = _findMin(rightSubtree);
            min->right = _removeMin(rightSubtree);
            min->left = leftSubtree;
            return _balance(min);
        }
        return _balance(node);
    }
    unsigned char _height(Node* node){
        return node ? node->height : 0;
    }

    unsigned char _balanceFactor(Node* node){
        return _height(node->left) - _height(node->right);
    }
    void _fixHeight(Node* node){
        unsigned char heightLeft = _height(node->left);
        unsigned char heightRight = _height(node->right);
        node->height = ((heightLeft > heightRight) ? heightLeft : heightRight) + 1;
    }

    Node* _root = nullptr;
    unsigned int _count = 0;
    Node* _rotationRight(Node* node) override{
        Node* newNode = node->left;
        node->left = newNode->right;
        newNode->right = node;
        _fixHeight(node);
        _fixHeight(newNode);
        return newNode;
    };
    Node* _rotationLeft(Node* node) override{
        Node* newNode = node->right;
        node->right = newNode->left;
        newNode->left = node;
        _fixHeight(node);
        _fixHeight(newNode);
        return newNode;
    };
    Node* _balance(Node* node){
        _fixHeight(node);
        if (_balanceFactor(node) == 2) {
            if (_balanceFactor(node->left) < 0){
                node->left = _rotationLeft(node->left);
            }
            return _rotationRight(node);
        }
        if (_balanceFactor(node) == -2){
            if (_balanceFactor(node->right) > 0){
                node->right = _rotationRight(node->right);
            }
            return _rotationLeft(node);
        }
        return node;
    }
};
}

#endif //DICTORIUM_AVLDICTIONARY_H
