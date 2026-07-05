#include "HashType.hpp"
#include <stdexcept>

HashType::HashType(std::unordered_map<std::string, std::string> value) {
    _value = value;
}

std::string HashType::getType() {
    return "hash";
}

// TODO: delimiter breaks if keys/values contain ':' or '|'
std::string HashType::serialize() {
    std::string result = "";
    for (auto it = _value.begin(); it != _value.end(); ++it) {
        if (it != _value.begin()) result += "|";
        result += it->first + ":" + it->second;
    }
    return result;
}

std::unordered_map<std::string, std::string> HashType::getValue() {
    return _value;
}

void HashType::setField(const std::string& key, const std::string& val) {
    _value[key] = val;
}