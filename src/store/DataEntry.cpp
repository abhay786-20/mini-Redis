#include "DataEntry.hpp"
#include <chrono>

DataEntry::DataEntry() : _data(nullptr), _expiryMs(-1) {}
DataEntry::DataEntry(std::unique_ptr<IDataType> data, int64_t ttlMs) {
    _data = std::move(data);
    if (ttlMs == -1) {
        _expiryMs = -1;
    } else {
        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        _expiryMs = now + ttlMs;
    }
}

bool DataEntry::isExpired() const {
    if (_expiryMs == -1) return false;
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    return now > _expiryMs;
}

IDataType* DataEntry::getData() {
    return _data.get();
}

int64_t DataEntry::getExpiryMs() const {
    return _expiryMs;
}