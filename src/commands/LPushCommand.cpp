#include "LPushCommand.hpp"
#include <stdexcept>

LPushCommand::LPushCommand(const std::string& key, const std::string& value) : _key(key), _value(value) {}

std::string LPushCommand::execute() {
    auto& store = StoreEngine::getInstance();
    DataEntry* entry = store.getRaw(_key);

    if (entry == nullptr) {
        auto newList = std::make_unique<ListType>(std::vector<std::string>{_value});
        store.set(_key, std::move(newList));
        return "(integer) 1";
    }

    if (entry->getData()->getType() != "list") {
        throw std::invalid_argument("WRONGTYPE: key holds wrong type");
    }

    ListType* list = static_cast<ListType*>(entry->getData());
    list->pushBack(_value);
    return "(integer) 1";
}