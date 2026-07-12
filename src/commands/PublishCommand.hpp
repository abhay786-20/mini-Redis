#pragma once
#include "ICommand.hpp"
#include "store/StoreEngine.hpp"
#include "pubsub/PubSubManager.hpp"

class PublishCommand : public ICommand {
private:
    std::string _channel;
    std::string _message;
public:
    PublishCommand(const std::string& channel, const std::string& message);
    std::string execute() override;
};  