#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "types/IDataType.hpp"
#include "eviction/IEvictionPolicy.hpp"
#include "DataEntry.hpp"

class StoreEngine {
private:
    std::unordered_map<std::string, DataEntry> _store;
    std::unique_ptr<IEvictionPolicy> _evictionPolicy;
    StoreEngine() = default;
public:
    static StoreEngine& getInstance();
    std::string get(const std::string& key);
    void set(const std::string& key, std::unique_ptr<IDataType> value, int64_t ttlMs = -1);
    void del(const std::string& key);
    DataEntry* getRaw(const std::string& key);
    void setEvictionPolicy(std::unique_ptr<IEvictionPolicy> policy);
};