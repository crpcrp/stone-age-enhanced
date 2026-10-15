// Tests of what the level editor does to a level (level_edit.hpp;
// ENHANCEMENTS.md, E9): the empty slot, placing tiles and the rock, the
// checks, passwords for new levels and a pack written and read back.
#include "level_edit.hpp"
#include "solver.hpp"
#include <iostream>
#include <set>
#include <string>

namespace {
unsigned failures = 0;

void check(bool ok, const char* what, int line) {
    if (!ok) {
        std::cerr << "level_edit_tests.cpp:" << line << ": failed: " << what << "\n";
        ++failures;
    }
}
#define CHECK(condition) check(condition, #condition, __LINE__)

using stone::LevelRecord;

unsigned count(const LevelRecord& level, uint8_t tile) {
    unsigned found = 0;
    for (unsigned row = 0; row < LevelRecord::rows; ++row)
        for (unsigned column = 0; column < LevelRecord::columns; ++column)
            found += level.at(column, row) == tile;
    return found;
}

void empty_slot() {
    const LevelRecord level = LevelRecord::empty();
    CHECK(level.time() == 60);
    CHECK(level.theme() == 0 && level.palette() == 0 && level.background() == 0);
    CHECK(level.at(0, 0) == 'L' && level.at(19, 10) == 'L' && level.at(1, 1) == 'B');
    CHECK(count(level, 'R') == 1 && count(level, 'S') == 1);
    CHECK(level.check().errors.empty() && level.check().warnings.empty());
    // It can be played: the solver walks from the start to the exit.
    const stone::Solution solution = stone::solve(level.bytes.data());
    CHECK(solution.verdict == stone::Solution::solvable);
    CHECK(solution.ticks == 15 * 16 + 1); // fifteen steps, and the tick on the exit
}

void placing() {
    LevelRecord level = LevelRecord::empty();
    CHECK(level.place(5, 5, 'J'));
    CHECK(level.at(5, 5) == 'J');
    CHECK(!level.place(5, 5, 'J')); // nothing changes
    CHECK(!level.place(20, 0, 'J'));
    // One start: it moves.
    CHECK(level.place(9, 3, 'R'));
    CHECK(count(level, 'R') == 1 && level.at(9, 3) == 'R' && level.at(2, 5) == 'B');
    // The rock: three by three from the cell, inside the board only.
    CHECK(!level.place(18, 2, LevelRecord::rock));
    CHECK(!level.place(4, 9, LevelRecord::rock));
    CHECK(level.place(4, 1, LevelRecord::rock));
    CHECK(level.at(4, 1) == 'b' && level.at(5, 2) == 'f' && level.at(6, 3) == 'j');
    // A cell of it takes the whole rock with it.
    CHECK(level.place(5, 3, 'K'));
    CHECK(level.at(5, 3) == 'K' && level.at(4, 1) == 'A' && level.at(6, 3) == 'A');
    CHECK(count(level, 'f') == 0);
    // A rock over part of another replaces it.
    CHECK(level.place(4, 1, LevelRecord::rock));
    CHECK(level.place(6, 2, LevelRecord::rock));
    CHECK(level.at(4, 1) == 'A' && level.at(6, 2) == 'b' && count(level, 'b') == 1);
    CHECK(level.clear(7, 3));
    CHECK(count(level, 'e') == 0 && level.at(7, 3) == 'A');
    // k-m are not among the tiles placed.
    for (const uint8_t tile : LevelRecord::tiles())
        CHECK(tile < 'k' || tile > 'm');
}

void settings() {
    LevelRecord level = LevelRecord::empty();
    level.set_time(999);
    CHECK(level.time() == 999);
    level.set_time(1500);
    CHECK(level.time() == 999);
    level.set_time(5);
    CHECK(level.time() == 5 && level.bytes[0] == '0' && level.bytes[2] == '5');
    level.set_theme(4);
    level.set_palette(5);
    level.set_background(3);
    CHECK(level.theme() == 4 && level.palette() == 5 && level.background() == 3);
}

void checks() {
    LevelRecord level = LevelRecord::empty();
    level.set(2, 5, 'B');
    CHECK(level.check().errors.size() == 1);
    level.set(2, 5, 'R');
    level.set(3, 5, 'R');
    CHECK(level.check().errors.size() == 1 && level.check().errors[0] == "2 STARTS");
    level.set(3, 5, 'B');
    level.set(17, 5, 'B');
    CHECK(level.check().errors.size() == 1 && level.check().errors[0] == "NO EXIT");
    level.set(17, 5, 'S');
    level.set(5, 5, 'O');
    CHECK(level.check().warnings.size() == 1);
    level.set(6, 5, 'M');
    level.set(7, 5, 'N');
    CHECK(level.check().warnings.size() == 1 &&
          level.check().warnings[0] == "GREEN GATE, NO KEY");
    level.set(7, 5, 'B');
    level.set(9, 9, 'f'); // a piece of a rock alone
    CHECK(level.check().errors.empty() && level.check().warnings.size() == 1);
}

void packs() {
    // A pack built as the editor builds it, and read back the same.
    stone::LevelSet set;
    set.name = "Built Here";
    for (unsigned n = 0; n < 3; ++n) {
        LevelRecord level = LevelRecord::empty();
        level.place(4 + n, 4, 'J');
        set.records.resize((n + 1) * stone::LevelSet::record_size);
        std::copy(level.bytes.begin(), level.bytes.end(),
                  set.records.end() - stone::LevelSet::record_size);
        set.count = n + 1;
        stone::pad_levels(set);
        stone::new_password(set, n);
        stone::pad_levels(set);
    }
    CHECK(set.records.size() == 100 * 227 && set.passwords.size() == 300);
    std::set<std::string> passwords;
    for (unsigned n = 0; n < 3; ++n)
        passwords.insert(stone::password_letters(set, n));
    CHECK(passwords.size() == 3);
    const std::string text = stone::write_levels(set);
    const stone::LevelSet again =
        stone::read_levels(stone::Bytes(text.begin(), text.end()), "built.txt", true);
    CHECK(again.name == set.name && again.count == 3);
    CHECK(again.records == set.records);
    CHECK(again.passwords == set.passwords);
    CHECK(stone::pack_file_name("Cave Tour 2") == "Cave_Tour_2.txt");
    CHECK(stone::pack_file_name("a/b.c") == "a_b_c.txt");
}
} // namespace

unsigned level_edit_tests() {
    empty_slot();
    placing();
    settings();
    checks();
    packs();
    if (!failures)
        std::cout << "level_edit_tests passed\n";
    return failures;
}
