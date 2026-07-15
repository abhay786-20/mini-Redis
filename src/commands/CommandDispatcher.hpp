#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "ICommand.hpp"
#include<vector>
#include<functional>

using CommandFactory = std::function<std::unique_ptr<ICommand>(const std::vector<std::string>&, int)>;

class CommandDispatcher {
private:
    std::unordered_map<std::string, CommandFactory> _commands;
    std::unordered_set<std::string> _writeCommands;
public:
    void registerCommand(const std::string& name, CommandFactory factory, bool isWriteCommand = false);
    std::string dispatch(const std::vector<std::string>& tokens, int clientFd, bool logToAof = true);
};