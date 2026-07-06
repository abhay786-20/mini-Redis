#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "types/IDataType.hpp"

class StoreEngine {
private:
    std::unordered_map<std::string, std::unique_ptr<IDataType>> _store;
    StoreEngine() = default;
public:
    static StoreEngine& getInstance();
    std::string get(const std::string& key);
    void set(const std::string& key, std::unique_ptr<IDataType> value);
    void del(const std::string& key);
    IDataType* getRaw(const std::string& key);
};