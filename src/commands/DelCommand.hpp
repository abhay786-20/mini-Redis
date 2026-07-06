#pragma once 
#include "ICommand.hpp"
#include "store/StoreEngine.hpp"

class DelCommand : public ICommand {
private:
    std::string _key;
public:
    DelCommand(const std::string& key);
    std::string execute() override;
};