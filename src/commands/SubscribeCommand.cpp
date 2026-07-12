#include "SubscribeCommand.hpp"

SubscribeCommand::SubscribeCommand(const std::string& channel, int clientFd) 
    : _channel(channel), _clientFd(clientFd) {}

std::string SubscribeCommand::execute() {
    PubSubManager::getInstance().subscribe(_channel, _clientFd);
    return "+subscribed to " + _channel + "\r\n";
}