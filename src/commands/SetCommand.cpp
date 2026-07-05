#include "SetCommand.hpp"
#include "store/types/StringType.hpp" 

SetCommand::SetCommand(const std::string& key, const std::string& value) : _key(key), _value(value) {}

std::string SetCommand::execute() {
    auto& store = StoreEngine::getInstance();
    auto val = std::make_unique<StringType>(_value);
    store.set(_key, std::move(val));
    return "OK";
}