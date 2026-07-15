#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <mutex>

class AofWriter {
private:
    std::ofstream _file;
    std::mutex _mutex;
    AofWriter();
public:
    static AofWriter& getInstance();
    void append(const std::vector<std::string>& tokens);
};
