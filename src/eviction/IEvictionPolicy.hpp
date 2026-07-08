#pragma once
#include <string>

class IEvictionPolicy {
public:
    virtual void onGet(const std::string& key) = 0;
    virtual void onSet(const std::string& key) = 0;
    virtual std::string evict() = 0;
    virtual ~IEvictionPolicy() = default;
};