#ifndef DICTORIUM_IBALANCEDTREEDICTIONARY_H
#define DICTORIUM_IBALANCEDTREEDICTIONARY_H

namespace dtr {

    template<typename TKey, typename TValue, typename TNode>
    class IBalancedTreeDictionary {

    public:
        IBalancedTreeDictionary() = default;
        ~IBalancedTreeDictionary() = default;

        [[nodiscard]] virtual unsigned char Height(TNode* p);

    protected:
        virtual TNode* RotationRight(TNode* p);
        virtual TNode* RotationLeft(TNode* p);
        virtual std::vector<std::pair<TKey, TValue>> LowerBound(const TKey& key);
        virtual std::vector<std::pair<TKey, TValue>> UpperBound(const TKey& key);
    };
}

#endif //DICTORIUM_IBALANCEDTREEDICTIONARY_H
