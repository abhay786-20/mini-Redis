#pragma once
#include <string>

class IDataType {
public:
   virtual std::string getType() = 0;
   virtual std::string serialize() = 0;
   virtual ~IDataType() = default;
};