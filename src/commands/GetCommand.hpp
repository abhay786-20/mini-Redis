#pragma once
#include "ICommand.hpp"
#include "store/StoreEngine.hpp"

class GetCommand : public ICommand {
private:
    std::string _key;
public:
    GetCommand(const std::string& key);
    std::string execute() override;
};