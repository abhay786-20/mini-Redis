#pragma once
#include <list>
#include <unordered_map>
#include <string>
#include "IEvictionPolicy.hpp"

class LRUPolicy : public IEvictionPolicy {
private:
    std::list<std::string> _order;
    std::unordered_map<std::string, std::list<std::string>::iterator> _map;
public:
    void onGet(const std::string& key) override;
    void onSet(const std::string& key) override;
    std::string evict() override;
};