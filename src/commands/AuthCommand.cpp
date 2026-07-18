#include "AuthCommand.hpp"
#include "auth/AuthProxy.hpp"

AuthCommand::AuthCommand(const std::string& password, int clientFd) : _password(password), _clientFd(clientFd) {}

std::string AuthCommand::execute() {
    auto& auth = AuthProxy::getInstance();
    if (!auth.isEnabled()) {
        return "-ERR Client sent AUTH, but no password is set\r\n";
    }
    if (auth.authenticate(_clientFd, _password)) {
        return "+OK\r\n";
    }
    return "-ERR invalid password\r\n";
}
