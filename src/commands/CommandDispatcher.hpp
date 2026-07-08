#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "ICommand.hpp"
#include<vector>
#include<functional>

using CommandFactory = std::function<std::unique_ptr<ICommand>(const std::vector<std::string>&)>;

class CommandDispatcher {
private:
    std::unordered_map<std::string, CommandFactory> _commands;
public:
    void registerCommand(const std::string& name, CommandFactory factory);
    std::string dispatch(const std::vector<std::string>& tokens);
};