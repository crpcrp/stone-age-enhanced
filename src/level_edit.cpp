#include "level_edit.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>

namespace stone {
namespace {
// The rock's nine letters, row by row.
bool rock_letter(uint8_t tile) { return tile >= 'b' && tile <= 'j'; }
} // namespace

const uint8_t LevelRecord::rock;

LevelRecord LevelRecord::empty() {
    LevelRecord level;
    level.set_time(60);
    level.set_theme(0);
    level.set_palette(0);
    level.set_background(0);
    level.bytes[6] = '0'; // the flag
    for (unsigned row = 0; row < rows; ++row)
        for (unsigned column = 0; column < columns; ++column) {
            const bool edge = row == 0 || row == rows - 1 || column == 0 || column == columns - 1;
            level.set(column, row, edge ? 'L' : 'B');
        }
    level.set(2, 5, 'R');
    level.set(17, 5, 'S');
    return level;
}

unsigned LevelRecord::time() const {
    unsigned value = 0;
    for (unsigned n = 0; n < 3; ++n)
        value = value * 10 + unsigned(bytes[n] - '0') % 10;
    return value;
}

void LevelRecord::set_time(unsigned time) {
    char digits[8];
    std::snprintf(digits, sizeof digits, "%03u", std::min(time, 999u));
    std::copy_n(digits, 3, bytes.begin());
}

const std::vector<uint8_t>& LevelRecord::tiles() {
    static const std::vector<uint8_t> all = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                                             'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'a',
                                             rock};
    return all;
}

bool LevelRecord::rock_origin(unsigned column, unsigned row, unsigned& left, unsigned& top) const {
    const uint8_t tile = at(column, row);
    if (!rock_letter(tile))
        return false;
    const unsigned piece = unsigned(tile - 'b');
    if (column < piece % 3 || row < piece / 3)
        return false;
    left = column - piece % 3;
    top = row - piece / 3;
    if (left + 3 > columns || top + 3 > rows)
        return false;
    for (unsigned n = 0; n < 9; ++n)
        if (at(left + n % 3, top + n / 3) != 'b' + n)
            return false;
    return true;
}

void LevelRecord::remove_rock(unsigned column, unsigned row) {
    unsigned left, top;
    if (rock_origin(column, row, left, top))
        for (unsigned n = 0; n < 9; ++n)
            set(left + n % 3, top + n / 3, 'A');
}

bool LevelRecord::place(unsigned column, unsigned row, uint8_t tile) {
    if (column >= columns || row >= rows)
        return false;
    const LevelRecord before = *this;
    if (tile == rock) {
        if (column + 3 > columns || row + 3 > rows)
            return false;
        for (unsigned n = 0; n < 9; ++n)
            remove_rock(column + n % 3, row + n / 3);
        for (unsigned n = 0; n < 9; ++n)
            set(column + n % 3, row + n / 3, uint8_t('b' + n));
        return *this != before;
    }
    remove_rock(column, row);
    if (tile == 'R') // one start: it moves here
        for (unsigned cell = 0; cell < columns * rows; ++cell)
            if (bytes[7 + cell] == 'R')
                bytes[7 + cell] = 'B';
    set(column, row, tile);
    return *this != before;
}

LevelRecord::Check LevelRecord::check() const {
    unsigned count[128] = {};
    unsigned loose_rock = 0;
    for (unsigned row = 0; row < rows; ++row)
        for (unsigned column = 0; column < columns; ++column) {
            const uint8_t tile = at(column, row);
            if (tile < 128)
                ++count[tile];
            unsigned left, top;
            if (rock_letter(tile) && !rock_origin(column, row, left, top))
                ++loose_rock;
        }
    Check result;
    if (count['R'] == 0)
        result.errors.push_back("NO START");
    else if (count['R'] > 1)
        result.errors.push_back(std::to_string(count['R']) + " STARTS");
    if (count['S'] == 0)
        result.errors.push_back("NO EXIT");
    const struct {
        uint8_t key, gate;
        const char* colour;
    } pairs[] = {{'O', 'M', "RED"}, {'P', 'N', "GREEN"}};
    for (const auto& pair : pairs) {
        if (count[pair.key] && !count[pair.gate])
            result.warnings.push_back(std::string(pair.colour) + " KEY, NO GATE");
        else if (count[pair.gate] && !count[pair.key])
            result.warnings.push_back(std::string(pair.colour) + " GATE, NO KEY");
    }
    if (loose_rock)
        result.warnings.push_back("PART OF A ROCK");
    return result;
}

std::string pack_file_name(const std::string& name) {
    std::string file;
    for (const char c : name)
        file += std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' ? c : '_';
    return file + ".txt";
}
} // namespace stone
