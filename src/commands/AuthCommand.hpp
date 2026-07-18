#pragma once
#include <string>
#include "ICommand.hpp"

class AuthCommand : public ICommand {
private:
    std::string _password;
    int _clientFd;
public:
    AuthCommand(const std::string& password, int clientFd);
    std::string execute() override;
};
