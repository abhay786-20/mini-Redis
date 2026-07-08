#include "RespParser.hpp"
#include <sstream>

std::vector<std::string> RespParser::parse(const std::string& raw) {
    // step 1 — split by \r\n into lines
    // your code here
    std::vector<std::string> lines;
    std::istringstream iss(raw);
    std::string line;
   while (std::getline(iss, line)) {
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    lines.push_back(line);
}
    // step 2 — filter lines, skip * and $ prefixed
    // your code here
    std::vector<std::string> result;
    for (auto& line : lines) {
        if (line.size() == 0) continue;
        if (line[0] == '*' || line[0] == '$') {
            continue;
        }
        result.push_back(line);
    }

    // step 3 — return result
    return result;
}