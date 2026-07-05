#include <iostream>
#include "store/StoreEngine.hpp"
#include "store/types/DataTypeFactory.hpp"
#include "store/types/StringType.hpp"

int main() {
    try {
        auto& store = StoreEngine::getInstance();

        // set with actual value
        auto val = std::make_unique<StringType>("Abhay");
        store.set("name", std::move(val));
        std::cout << store.get("name") << std::endl; // should print Abhay

        store.del("name");
        std::cout << store.get("name") << std::endl; // should throw

    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}