#pragma once
#include <string>

class SnapshotWriter {
public:
    static void save(const std::string& path = "dump.rdb");
    // Returns true if a snapshot file was found and loaded.
    static bool load(const std::string& path = "dump.rdb");
};
