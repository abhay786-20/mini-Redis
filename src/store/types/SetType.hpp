#pragma once
#include <string>
#include <set>
#include "IDataType.hpp"

class SetType : public IDataType {
private:
    std::set<std::string> _value;
public:
    SetType(std::set<std::string> value);
    std::string getType() override;
    std::string serialize() override;
    std::set<std::string> getValue();
    void addItem(const std::string& item);
};