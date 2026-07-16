#include "SnapshotWriter.hpp"
#include "store/StoreEngine.hpp"
#include "store/types/StringType.hpp"
#include "store/types/ListType.hpp"
#include "store/types/HashType.hpp"
#include "store/types/SetType.hpp"
#include <fstream>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <cstring>

namespace {

const char MAGIC[4] = {'M', 'R', 'D', 'B'};
constexpr uint8_t VERSION = 1;

enum TypeTag : uint8_t { STRING = 0, LIST = 1, HASH = 2, SET = 3 };

int64_t nowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

void writeU32(std::ofstream& out, uint32_t value) {
    out.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void writeI64(std::ofstream& out, int64_t value) {
    out.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void writeString(std::ofstream& out, const std::string& value) {
    writeU32(out, static_cast<uint32_t>(value.size()));
    out.write(value.data(), value.size());
}

uint32_t readU32(std::ifstream& in) {
    uint32_t value;
    in.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

int64_t readI64(std::ifstream& in) {
    int64_t value;
    in.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

std::string readString(std::ifstream& in) {
    uint32_t len = readU32(in);
    std::string value(len, '\0');
    in.read(value.data(), len);
    return value;
}

} // namespace

void SnapshotWriter::save(const std::string& path) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to open " + path + " for writing");
    }

    out.write(MAGIC, sizeof(MAGIC));
    out.put(static_cast<char>(VERSION));

    int saved = 0;
    StoreEngine::getInstance().forEachEntry([&](const std::string& key, DataEntry& entry) {
        IDataType* data = entry.getData();
        writeString(out, key);
        writeI64(out, entry.getExpiryMs());

        if (auto* s = dynamic_cast<StringType*>(data)) {
            out.put(static_cast<char>(STRING));
            writeString(out, s->getValue());
        } else if (auto* l = dynamic_cast<ListType*>(data)) {
            out.put(static_cast<char>(LIST));
            const auto& items = l->getValue();
            writeU32(out, static_cast<uint32_t>(items.size()));
            for (const auto& item : items) writeString(out, item);
        } else if (auto* h = dynamic_cast<HashType*>(data)) {
            out.put(static_cast<char>(HASH));
            const auto& fields = h->getValue();
            writeU32(out, static_cast<uint32_t>(fields.size()));
            for (const auto& [k, v] : fields) {
                writeString(out, k);
                writeString(out, v);
            }
        } else if (auto* st = dynamic_cast<SetType*>(data)) {
            out.put(static_cast<char>(SET));
            const auto& items = st->getValue();
            writeU32(out, static_cast<uint32_t>(items.size()));
            for (const auto& item : items) writeString(out, item);
        } else {
            throw std::runtime_error("Unknown data type during snapshot save for key: " + key);
        }
        saved++;
    });

    out.flush();
    std::cout << "Snapshot saved to " << path << ": " << saved << " keys" << std::endl;
}

bool SnapshotWriter::load(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        return false;
    }

    char magic[4];
    in.read(magic, sizeof(magic));
    if (in.gcount() != sizeof(magic) || std::memcmp(magic, MAGIC, sizeof(MAGIC)) != 0) {
        std::cerr << "Snapshot file " << path << " has invalid header, ignoring" << std::endl;
        return false;
    }
    in.get(); // version byte, unused for now

    auto& store = StoreEngine::getInstance();
    int loaded = 0;
    int expired = 0;

    while (in.peek() != std::ifstream::traits_type::eof()) {
        std::string key = readString(in);
        int64_t expiryMs = readI64(in);
        uint8_t tag = static_cast<uint8_t>(in.get());

        std::unique_ptr<IDataType> value;
        switch (tag) {
            case STRING: {
                value = std::make_unique<StringType>(readString(in));
                break;
            }
            case LIST: {
                uint32_t count = readU32(in);
                std::vector<std::string> items;
                items.reserve(count);
                for (uint32_t i = 0; i < count; ++i) items.push_back(readString(in));
                value = std::make_unique<ListType>(std::move(items));
                break;
            }
            case HASH: {
                uint32_t count = readU32(in);
                std::unordered_map<std::string, std::string> fields;
                for (uint32_t i = 0; i < count; ++i) {
                    std::string k = readString(in);
                    std::string v = readString(in);
                    fields[k] = v;
                }
                value = std::make_unique<HashType>(std::move(fields));
                break;
            }
            case SET: {
                uint32_t count = readU32(in);
                std::set<std::string> items;
                for (uint32_t i = 0; i < count; ++i) items.insert(readString(in));
                value = std::make_unique<SetType>(std::move(items));
                break;
            }
            default:
                throw std::runtime_error("Corrupt snapshot: unknown type tag for key " + key);
        }

        if (expiryMs != -1) {
            int64_t remaining = expiryMs - nowMs();
            if (remaining <= 0) {
                expired++;
                continue;
            }
            store.set(key, std::move(value), remaining);
        } else {
            store.set(key, std::move(value), -1);
        }
        loaded++;
    }

    std::cout << "Snapshot loaded from " << path << ": " << loaded << " keys restored"
               << (expired > 0 ? (", " + std::to_string(expired) + " expired keys skipped") : "")
               << std::endl;
    return true;
}
