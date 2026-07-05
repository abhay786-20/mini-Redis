#include "DataTypeFactory.hpp"
#include "StringType.hpp"
#include "ListType.hpp"
#include "HashType.hpp"
#include "SetType.hpp"
#include <stdexcept>

std::unique_ptr<IDataType> DataTypeFactory::create(const std::string& type) {
    if (type == "string") return std::make_unique<StringType>("");
    if (type == "list") return std::make_unique<ListType>(std::vector<std::string>());
    if (type == "hash") return std::make_unique<HashType>(std::unordered_map<std::string, std::string>());
    if (type == "set") return std::make_unique<SetType>(std::set<std::string>());
    throw std::invalid_argument("Unknown data type: " + type);
}