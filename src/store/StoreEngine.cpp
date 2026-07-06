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
    return it->second->serialize();
}

void StoreEngine::set(const std::string& key, std::unique_ptr<IDataType> value) {
   _store[key] = std::move(value);
}

void StoreEngine::del(const std::string& key) {
    _store.erase(key);
}   

IDataType* StoreEngine::getRaw(const std::string& key) {
    auto it = _store.find(key);
    if (it == _store.end()) return nullptr;
    return it->second.get();
}