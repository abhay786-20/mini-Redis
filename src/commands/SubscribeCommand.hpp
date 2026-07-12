#pragma once
#include "ICommand.hpp"
#include "pubsub/PubSubManager.hpp"

class SubscribeCommand : public ICommand {
private:
    std::string _channel;
    int _clientFd;
public:
    SubscribeCommand(const std::string& channel, int clientFd);
    std::string execute() override;
};