#include "StoreEngine.hpp"
#include <stdexcept>

StoreEngine& StoreEngine::getInstance() {
    static StoreEngine instance;
    return instance;
}

std::string StoreEngine::get(const std::string& key) {
    auto it = _store.find(key);
    if (it == _store.end()) {
        throw std::invalid_argument("Key not found: " + key);
    }
    if (it->second.isExpired()) {
        _store.erase(it);
        throw std::invalid_argument("Key expired: " + key);
    }
    if (_evictionPolicy) _evictionPolicy->onGet(key);
    return it->second.getData()->serialize();
}

void StoreEngine::set(const std::string& key, std::unique_ptr<IDataType> value, int64_t ttlMs) {
    _store[key] = DataEntry(std::move(value), ttlMs);
    if (_evictionPolicy) _evictionPolicy->onSet(key);
}

void StoreEngine::del(const std::string& key) {
    _store.erase(key);
}

DataEntry* StoreEngine::getRaw(const std::string& key) {
    auto it = _store.find(key);
    if (it == _store.end()) return nullptr;
    return &it->second;
}

void StoreEngine::setEvictionPolicy(std::unique_ptr<IEvictionPolicy> policy) {
    _evictionPolicy = std::move(policy);
}