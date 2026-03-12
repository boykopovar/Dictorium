#ifndef CONSOLE_H
#define CONSOLE_H

#include <iostream>
#include <concepts>

namespace dtr {

template<typename T>
concept CPrintable = requires(std::ostream& os, T value){ os << value; };


template<CPrintable T>
inline void print(T&& last) {
    std::cout << last << '\n';
}

template <CPrintable T, CPrintable... Args>
inline void print(T&& first, Args&&... args) {
    std::cout<<first<<' ';
    print(std::forward<Args>(args)...);
}

inline std::string input() {
    std::string input;
    std::getline(std::cin, input);
    return input;
}

template <CPrintable T, CPrintable... Args>
inline std::string input(T&& first, Args&&... args) {
    std::cout<<first<<' ';

    if constexpr (sizeof...(args) > 0) print(std::forward<Args>(args)...);
    return input();
}

}

#endif //CONSOLE_H
