#pragma once
#include "ICommand.hpp"
#include "store/StoreEngine.hpp"

class SetCommand : public ICommand {
private:
    std::string _key;
    std::string _value;
    int64_t _ttlMs;
public:
    SetCommand(const std::string& key, const std::string& value, int64_t ttlMs = -1);
    std::string execute() override;
};