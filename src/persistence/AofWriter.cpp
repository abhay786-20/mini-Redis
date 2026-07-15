#include "AofWriter.hpp"
#include <stdexcept>

AofWriter::AofWriter() {
    _file.open("appendonly.aof", std::ios::app);
    if (!_file.is_open()) {
        throw std::runtime_error("Failed to open appendonly.aof for writing");
    }
}

AofWriter& AofWriter::getInstance() {
    static AofWriter instance;
    return instance;
}

void AofWriter::append(const std::vector<std::string>& tokens) {
    std::lock_guard<std::mutex> lock(_mutex);
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0) _file << ' ';
        _file << tokens[i];
    }
    _file << '\n';
    _file.flush();
}
