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
std::string level_key(const char* key, uint32_t levels) {
    char text[24];
    std::snprintf(text, sizeof text, "%s %08x", key, unsigned(levels));
    return text;
}
} // namespace

unsigned Settings::level(const char* key, uint32_t levels) const {
    std::string value;
    if (!find(level_key(key, levels), value) || value.empty() || value.size() > 3 ||
        value.find_first_not_of("0123456789") != std::string::npos)
        return 0;
    const unsigned level = unsigned(std::stoul(value));
    return level >= 1 && level <= 100 ? level : 0;
}

unsigned Settings::reached(uint32_t levels) const {
    const unsigned furthest = level("reached", levels);
    return furthest ? furthest : 1;
}

void Settings::set_reached(uint32_t levels, unsigned level) {
    set(level_key("reached", levels), std::to_string(level));
}

unsigned Settings::last(uint32_t levels) const { return level("last", levels); }

void Settings::set_last(uint32_t levels, unsigned level) {
    set(level_key("last", levels), std::to_string(level));
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
