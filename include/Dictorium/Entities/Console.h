#ifndef CONSOLE_H
#define CONSOLE_H

#include <iostream>
#include <concepts>
#include <typeinfo>
#include <vector>
#include "Dictorium/Contracts/Concepts.h"

namespace dtr {

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const std::vector<T>& vector) {
    if constexpr (CStreamWritable<T>) {
        os << '[';

        bool first = true;

        for (auto& value : vector) {
            if (!first) os << ',' << ' ';
            first = false;
            os << value;
        }
        os << ']';
        return os;
    }
    else {
        os << "<class 'std::vector<" << typeid(T).name() << ">'>";
        return os;
    }
}

template<typename T>
concept CPrintable = requires(std::ostream& os, T value){ os << value; };

}

#endif //CONSOLE_H
