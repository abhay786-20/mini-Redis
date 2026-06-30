#pragma once 
#include<string>
#include<vector>
#include "IDataType.hpp"

class ListType : public IDataType {
private:
    std::vector<std::string> _value;
public:
    ListType(std::vector<std::string> value);
    std::string getType() override;
    std::string serialize() override;

    std::vector<std::string> getValue();
    void setValue(std::vector<std::string> value);
};  