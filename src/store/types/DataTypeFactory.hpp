#pragma once
#include <string>
#include <memory>
#include "IDataType.hpp"

class DataTypeFactory {
public:
    // We will fill this as we add StringType, ListType etc.
    static std::unique_ptr<IDataType> create(const std::string& type);
};