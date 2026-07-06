#pragma once 
#include "ICommand.hpp"
#include "store/StoreEngine.hpp"
#include "store/types/HashType.hpp"

class HSetCommand : public ICommand {
private:
    std::string _key;
    std::string _field;
    std::string _value;
public:
    HSetCommand(const std::string& key, const std::string& field, const std::string& value);
    std::string execute() override;
};