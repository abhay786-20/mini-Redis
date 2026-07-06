#include "ListType.hpp"

ListType::ListType(std::vector<std::string> value) {
    _value = value;
}

std::string ListType::getType() {
    return "list";
}

// TODO: comma-separated serialization breaks if items contain commas.
// Will be replaced with proper RESP length-prefixed encoding in Week 2.
std::string ListType::serialize() {
    std::string serialized = "";
    for (size_t i = 0; i < _value.size(); i++) {
        serialized += _value[i];
        if (i != _value.size() - 1) {
            serialized += ",";
        }
    }
    return serialized;
}

std::vector<std::string> ListType::getValue() {
    return _value;
}

void ListType::setValue(std::vector<std::string> value) {
    _value = value;
}

void ListType::pushBack(const std::string& item) {
    _value.push_back(item);
}