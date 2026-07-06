#include "DelCommand.hpp"
#include <stdexcept>

DelCommand::DelCommand(const std::string& key) : _key(key) {}

std::string DelCommand::execute() {
    StoreEngine::getInstance().del(_key);
    return "(integer) 1";
}