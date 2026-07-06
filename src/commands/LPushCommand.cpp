#include "LPushCommand.hpp"

#include <stdexcept>

LPushCommand::LPushCommand(const std::string& key, const std::string& value) : _key(key), _value(value) {}

std::string LPushCommand::execute() {
    auto& store = StoreEngine::getInstance();
    IDataType* raw = store.getRaw(_key);

    if (raw == nullptr) {
        // key doesn't exist — create new list
        auto newList = std::make_unique<ListType>(std::vector<std::string>{_value});
        store.set(_key, std::move(newList));
        return "(integer) 1";
    }

    // key exists — must be a list
    if (raw->getType() != "list") {
        throw std::invalid_argument("WRONGTYPE: key holds wrong type");
    }

    ListType* list = static_cast<ListType*>(raw);
    list->pushBack(_value);
    return "(integer) 1";
}