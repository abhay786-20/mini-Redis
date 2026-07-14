#include "UnsubscribeCommand.hpp"

UnsubscribeCommand::UnsubscribeCommand(const std::string& channel, int clientFd)
    : _channel(channel), _clientFd(clientFd) {}

std::string UnsubscribeCommand::execute() {
    PubSubManager::getInstance().unsubscribe(_channel, _clientFd);
    return "*3\r\n$11\r\nunsubscribe\r\n$" + std::to_string(_channel.size()) + "\r\n" + _channel + "\r\n:0\r\n";
}