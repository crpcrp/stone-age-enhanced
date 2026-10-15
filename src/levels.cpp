#include "levels.hpp"
#include "machine.hpp"
#include <algorithm>
#include <cstdio>
#include <set>
#include <sstream>

namespace stone {
namespace {
const uint16_t consonants_at = 0x1f06, vowels_at = 0x1f15;

const char* const levels_help =
    "# Stone Age levels. Each level is a header line and 11 rows of 20 tiles.\n"
    "#   time        countdown parameter, 0..999 (shown value is time + time/4)\n"
    "#   theme       tile set 0..4 (tiles_1..5.bmp)\n"
    "#   palette     color set of the tiles, 0..5\n"
    "#   background  0..4 (background_a1..5 or background_b1..5)\n"
    "#   flag        fourth header byte; the game reads it but does not use it\n"
    "#   password    six letters, a consonant and a vowel in turn; consonants are\n"
    "#               D M T S P F B R N L V W H K C, vowels U I A O E\n"
    "# A level pack in levels/ beside the player holds 1 to 100 levels; it may\n"
    "# start with a line `name <text>`, and a level without a password gets one.\n"
    "#\n"
    "# Tile codes. The picture of each tile is in tiles_1..5.bmp, eight per row,\n"
    "# in this order: A B C D E F G H / I J K L M N O P / Q R S ...\n"
    "#   A  empty space: no floor to walk on; platforms travel across it\n"
    "#   B  floor\n"
    "#   C  platform that moves up\n"
    "#   D  platform that moves down\n"
    "#   E  platform that moves right\n"
    "#   F  platform that moves left\n"
    "#   G  platform that moves up or down\n"
    "#   H  platform that moves left or right\n"
    "#   I  platform that moves in all four directions\n"
    "#   J  hourglass: time bonus\n"
    "#   K  cracked floor: crumbles away after it is stepped on\n"
    "#   L  wall block\n"
    "#   M  red gate, opened with a red key\n"
    "#   N  green gate, opened with a green key\n"
    "#   O  red key\n"
    "#   P  green key\n"
    "#   Q  teleport\n"
    "#   R  start position (exactly one per level)\n"
    "#   S  exit\n"
    "#   a  outside: the black area around the level\n"
    "#   b c d   the large decorative rock, three by three tiles:\n"
    "#   e f g   use all nine letters together, in this arrangement\n"
    "#   h i j\n"
    "#   k l m   stages of a crumbling floor; the game writes these itself\n";

// Passwords are three bytes a level (DS:1f24), each a consonant by its high
// nibble and a vowel by its low one (CS:0b92), in the alphabet of the program:
// fifteen consonants at DS:1f06 and fifteen vowels at DS:1f15, as built in.
struct Alphabet {
    uint8_t consonants[15], vowels[15];
    Alphabet() {
        Machine image;
        std::copy_n(&image.data(consonants_at), 15, consonants);
        std::copy_n(&image.data(vowels_at), 15, vowels);
    }
    std::string letters(const uint8_t* code) const {
        std::string s;
        for (int n = 0; n < 3; ++n)
            s += {char(consonants[code[n] >> 4 < 15 ? code[n] >> 4 : 0]),
                  char(vowels[(code[n] & 15) < 15 ? code[n] & 15 : 0])};
        return s;
    }
    // The first code that spells the letters; false when none does.
    bool code(const std::string& letters, uint8_t* code) const {
        if (letters.size() != 6)
            return false;
        for (int n = 0; n < 3; ++n) {
            const auto c = std::find(consonants, consonants + 15, uint8_t(letters[2 * n]));
            const auto v = std::find(vowels, vowels + 15, uint8_t(letters[2 * n + 1]));
            if (c == consonants + 15 || v == vowels + 15)
                return false;
            code[n] = uint8_t((c - consonants) << 4 | (v - vowels));
        }
        return true;
    }
};
const Alphabet& alphabet() {
    static const Alphabet built_in;
    return built_in;
}
} // namespace

std::string password_letters(const LevelSet& set, unsigned level);

namespace {
// A password for `level` from the sequence `seed` goes on, spelt unlike
// those `taken`, which it joins.
void make_password(LevelSet& set, unsigned level, uint32_t& seed, std::set<std::string>& taken) {
    uint8_t* const code = &set.passwords[3 * level];
    do {
        for (int pair = 0; pair < 3; ++pair) {
            seed = seed * 1103515245u + 12345u;
            code[pair] = uint8_t((seed >> 16) % 15 << 4 | (seed >> 24) % 5);
        }
    } while (!taken.insert(password_letters(set, level)).second);
    alphabet().code(password_letters(set, level), code); // spelt as a file reads it back
}
} // namespace

LevelSet read_levels(const Bytes& file, const std::string& path, bool pack) {
    const Alphabet& letters = alphabet();
    std::istringstream text(std::string(file.begin(), file.end()));
    LevelSet set;
    std::vector<bool> has_password;
    std::string line;
    unsigned number = 0, header = 0; // lines read; the line of the level's header
    auto fail = [&](const std::string& why, unsigned at) {
        throw LevelError(path + ", line " + std::to_string(at) + ": " + why, at);
    };
    auto next_line = [&]() {
        if (!std::getline(text, line))
            return false;
        ++number;
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        return true;
    };
    while (next_line()) {
        if (line.empty() || line[0] == '#')
            continue;
        if (pack && line.compare(0, 5, "name ") == 0) {
            if (set.count || !set.name.empty())
                fail("the name goes before the first level", number);
            set.name = line.substr(5);
            if (set.name.find_first_not_of(' ') == std::string::npos)
                fail("the name is empty", number);
            continue;
        }
        header = number;
        int level, time, theme, palette, background, flag, end = 0;
        if (std::sscanf(line.c_str(),
                        "level %d time %d theme %d palette %d background %d flag %d%n", &level,
                        &time, &theme, &palette, &background, &flag, &end) != 6)
            fail("a header is `level N time T theme T palette P background B flag F`", header);
        if (level != int(set.count) + 1)
            fail("level " + std::to_string(set.count + 1) + " must come next", header);
        if (set.count == LevelSet::most)
            fail("more than 100 levels", header);
        if (time < 0 || time > 999)
            fail("time must be 0-999", header);
        // Each is one byte of the record, theme and flag added to '0'.
        const auto byte = [](int value) { return value >= 0 && value <= 255; };
        if (!byte(theme + '0') || !byte(flag + '0') || !byte(palette) || !byte(background))
            fail("theme, palette, background or flag out of range", header);
        char word[16] = "";
        int after = 0;
        uint8_t code[3] = {};
        const bool password =
            std::sscanf(line.c_str() + end, " password %15s %n", word, &after) == 1;
        if (password ? line[end + after] != 0
                     : line.find_first_not_of(" \t", std::size_t(end)) != std::string::npos ||
                           !pack)
            fail("a password after the flag", header);
        if (password && !letters.code(word, code))
            fail(std::string("no password spells ") + word, header);
        set.passwords.insert(set.passwords.end(), code, code + 3);
        has_password.push_back(password);
        char digits[8];
        std::snprintf(digits, sizeof digits, "%03d", time);
        set.records.insert(set.records.end(), digits, digits + 3);
        for (int value : {theme + '0', palette, background, flag + '0'})
            set.records.push_back(uint8_t(value));
        for (unsigned row = 0; row < 11; ++row) {
            if (!next_line())
                fail("level " + std::to_string(level) + " has " + std::to_string(row) +
                         " of its 11 rows",
                     header);
            if (line.size() != 20)
                fail("a row must have 20 tiles", number);
            set.records.insert(set.records.end(), line.begin(), line.end());
        }
        ++set.count;
    }
    if (!pack && set.count != LevelSet::most)
        fail("levels.txt must hold 100 levels", 1);
    if (!set.count)
        fail("no levels", 1);
    pad_levels(set); // the passwords of the levels past count come after
    // The levels without a password: one made from the CRC and the level,
    // spelt unlike any other of the set.
    std::set<std::string> taken;
    for (unsigned n = 0; n < set.count; ++n)
        if (has_password[n])
            taken.insert(password_letters(set, n));
    uint32_t seed = set.crc();
    for (unsigned n = 0; n < set.count; ++n)
        if (!has_password[n])
            make_password(set, n, seed, taken);
    pad_levels(set);
    return set;
}

void pad_levels(LevelSet& set) {
    const unsigned size = LevelSet::record_size;
    set.records.resize(set.count * size);
    set.passwords.resize(set.count * 3);
    for (unsigned n = set.count; n < LevelSet::most; ++n) {
        set.records.insert(set.records.end(), set.records.end() - size, set.records.end());
        set.passwords.insert(set.passwords.end(), set.passwords.end() - 3, set.passwords.end());
    }
}

void new_password(LevelSet& set, unsigned level) {
    std::set<std::string> taken;
    for (unsigned n = 0; n < set.count; ++n)
        if (n != level)
            taken.insert(password_letters(set, n));
    if (set.passwords.size() < 3 * (level + 1))
        set.passwords.resize(3 * (level + 1));
    uint32_t seed = set.crc() + 7919u * level;
    make_password(set, level, seed, taken);
}

std::string write_levels(const LevelSet& set) {
    std::ostringstream text;
    text << levels_help;
    if (!set.name.empty())
        text << "\nname " << set.name << "\n";
    for (unsigned n = 0; n < set.count; ++n) {
        const uint8_t* r = set.records.data() + n * LevelSet::record_size;
        text << "\nlevel " << n + 1 << " time " << std::string(r, r + 3) << " theme "
             << int(r[3]) - '0' << " palette " << int(r[4]) << " background " << int(r[5])
             << " flag " << int(r[6]) - '0' << " password " << password_letters(set, n) << "\n";
        for (unsigned row = 0; row < 11; ++row)
            text << std::string(r + 7 + row * 20, r + 27 + row * 20) << "\n";
    }
    return text.str();
}

std::string password_letters(const LevelSet& set, unsigned level) {
    return alphabet().letters(&set.passwords.at(3 * level));
}
} // namespace stone
