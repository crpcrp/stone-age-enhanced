#include "settings.hpp"
#include <cstdio>

namespace stone {
const char* const Settings::file_name = "STONEAGE.ENH";

Settings::Settings(const Bytes& file) {
    std::string line;
    for (const uint8_t byte : file) {
        if (byte == '\n') {
            lines.push_back(line);
            line.clear();
        } else if (byte != '\r')
            line += char(byte);
    }
    if (!line.empty())
        lines.push_back(line);
}

Bytes Settings::bytes() const {
    Bytes file;
    for (const std::string& line : lines) {
        file.insert(file.end(), line.begin(), line.end());
        file.push_back('\n');
    }
    return file;
}

bool Settings::enhanced() const {
    std::string value;
    return !find("enhanced", value) || value != "0";
}

void Settings::set_enhanced(bool on) { set("enhanced", on ? "1" : "0"); }

namespace {
const char* const part_keys[Settings::parts] = {"images", "maps", "music"};
} // namespace

bool Settings::original(Part part) const {
    std::string value;
    return !find(part_keys[part], value) || value != "free";
}

void Settings::set_original(Part part, bool on) {
    set(part_keys[part], on ? "original" : "free");
}

namespace {
std::string reached_key(uint32_t levels) {
    char key[20];
    std::snprintf(key, sizeof key, "reached %08x", unsigned(levels));
    return key;
}
} // namespace

unsigned Settings::reached(uint32_t levels) const {
    std::string value;
    if (!find(reached_key(levels), value) || value.empty() || value.size() > 3 ||
        value.find_first_not_of("0123456789") != std::string::npos)
        return 1;
    const unsigned level = unsigned(std::stoul(value));
    return level >= 1 && level <= 100 ? level : 1;
}

void Settings::set_reached(uint32_t levels, unsigned level) {
    set(reached_key(levels), std::to_string(level));
}

bool Settings::find(const std::string& key, std::string& value) const {
    for (const std::string& line : lines)
        if (line.compare(0, key.size() + 1, key + ' ') == 0) {
            value = line.substr(key.size() + 1);
            return true;
        }
    return false;
}

void Settings::set(const std::string& key, const std::string& value) {
    for (std::string& line : lines)
        if (line.compare(0, key.size() + 1, key + ' ') == 0) {
            line = key + ' ' + value;
            return;
        }
    lines.push_back(key + ' ' + value);
}
} // namespace stone
