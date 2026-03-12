#include <iostream>
#include "Dictorium/Dictorium.h"

void print_dict(dtr::IDictionary<std::string, int>& dict) {
    for (auto& [key, value] : dict.Items()) {
        std::cout << key << ": " << value << std::endl;
    }
}