#ifndef DICTORIUM_IBALANCEDTREEDICTIONARY_H
#define DICTORIUM_IBALANCEDTREEDICTIONARY_H

namespace dtr {

    template<typename TKey, typename TValue, typename TNode>
    class IBalancedTreeDictionary {

    public:
        IBalancedTreeDictionary() = default;
        virtual ~IBalancedTreeDictionary() = default;

        [[nodiscard]] virtual unsigned char Height();

    protected:
        virtual TNode* RotationRight(TNode* p) = 0;
        virtual TNode* RotationLeft(TNode* p) = 0;
    };
}

#endif //DICTORIUM_IBALANCEDTREEDICTIONARY_H
