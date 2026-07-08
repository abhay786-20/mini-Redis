#include <iostream>
#include "protocol/RespParser.hpp"
#include "commands/CommandDispatcher.hpp"
#include "commands/SetCommand.hpp"
#include "commands/GetCommand.hpp"
#include "commands/DelCommand.hpp"

int main() {
    try {
        RespParser parser;
        CommandDispatcher dispatcher;

        // register factories — not pre-built objects
        dispatcher.registerCommand("SET", [](const std::vector<std::string>& tokens) {
            return std::make_unique<SetCommand>(tokens[1], tokens[2]);
        });

        dispatcher.registerCommand("GET", [](const std::vector<std::string>& tokens) {
            return std::make_unique<GetCommand>(tokens[1]);
        });

        dispatcher.registerCommand("DEL", [](const std::vector<std::string>& tokens) {
            return std::make_unique<DelCommand>(tokens[1]);
        });

        // simulate raw RESP input
        std::string raw1 = "*3\r\n$3\r\nSET\r\n$4\r\nname\r\n$5\r\nAbhay\r\n";
        std::string raw2 = "*2\r\n$3\r\nGET\r\n$4\r\nname\r\n";
        std::string raw3 = "*2\r\n$3\r\nDEL\r\n$4\r\nname\r\n";
        std::string raw4 = "*2\r\n$3\r\nGET\r\n$4\r\nname\r\n";

        std::cout << dispatcher.dispatch(parser.parse(raw1)) << std::endl; // OK
        std::cout << dispatcher.dispatch(parser.parse(raw2)) << std::endl; // Abhay
        std::cout << dispatcher.dispatch(parser.parse(raw3)) << std::endl; // (integer) 1
        std::cout << dispatcher.dispatch(parser.parse(raw4)) << std::endl; // throws

    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}