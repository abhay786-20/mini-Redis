#include "DataTypeFactory.hpp"
#include "StringType.hpp"
#include "ListType.hpp"
#include <stdexcept>

std::unique_ptr<IDataType> DataTypeFactory::create(const std::string& type) {
    if (type == "string") {
        return std::make_unique<StringType>("");
    } else if (type == "list") {
        return std::make_unique<ListType>(std::vector<std::string>());
    }
    throw std::invalid_argument("Unknown data type: " + type);
}