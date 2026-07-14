#include "SubscribeCommand.hpp"

SubscribeCommand::SubscribeCommand(const std::string& channel, int clientFd) 
    : _channel(channel), _clientFd(clientFd) {}

std::string SubscribeCommand::execute() {
    PubSubManager::getInstance().subscribe(_channel, _clientFd);
    return "*3\r\n$9\r\nsubscribe\r\n$" + std::to_string(_channel.size()) + "\r\n" + _channel + "\r\n:1\r\n";
}