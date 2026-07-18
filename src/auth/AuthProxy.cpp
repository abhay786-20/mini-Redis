#include "AuthProxy.hpp"

AuthProxy& AuthProxy::getInstance() {
    static AuthProxy instance;
    return instance;
}

void AuthProxy::setPassword(const std::string& password) {
    _password = password;
}

bool AuthProxy::isEnabled() const {
    return !_password.empty();
}

bool AuthProxy::authenticate(int clientFd, const std::string& password) {
    if (password != _password) return false;
    std::lock_guard<std::mutex> lock(_mutex);
    _authenticatedClients.insert(clientFd);
    return true;
}

bool AuthProxy::isAuthenticated(int clientFd) {
    std::lock_guard<std::mutex> lock(_mutex);
    return _authenticatedClients.count(clientFd) > 0;
}

void AuthProxy::removeClient(int clientFd) {
    std::lock_guard<std::mutex> lock(_mutex);
    _authenticatedClients.erase(clientFd);
}

std::string AuthProxy::dispatch(CommandDispatcher& dispatcher, const std::vector<std::string>& tokens, int clientFd, bool logToAof) {
    if (isEnabled() && !tokens.empty() && tokens[0] != "AUTH" && !isAuthenticated(clientFd)) {
        return "-NOAUTH Authentication required.\r\n";
    }
    return dispatcher.dispatch(tokens, clientFd, logToAof);
}
