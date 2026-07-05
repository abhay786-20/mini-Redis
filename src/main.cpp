#include <iostream>
#include "commands/CommandDispatcher.hpp"
#include "commands/GetCommand.hpp"
#include "commands/SetCommand.hpp"

int main() {
    try {
        CommandDispatcher dispatcher;

        dispatcher.registerCommand("SET", std::make_unique<SetCommand>("name", "Abhay"));
        dispatcher.registerCommand("GET", std::make_unique<GetCommand>("name"));

        std::cout << dispatcher.dispatch("SET") << std::endl; // OK
        std::cout << dispatcher.dispatch("GET") << std::endl; // Abhay
        std::cout << dispatcher.dispatch("DEL") << std::endl; // throws

    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}