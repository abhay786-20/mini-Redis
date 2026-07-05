#include <iostream>
#include "store/types/DataTypeFactory.hpp"

int main() {
    try {
        auto s = DataTypeFactory::create("string");
        std::cout << s->getType() << " -> " << s->serialize() << std::endl;

        auto l = DataTypeFactory::create("list");
        std::cout << l->getType() << " -> " << l->serialize() << std::endl;

        auto h = DataTypeFactory::create("hash");
        std::cout << h->getType() << " -> " << h->serialize() << std::endl;

        auto st = DataTypeFactory::create("set");
        std::cout << st->getType() << " -> " << st->serialize() << std::endl;

        auto bad = DataTypeFactory::create("unknown");

    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}