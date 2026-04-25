#ifndef DICTORIUM_AVLDICTIONARY_H
#define DICTORIUM_AVLDICTIONARY_H

#include <utility>
#include <stdexcept>
#include <vector>

namespace dtr {

template<typename TKey, typename TValue>
struct Node {
    std::pair<TKey, TValue> data;
    Node* left;
    Node* right;
    unsigned char height;
};

template<typename TKey, typename TValue>
class IDictionary;

template<typename TKey, typename TValue, typename TNode>
class IBalancedTreeDictionary;

template<typename TKey, typename TValue>
class AvlDictionary : public IDictionary<TKey, TValue>, IBalancedTreeDictionary<TKey, TValue, dtr::Node<TKey, TValue>> {

public:

    bool ContainsKey(const TKey& key) const override;
    bool TryGetValue(const TKey& key, TValue& value) const override;

    void Add(const TKey& key, const TValue& value) override;

    void InsertOrAssign(const TKey& key, const TValue& value) override;
    bool Remove(const TKey& key) override;
    void Clear() override;
    [[nodiscard]] size_t Count() const override;
    TValue& GetValue(const TKey& key) override;
    const TValue& GetValue(const TKey& key) const override;
    std::vector<std::pair<TKey, TValue>> LowerBound(const TKey& key) override;
    std::vector<std::pair<TKey, TValue>> UpperBound(const TKey& key) override;

private:

    Node<TKey,TValue>* _insert(Node<TKey,TValue>* node, Node<TKey,TValue>* newNode){
        if (!node)
            return newNode;
        if (node->data.first > newNode->data.first)
            node->left = _insert(node->left, newNode);
        if (node->data.first < newNode->data.first)
            node->right = _insert(node->right, newNode);
        return _balance(node);
    };

    Node<TKey,TValue>* _findMin(Node<TKey,TValue>* node){
        return node->left? _findMin(node->left):node;
    }

    Node<TKey,TValue>* _removeMin(Node<TKey,TValue>* node){
        if (node->left == 0){
            return node->right;
        }
        node->left == _removeMin(node->left);
        return _balance(node);
    }

    Node<TKey,TValue>* _remove(Node<TKey,TValue>* node, Node<TKey,TValue>* removeNode){
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
    unsigned char _height(Node<TKey, TValue>* node){
        return node->height ? node->height : 0;
    }

    unsigned char _balanceFactor(Node<TKey, TValue>* node){
        return _height(node->left) - _height(node->right);
    }
    void _fixHeight(Node<TKey, TValue>* node){
        unsigned char heightLeft = _height(node->left->height);
        unsigned char heightRight = _height(node->right->height);
        node->height = ((heightLeft > heightRight) ? heightLeft : heightRight) + 1;
    }

    Node<TKey,TValue>* _root = nullptr;
    Node<TKey, TValue>* _rotationRight(Node<TKey, TValue>* node) override{
        Node<TKey, TValue>* newNode = node->left;
        node->left = newNode->right;
        newNode->right = node;
        _fixHeight(node);
        _fixHeight(newNode);
        return newNode;
    };
    Node<TKey, TValue>* _rotationLeft(Node<TKey, TValue>* node) override{
        Node<TKey, TValue>* newNode = node->right;
        node->right = newNode->left;
        newNode->left = node;
        _fixHeight(node);
        _fixHeight(newNode);
        return newNode;
    };
    Node<TKey, TValue>* _balance(Node<TKey, TValue>* node){
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
