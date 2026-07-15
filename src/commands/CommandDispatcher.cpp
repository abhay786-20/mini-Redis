#include "CommandDispatcher.hpp"
#include "persistence/AofWriter.hpp"
#include <stdexcept>

void CommandDispatcher::registerCommand(const std::string& name, CommandFactory factory, bool isWriteCommand) {
    _commands[name] = factory;
    if (isWriteCommand) {
        _writeCommands.insert(name);
    }
}

std::string CommandDispatcher::dispatch(const std::vector<std::string>& tokens, int clientFd, bool logToAof) {
    if (tokens.empty()) {
        throw std::invalid_argument("Empty command");
    }
    auto it = _commands.find(tokens[0]);
    if (it == _commands.end()) {
        throw std::invalid_argument("Unknown command: " + tokens[0]);
    }
    std::string response = it->second(tokens, clientFd)->execute();
    if (logToAof && _writeCommands.count(tokens[0])) {
        AofWriter::getInstance().append(tokens);
    }
    return response;
}