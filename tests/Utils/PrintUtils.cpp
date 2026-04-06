#include <iostream>
#include "../../include/Dictorium/Dictorium.h"

void print_json(const dtr::IDictionary<std::string, int>& dict) {
    std::cout << dict << '\n';
}
