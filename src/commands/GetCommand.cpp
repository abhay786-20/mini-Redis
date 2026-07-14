#include "GetCommand.hpp"

GetCommand::GetCommand(const std::string& key) : _key(key) {}

std::string GetCommand::execute() {
    try {
        std::string value = StoreEngine::getInstance().get(_key);
        return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
    } catch (const std::exception&) {
        return "$-1\r\n";
    }
}