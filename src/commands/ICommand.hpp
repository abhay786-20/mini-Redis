#pragma once 
#include <string>

class ICommand {
public:
    virtual std::string execute() = 0;
    virtual ~ICommand() = default;
};