#include "HSetCommand.hpp"
#include <stdexcept>

HSetCommand::HSetCommand(const std::string& key, const std::string& field, const std::string& value) : _key(key), _field(field), _value(value) {}

std::string HSetCommand::execute() {
    auto& store = StoreEngine::getInstance();
    DataEntry* entry = store.getRaw(_key);

    if (entry == nullptr) {
        auto newHash = std::make_unique<HashType>(std::unordered_map<std::string, std::string>{{_field, _value}});
        store.set(_key, std::move(newHash));
        return "(integer) 1";
    }

    if (entry->getData()->getType() != "hash") {
        throw std::invalid_argument("WRONGTYPE: key holds wrong type");
    }

    HashType* hash = static_cast<HashType*>(entry->getData());
    hash->setField(_field, _value);
    return "(integer) 1";
}