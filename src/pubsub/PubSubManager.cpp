#include "PubSubManager.hpp"
#include <stdexcept>
#include <unistd.h>
#include <algorithm>

PubSubManager& PubSubManager::getInstance() {
    static PubSubManager instance;
    return instance;
}

void PubSubManager::subscribe(const std::string& channel, int clientId) {
    std::lock_guard<std::mutex> lock(_mutex);
    _channels[channel].push_back(clientId);
}

void PubSubManager::unsubscribe(const std::string& channel, int clientId) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_channels.find(channel) == _channels.end()) return;
    auto& clients = _channels[channel];
    auto it = std::find(clients.begin(), clients.end(), clientId);
    if (it == clients.end()) return;
    clients.erase(it);
}

void PubSubManager::publish(const std::string& channel, const std::string& message) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_channels.find(channel) == _channels.end()) return;
    std::string resp = "*3\r\n$7\r\nmessage\r\n$" + std::to_string(channel.size())
    + "\r\n" + channel + "\r\n$" + std::to_string(message.size())
    + "\r\n" + message + "\r\n";
    for (auto clientFd : _channels[channel]) {
        write(clientFd, resp.c_str(), resp.size());
    }
}