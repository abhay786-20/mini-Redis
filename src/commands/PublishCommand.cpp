#include "PublishCommand.hpp"

PublishCommand::PublishCommand(const std::string& channel, const std::string& message)
    : _channel(channel), _message(message) {}

std::string PublishCommand::execute() {
    PubSubManager::getInstance().publish(_channel, _message);
    return "(integer) 1";
}