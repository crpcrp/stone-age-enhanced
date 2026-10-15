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
    unknown_lines_kept();
    if (failures)
        return 1;
    std::cout << "settings_tests passed\n";
    return 0;
}
