#include <iostream>
#include "store/types/DataTypeFactory.hpp"

int main() {
    try {
        auto s = DataTypeFactory::create("string");
        std::cout << s->getType() << " -> " << s->serialize() << std::endl;

        auto l = DataTypeFactory::create("list");
        std::cout << l->getType() << " -> " << l->serialize() << std::endl;

        // this will throw
        auto bad = DataTypeFactory::create("hash");
        std::cout << bad->getType() << std::endl;

    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}