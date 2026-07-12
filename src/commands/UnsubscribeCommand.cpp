#include "UnsubscribeCommand.hpp"

UnsubscribeCommand::UnsubscribeCommand(const std::string& channel, int clientFd)
    : _channel(channel), _clientFd(clientFd) {}

std::string UnsubscribeCommand::execute() {
    PubSubManager::getInstance().unsubscribe(_channel, _clientFd);
    return "+unsubscribed from " + _channel + "\r\n";
}