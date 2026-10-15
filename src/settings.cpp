#include "settings.hpp"
#include <algorithm>
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
const char* const display_names[Settings::displays] = {"sharp", "crt", "smooth"};
} // namespace

const char* Settings::display_name(Display display) { return display_names[display]; }

Settings::Display Settings::display() const {
    std::string value;
    if (find("display", value))
        for (unsigned n = 0; n < displays; ++n)
            if (value == display_names[n])
                return Display(n);
    return sharp;
}

void Settings::set_display(Display display) { set("display", display_names[display]); }

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

namespace {
std::string best_key(uint32_t levels, unsigned level) {
    char text[32];
    std::snprintf(text, sizeof text, "best %08x %u", unsigned(levels), level);
    return text;
}

// A number of up to nine digits; false for anything else.
bool read_number(const std::string& text, unsigned& number) {
    if (text.empty() || text.size() > 9 || text.find_first_not_of("0123456789") != std::string::npos)
        return false;
    number = unsigned(std::stoul(text));
    return true;
}
} // namespace

Settings::Best Settings::best(uint32_t levels, unsigned level) const {
    std::string value;
    Best best;
    if (!find(best_key(levels, level), value))
        return best;
    const std::size_t space = value.find(' ');
    if (space == std::string::npos || !read_number(value.substr(0, space), best.ticks) ||
        !read_number(value.substr(space + 1), best.points))
        return Best();
    return best;
}

unsigned Settings::improve_best(uint32_t levels, unsigned level, unsigned ticks,
                                unsigned points) {
    Best best = this->best(levels, level);
    const bool first = !best.cleared();
    ticks = std::max(ticks, 1u);
    unsigned beaten = 0;
    if (first || ticks < best.ticks) {
        best.ticks = ticks;
        beaten |= faster;
    }
    if (first || points > best.points) {
        best.points = points;
        beaten |= higher;
    }
    if (beaten)
        set(best_key(levels, level), std::to_string(best.ticks) + ' ' + std::to_string(best.points));
    return beaten;
}

namespace {
// `count` numbers of up to 18 digits, one space apart; false for anything else.
bool read_numbers(const std::string& text, uint64_t* numbers, unsigned count) {
    std::size_t at = 0;
    for (unsigned n = 0; n < count; ++n) {
        const std::size_t end = n + 1 < count ? text.find(' ', at) : text.size();
        if (end == std::string::npos)
            return false;
        const std::string number = text.substr(at, end - at);
        if (number.empty() || number.size() > 18 ||
            number.find_first_not_of("0123456789") != std::string::npos)
            return false;
        numbers[n] = std::stoull(number);
        at = end + 1;
    }
    return true;
}

std::string write_numbers(const uint64_t* numbers, unsigned count) {
    std::string text;
    for (unsigned n = 0; n < count; ++n)
        text += (n ? " " : "") + std::to_string(numbers[n]);
    return text;
}

std::string stats_key(uint32_t levels, unsigned level) {
    char text[32];
    std::snprintf(text, sizeof text, "stats %08x %u", unsigned(levels), level);
    return text;
}
} // namespace

Settings::MapStats Settings::map_stats(uint32_t levels, unsigned level) const {
    std::string value;
    uint64_t numbers[5];
    MapStats stats;
    if (find(stats_key(levels, level), value) && read_numbers(value, numbers, 5)) {
        stats.attempts = numbers[0];
        stats.clears = numbers[1];
        stats.lives_lost = numbers[2];
        stats.give_ups = numbers[3];
        stats.ticks = numbers[4];
    }
    return stats;
}

void Settings::set_map_stats(uint32_t levels, unsigned level, const MapStats& stats) {
    const uint64_t numbers[5] = {stats.attempts, stats.clears, stats.lives_lost, stats.give_ups,
                                 stats.ticks};
    set(stats_key(levels, level), write_numbers(numbers, 5));
}

Settings::Totals Settings::totals() const {
    std::string value;
    uint64_t numbers[9];
    Totals totals;
    if (find("totals", value) && read_numbers(value, numbers, 9)) {
        totals.ticks = numbers[0];
        totals.games = numbers[1];
        totals.cleared = numbers[2];
        totals.steps = numbers[3];
        totals.hourglasses = numbers[4];
        totals.keys = numbers[5];
        totals.gates = numbers[6];
        totals.teleports = numbers[7];
        totals.platforms = numbers[8];
    }
    return totals;
}

void Settings::set_totals(const Totals& totals) {
    const uint64_t numbers[9] = {totals.ticks, totals.games,       totals.cleared,
                                 totals.steps, totals.hourglasses, totals.keys,
                                 totals.gates, totals.teleports,   totals.platforms};
    set("totals", write_numbers(numbers, 9));
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
