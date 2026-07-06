#pragma once 
#include "ICommand.hpp"
#include "store/StoreEngine.hpp"
#include "store/types/ListType.hpp"

class LPushCommand : public ICommand {
private:
    std::string _key;
    std::string _value;
public:
    LPushCommand(const std::string& key, const std::string& value);
    std::string execute() override;
};