#pragma once
#include <string>
#include <memory>
#include <cstdint>
#include "types/IDataType.hpp"

class DataEntry {
private:
    std::unique_ptr<IDataType> _data;
    int64_t _expiryMs;
public:
    DataEntry();  // default constructor
    DataEntry(std::unique_ptr<IDataType> data, int64_t ttlMs = -1);  // main constructor
    bool isExpired() const;
    IDataType* getData();
};