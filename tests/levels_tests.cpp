// Tests of level sets as text (levels.hpp; ENHANCEMENTS.md, E7): packs of
// 1, 37 and 100 levels, names and passwords, broken files and writing.
#include "levels.hpp"
#include <iostream>
#include <set>
#include <string>

namespace {
unsigned failures = 0;

void check(bool ok, const char* what, int line) {
    if (!ok) {
        std::cerr << "levels_tests.cpp:" << line << ": failed: " << what << "\n";
        ++failures;
    }
}
#define CHECK(condition) check(condition, #condition, __LINE__)

stone::Bytes bytes(const std::string& text) { return stone::Bytes(text.begin(), text.end()); }

// A level: walls round a start and an exit, `tile` in the middle.
std::string level(unsigned number, const std::string& password = "", char tile = 'B') {
    std::string text = "level " + std::to_string(number) + " time 050 theme " +
                       std::to_string(number % 5) + " palette 2 background 3 flag 0";
    if (!password.empty())
        text += " password " + password;
    text += "\nLLLLLLLLLLLLLLLLLLLL\n";
    for (unsigned row = 1; row < 10; ++row)
        text += row == 5 ? std::string("LRBBBBBBB") + tile + "BBBBBBBBSL\n"
                         : "LBBBBBBBBBBBBBBBBBBL\n";
    return text + "LLLLLLLLLLLLLLLLLLLL\n";
}
std::string levels(unsigned count) {
    std::string text;
    for (unsigned n = 1; n <= count; ++n)
        text += level(n);
    return text;
}
// The error line of a file that does not read, or 0.
unsigned error_line(const std::string& text, bool pack = true) {
    try {
        stone::read_levels(bytes(text), "pack.txt", pack);
        return 0;
    } catch (const stone::LevelError& e) {
        return e.line;
    }
}

void packs() {
    for (const unsigned count : {1u, 37u, 100u}) {
        const stone::LevelSet set = stone::read_levels(bytes(levels(count)), "pack.txt", true);
        CHECK(set.count == count);
        CHECK(set.name.empty());
        CHECK(set.records.size() == 100 * 227);
        CHECK(set.passwords.size() == 300);
        // Each level has a password of its own, made from the CRC.
        std::set<std::string> passwords;
        for (unsigned n = 0; n < count; ++n)
            passwords.insert(stone::password_letters(set, n));
        CHECK(passwords.size() == count);
        // Padded with the last level.
        CHECK(std::equal(set.records.end() - 227, set.records.end(),
                         set.records.begin() + (count - 1) * 227));
    }
    const stone::LevelSet a = stone::read_levels(bytes(levels(37)), "a.txt", true),
                          b = stone::read_levels(bytes(levels(37)), "b.txt", true);
    CHECK(a.crc() == b.crc());
    CHECK(stone::password_letters(a, 3) == stone::password_letters(b, 3));
}

void record() {
    const stone::LevelSet set = stone::read_levels(bytes(level(1, "WEFARO", 'J')), "one.txt", true);
    CHECK(stone::password_letters(set, 0) == "WEFARO");
    const uint8_t* r = set.records.data();
    CHECK(std::string(r, r + 3) == "050");
    CHECK(r[3] == '1' && r[4] == 2 && r[5] == 3 && r[6] == '0');
    CHECK(r[7 + 5 * 20 + 9] == 'J');
}

void names() {
    const stone::LevelSet set =
        stone::read_levels(bytes("# a pack\nname Cave Tour\n\n" + levels(2)), "cave.txt", true);
    CHECK(set.name == "Cave Tour");
    CHECK(set.count == 2);
    CHECK(error_line(level(1) + "name Late\n" + level(2)) == 13);
    // levels.txt of an asset set has no name.
    CHECK(error_line("name X\n" + levels(100), false) != 0);
}

void errors() {
    CHECK(error_line("") == 1);                     // no levels
    CHECK(error_line("# only words\n") == 1);
    CHECK(error_line(level(2)) == 1);               // not level 1
    CHECK(error_line(level(1) + level(3)) == 13);   // not level 2
    CHECK(error_line(levels(101)) == 1201);         // too many
    CHECK(error_line(level(1, "ABCDEF")) == 1);     // no such password
    std::string text = levels(3);
    text.replace(text.find("LRBBB", text.find("level 2")), 5, "LRBB");
    CHECK(error_line(text) == 19);                  // a short row
    text = levels(2);
    text.erase(text.rfind("\nLLLL") + 1);
    CHECK(error_line(text) == 13);                  // a missing row: the level's header
    CHECK(error_line("level 1 time 1000 theme 0 palette 0 background 0 flag 0\n") == 1);
    CHECK(error_line(level(1) + "rubbish\n") == 13);
    // levels.txt of an asset set: 100 levels, each with a password.
    CHECK(error_line(levels(37), false) == 1);
    CHECK(error_line(levels(100), false) == 1);
}

void written() {
    const stone::LevelSet set =
        stone::read_levels(bytes("name Round Trip\n" + levels(37)), "trip.txt", true);
    const std::string text = stone::write_levels(set);
    const stone::LevelSet again = stone::read_levels(bytes(text), "trip.txt", true);
    CHECK(again.name == set.name);
    CHECK(again.count == 37);
    CHECK(again.records == set.records);
    CHECK(again.passwords == set.passwords);
    CHECK(stone::write_levels(again) == text);
}
} // namespace

unsigned levels_tests() {
    packs();
    record();
    names();
    errors();
    written();
    if (!failures)
        std::cout << "levels_tests passed\n";
    return failures;
}
