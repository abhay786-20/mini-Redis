#include "CommandDispatcher.hpp"
#include <stdexcept>

void CommandDispatcher::registerCommand(const std::string& name, std::unique_ptr<ICommand> command) {
    _commands[name] = std::move(command);
}

std::string CommandDispatcher::dispatch(const std::string& command) {
    auto it = _commands.find(command);
    if (it == _commands.end()) {
     throw std::invalid_argument("Unknown command: " + command);
    }
    return it->second->execute();
}