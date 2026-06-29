#include <iostream>
#include "store/types/StringType.hpp"

int main() {
    StringType * s  = new StringType("Hello World");
    std::cout << s->getType() << std::endl;
    std::cout << s->serialize() << std::endl;

    delete s;
    return 0;
}