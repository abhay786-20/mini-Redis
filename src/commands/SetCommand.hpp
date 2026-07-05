#pragma once
#include "ICommand.hpp"
#include "store/StoreEngine.hpp"

class SetCommand : public ICommand {
private:
    std::string _key;
    std::string _value;
public:
    SetCommand(const std::string& key, const std::string& value);
    std::string execute() override;
};