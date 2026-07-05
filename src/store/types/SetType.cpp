#include "SetType.hpp"

SetType::SetType(std::set<std::string> value) {
    _value = value;
}

std::string SetType::getType() {
    return "set";
}

// TODO: delimiter breaks if values contain '|'
std::string SetType::serialize() {
    std::string result = "";
    for (auto it = _value.begin(); it != _value.end(); ++it) {
        if (it != _value.begin()) result += "|";
        result += *it;
    }
    return result;
}

std::set<std::string> SetType::getValue() {
    return _value;
}

void SetType::addItem(const std::string& item) {
    _value.insert(item);
}