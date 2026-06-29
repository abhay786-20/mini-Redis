#pragma once
#include <string>
#include "IDataType.hpp"

class StringType : public IDataType {
private:
    std::string _value;

public:
    StringType(std::string value);

    std::string getType() override;
    std::string serialize() override;

    std::string getValue();
    void setValue(std::string value);
};