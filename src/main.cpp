#include <iostream>
#include "commands/CommandDispatcher.hpp"
#include "commands/GetCommand.hpp"
#include "commands/SetCommand.hpp"
#include "commands/DelCommand.hpp"
#include "commands/LPushCommand.hpp"
#include "commands/HSetCommand.hpp"

int main() {
    try {
        CommandDispatcher dispatcher;

        // register commands
        dispatcher.registerCommand("SET", std::make_unique<SetCommand>("name", "Abhay"));
        dispatcher.registerCommand("GET", std::make_unique<GetCommand>("name"));
        dispatcher.registerCommand("DEL", std::make_unique<DelCommand>("name"));
        dispatcher.registerCommand("LPUSH", std::make_unique<LPushCommand>("mylist", "hello"));
        dispatcher.registerCommand("HSET", std::make_unique<HSetCommand>("user:1", "age", "25"));

        std::cout << dispatcher.dispatch("SET") << std::endl;   // OK
        std::cout << dispatcher.dispatch("GET") << std::endl;   // Abhay
        std::cout << dispatcher.dispatch("LPUSH") << std::endl; // (integer) 1
        std::cout << dispatcher.dispatch("HSET") << std::endl;  // (integer) 1
        std::cout << dispatcher.dispatch("DEL") << std::endl;   // (integer) 1
        std::cout << dispatcher.dispatch("GET") << std::endl;   // throws — key deleted

    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}