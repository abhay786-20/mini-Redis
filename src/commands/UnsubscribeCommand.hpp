#pragma once
#include "ICommand.hpp"
#include "store/StoreEngine.hpp"
#include "pubsub/PubSubManager.hpp"

class UnsubscribeCommand : public ICommand {
private:
    std::string _channel;
    int _clientFd;
public:
    UnsubscribeCommand(const std::string& channel, int clientFd);
    std::string execute() override;
};  