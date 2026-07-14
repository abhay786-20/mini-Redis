#include "CommandDispatcher.hpp"
#include <stdexcept>

void CommandDispatcher::registerCommand(const std::string& name, CommandFactory factory) {
    _commands[name] = factory;
}

std::string CommandDispatcher::dispatch(const std::vector<std::string>& tokens, int clientFd) {
    if (tokens.empty()) {
        throw std::invalid_argument("Empty command");
    }
    auto it = _commands.find(tokens[0]);
    if (it == _commands.end()) {
        throw std::invalid_argument("Unknown command: " + tokens[0]);
    }
    return it->second(tokens, clientFd)->execute();
}