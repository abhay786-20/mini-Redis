#pragma once
#include <string>
#include <unordered_map>
#include "IDataType.hpp"

class HashType : public IDataType {
private:
    std::unordered_map<std::string, std::string> _value;
public:
    HashType(std::unordered_map<std::string, std::string> value);
    std::string getType() override;
    std::string serialize() override;
    std::unordered_map<std::string, std::string> getValue();
    void setField(const std::string& key, const std::string& val);
};