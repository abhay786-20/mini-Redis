#include "GetCommand.hpp"

GetCommand::GetCommand(const std::string& key) : _key(key) {}

std::string GetCommand::execute( ) {
    return StoreEngine::getInstance().get(_key);
}