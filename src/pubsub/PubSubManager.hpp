#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

class PubSubManager {
private:
   std::unordered_map<std::string, std::vector<int>> _channels;
   std::mutex _mutex;
   PubSubManager() = default;
public:
    static PubSubManager& getInstance();
    void subscribe(const std::string& channel, int clientId);
    void unsubscribe(const std::string& channel, int clientId);
    void publish(const std::string& channel, const std::string& message);
};