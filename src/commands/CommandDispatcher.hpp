#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "ICommand.hpp"

class CommandDispatcher {
private:
    std::unordered_map<std::string, std::unique_ptr<ICommand>> _commands;
public:
    void registerCommand(const std::string& name, std::unique_ptr<ICommand> command);
    std::string dispatch(const std::string& command);
};