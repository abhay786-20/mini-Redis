#pragma once
#include <string>
#include <vector>

class RespParser {
public:
    std::vector<std::string> parse(const std::string& raw);
};