#include "SetCommand.hpp"
#include "store/types/StringType.hpp" 

SetCommand::SetCommand(const std::string& key, const std::string& value, int64_t ttlMs) : _key(key), _value(value), _ttlMs(ttlMs) {}

std::string SetCommand::execute() {
    auto& store = StoreEngine::getInstance();
    auto val = std::make_unique<StringType>(_value);
    store.set(_key, std::move(val), _ttlMs);
    return "OK";
}