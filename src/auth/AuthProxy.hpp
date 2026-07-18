#pragma once
#include <string>
#include <unordered_set>
#include <mutex>
#include <vector>
#include "commands/CommandDispatcher.hpp"

class AuthProxy {
private:
    std::string _password;
    std::unordered_set<int> _authenticatedClients;
    std::mutex _mutex;
    AuthProxy() = default;
public:
    static AuthProxy& getInstance();
    void setPassword(const std::string& password);
    bool isEnabled() const;
    bool authenticate(int clientFd, const std::string& password);
    bool isAuthenticated(int clientFd);
    void removeClient(int clientFd);
    std::string dispatch(CommandDispatcher& dispatcher, const std::vector<std::string>& tokens, int clientFd, bool logToAof = true);
};
