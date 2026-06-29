#include "StringType.hpp"

StringType::StringType(std::string value) {
    _value = value;
}

std::string StringType::getType() {
    return "string";
}

std::string StringType::serialize() {
    return _value;
}

std::string StringType::getValue() {
    return _value;
}

void StringType::setValue(std::string value) {
    _value = value;
}