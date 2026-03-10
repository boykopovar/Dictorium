#ifndef IENUMERABLE_H
#define IENUMERABLE_H

namespace dtr {
    template<typename TIterator>
    class IEnumerable {
    public:
        virtual ~IEnumerable() = default;

        virtual TIterator begin() = 0;
        virtual TIterator end() = 0;

        virtual TIterator begin() const = 0;
        virtual TIterator end() const = 0;
    };
}

#endif //IENUMERABLE_H
