// Tests of STONEAGE.ENH (settings.hpp) that need nothing of the original game.
#include "settings.hpp"
#include <iostream>
#include <string>

namespace {
unsigned failures = 0;

void check(bool ok, const char* what, int line) {
    if (!ok) {
        std::cerr << "settings_tests.cpp:" << line << ": failed: " << what << "\n";
        ++failures;
    }
}
#define CHECK(condition) check(condition, #condition, __LINE__)

stone::Bytes bytes(const std::string& text) { return stone::Bytes(text.begin(), text.end()); }
std::string text(const stone::Bytes& bytes) { return std::string(bytes.begin(), bytes.end()); }

const uint32_t set_a = 0x12345678, set_b = 0xdeadbeef;

void defaults() {
    const stone::Settings settings;
    CHECK(settings.enhanced());
    CHECK(settings.original(stone::Settings::maps));
    CHECK(settings.reached(set_a) == 1);
    CHECK(settings.last(set_a) == 0);
    CHECK(settings.bytes().empty());
}

void reading() {
    const stone::Settings settings(bytes("enhanced 0\r\nmaps free\nreached 12345678 40\n"
                                         "last 12345678 7\nlast deadbeef 101\nreached deadbeef x"));
    CHECK(!settings.enhanced());
    CHECK(!settings.original(stone::Settings::maps));
    CHECK(settings.original(stone::Settings::music));
    CHECK(settings.reached(set_a) == 40);
    CHECK(settings.last(set_a) == 7);
    CHECK(settings.last(set_b) == 0);    // out of range
    CHECK(settings.reached(set_b) == 1); // not a number
}

// E2: each level set has its own last map, apart from the furthest.
void last_map() {
    stone::Settings settings;
    settings.set_reached(set_a, 30);
    settings.set_last(set_a, 12);
    settings.set_last(set_b, 3);
    settings.set_last(set_a, 14);
    const stone::Settings saved(settings.bytes());
    CHECK(saved.last(set_a) == 14);
    CHECK(saved.last(set_b) == 3);
    CHECK(saved.reached(set_a) == 30);
    CHECK(saved.reached(set_b) == 1);
    CHECK(text(settings.bytes()) == "reached 12345678 30\nlast 12345678 14\nlast deadbeef 3\n");
}

// E3: the fewest ticks and the most points, kept apart and only improved.
void bests() {
    using stone::Settings;
    Settings settings;
    CHECK(!settings.best(set_a, 12).cleared());
    CHECK(settings.improve_best(set_a, 12, 2891, 1250) == (Settings::faster | Settings::higher));
    CHECK(settings.improve_best(set_a, 12, 3000, 1300) == Settings::higher);
    CHECK(settings.improve_best(set_a, 12, 2500, 900) == Settings::faster);
    CHECK(settings.improve_best(set_a, 12, 2500, 1300) == 0);
    CHECK(settings.improve_best(set_a, 1, 700, 0) == (Settings::faster | Settings::higher));
    CHECK(settings.improve_best(set_b, 12, 4000, 100) != 0);
    const Settings saved(settings.bytes());
    CHECK(saved.best(set_a, 12).ticks == 2500);
    CHECK(saved.best(set_a, 12).points == 1300);
    CHECK(saved.best(set_a, 1).cleared());
    CHECK(saved.best(set_a, 1).points == 0);
    CHECK(saved.best(set_b, 12).ticks == 4000);
    CHECK(!saved.best(set_b, 1).cleared());
    CHECK(text(settings.bytes()) ==
          "best 12345678 12 2500 1300\nbest 12345678 1 700 0\nbest deadbeef 12 4000 100\n");
}

void bad_bests() {
    const stone::Settings settings(bytes("best 12345678 1 x 5\nbest 12345678 2 50\n"
                                         "best 12345678 3 0 5\nbest 12345678 4 60 7 8\n"));
    for (unsigned level = 1; level <= 4; ++level)
        CHECK(!settings.best(set_a, level).cleared());
}

// E4: a line per map of each level set, and one of totals.
void statistics() {
    using stone::Settings;
    Settings settings;
    CHECK(settings.map_stats(set_a, 5).attempts == 0);
    CHECK(settings.totals().games == 0);
    Settings::MapStats stats;
    stats.attempts = 4;
    stats.clears = 1;
    stats.lives_lost = 2;
    stats.give_ups = 1;
    stats.ticks = 12345678901ull;
    settings.set_map_stats(set_a, 5, stats);
    stats.attempts = 1;
    settings.set_map_stats(set_b, 5, stats);
    Settings::Totals totals;
    totals.ticks = 70;
    totals.games = 2;
    totals.platforms = 9;
    settings.set_totals(totals);
    totals.games = 3;
    settings.set_totals(totals);
    const Settings saved(settings.bytes());
    const Settings::MapStats read = saved.map_stats(set_a, 5);
    CHECK(read.attempts == 4);
    CHECK(read.attempts == read.clears + read.lives_lost + read.give_ups);
    CHECK(read.ticks == 12345678901ull);
    CHECK(saved.map_stats(set_b, 5).attempts == 1);
    CHECK(saved.map_stats(set_a, 6).attempts == 0);
    CHECK(saved.totals().games == 3);
    CHECK(saved.totals().platforms == 9);
    CHECK(text(settings.bytes()) == "stats 12345678 5 4 1 2 1 12345678901\n"
                                    "stats deadbeef 5 1 1 2 1 12345678901\n"
                                    "totals 70 3 0 0 0 0 0 0 9\n");
}

void bad_statistics() {
    const stone::Settings settings(bytes("stats 12345678 1 1 1 0 0\nstats 12345678 2 1 1 0 0 x\n"
                                         "stats 12345678 3 1 1 0 0 5 6\nstats 12345678 4 1  1 0 0 5\n"
                                         "totals 1 2 3\n"));
    for (unsigned level = 1; level <= 4; ++level)
        CHECK(settings.map_stats(set_a, level).attempts == 0);
    CHECK(settings.totals().ticks == 0);
}

void unknown_lines_kept() {
    stone::Settings settings(bytes("future 1 2 3\nenhanced 1\n# note\n"));
    settings.set_enhanced(false);
    settings.set_last(set_a, 5);
    CHECK(text(settings.bytes()) == "future 1 2 3\nenhanced 0\n# note\nlast 12345678 5\n");
}
} // namespace

int main() {
    defaults();
    reading();
    last_map();
    bests();
    bad_bests();
    statistics();
    bad_statistics();
    unknown_lines_kept();
    if (failures)
        return 1;
    std::cout << "settings_tests passed\n";
    return 0;
}
