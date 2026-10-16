// The lines of the screens driven by function keys (ENHANCEMENTS.md, E6).
// Not original. A gamepad has no function keys and a phone no keyboard, so
// these screens tell the host where their lines are: a tap on one sends its
// key, in both modes. In enhanced mode they also get a cursor, a marker that
// the arrow keys move from line to line and Enter or Space act on.
#include "game.hpp"
#include <algorithm>
#include <cstring>
#include <string>

namespace stone {
namespace {
using namespace scan;
const uint8_t marker_slot = 253;
} // namespace

LineCursor::LineCursor(const Game& game, Machine::Showing showing,
                       std::vector<Machine::Line> lines, unsigned line)
    : g(game), showing_before(game.m.showing), lines_before(game.m.lines) {
    g.m.showing = showing;
    replace(std::move(lines), line);
    hide();
}

void LineCursor::replace(std::vector<Machine::Line> lines, unsigned line) {
    g.m.lines = std::move(lines);
    g.m.tapped = -1;
    at = line < g.m.lines.size() ? line : 0;
}

bool LineCursor::active() const {
    return g.m.enhanced && !left && !g.m.lines.empty() && g.m.lines[0].key;
}

uint8_t LineCursor::key(uint8_t scan) {
    was_moved = false;
    if (!active())
        return scan;
    const unsigned count = unsigned(g.m.lines.size());
    if (scan == arrow_up || scan == arrow_down) {
        at = (at + (scan == arrow_up ? count - 1 : 1)) % count;
        was_moved = true;
        ticks = 0; // bright at once where it now is
        return 0;
    }
    if (scan == enter || scan == space)
        return g.m.lines[at].key;
    for (unsigned n = 0; n < count; ++n)
        if (g.m.lines[n].key == scan)
            at = n;
    return scan;
}

// A triangle pointing right, seven pixels high, ending four pixels before
// the line.
void LineCursor::draw() const {
    if (!active())
        return;
    const Machine::Line& line = g.m.lines[at];
    const unsigned middle = line.y + line.height / 2;
    for (unsigned column = 0; column < 4; ++column)
        g.screen.fill(line.x - 8 + column, middle - 3 + column, 1, 7 - 2 * column, marker_slot);
}

void LineCursor::pulse() {
    if (!active() || ticks++ % 16)
        return;
    const uint8_t level = ticks / 16 % 2 ? 40 : 63;
    uint8_t palette[3 * (marker_slot + 1)] = {};
    std::fill_n(palette + 3 * marker_slot, 3, level);
    g.screen.set_colours(palette, marker_slot, marker_slot);
}

void LineCursor::hide() {
    ticks = 0;
    if (!active())
        return;
    const uint8_t palette[3 * (marker_slot + 1)] = {};
    g.screen.set_colours(palette, marker_slot, marker_slot);
}

void LineCursor::leave() {
    if (left)
        return;
    hide();
    left = true;
    g.m.showing = showing_before;
    g.m.lines = lines_before;
    g.m.tapped = -1;
}

uint8_t Game::wait_for_line_key(LineCursor& cursor, const std::function<void()>& redraw) const {
    if (!m.enhanced)
        return wait_for_next_key();
    for (;;) {
        screen.wait(1);
        music.tick();
        cursor.pulse();
        read_scan();
        if (!scan_change())
            continue;
        uint8_t key = scan();
        if (key < 0x80) {
            key = cursor.key(key);
            if (cursor.moved()) {
                redraw();
                continue;
            }
        }
        return key;
    }
}

// The menu's lines are part of its picture, ten pixels apart. On the free
// one they start at (107,65) (ASSETS.md); the original's step right seven
// pixels a line from (73,67), as add_stats_line measured them, and with its
// F8 and F9 lines in enhanced mode they start at y=59, 8 pixels apart
// (add_editor_line). With enhanced mode off F8 and F9 do nothing and have
// no line.
std::vector<Machine::Line> Game::menu_lines() const {
    const bool original = m.original_assets && load_settings().original(Settings::images);
    std::vector<Machine::Line> lines;
    for (unsigned n = 0; n < (m.enhanced ? 9u : 7u); ++n)
        lines.push_back(original ? Machine::Line{73 + 7 * n, m.enhanced ? 59 + 8 * n : 67 + 10 * n,
                                                 112, 7, uint8_t(f1 + n)}
                                 : Machine::Line{107, 65 + 10 * n, 96, 7, uint8_t(f1 + n)});
    return lines;
}

// draw_option_lines writes the lines 17 pixels apart from y=64, their
// letters from the third row of the font to the ninth; the F10 line is the
// picture's, at y=156 on the free one.
std::vector<Machine::Line> Game::option_lines(unsigned count) const {
    std::vector<Machine::Line> lines;
    for (unsigned n = 0; n < count; ++n)
        lines.push_back({91, 65 + 17 * n, 124, 12, uint8_t(f1 + n)});
    lines.push_back({91, 153, 124, 12, f10});
    return lines;
}

// The jukebox writes its texts itself (jukebox()): the songs of F1-F8 a line
// of 8 pixels each from (72,61), the help from (32,168), each of its lines
// starting with the name of its key.
std::vector<Machine::Line> Game::jukebox_lines() const {
    std::vector<Machine::Line> lines;
    const auto each_line = [](const uint8_t* text, const std::function<void(const std::string&, unsigned)>& line) {
        std::string current;
        unsigned number = 0;
        for (;; ++text) {
            if (*text == '\r' || !*text) {
                line(current, number++);
                current.clear();
                if (!*text)
                    return;
            } else
                current += char(*text);
        }
    };
    each_line(jukebox_song_list(), [&](const std::string& text, unsigned n) {
        if (n < 8)
            lines.push_back({72, 61 + 8 * n, 8 * unsigned(text.size()), 8, uint8_t(f1 + n)});
    });
    each_line(jukebox_help(), [&](const std::string& text, unsigned n) {
        const std::string name = text.substr(0, text.find_first_of(" -"));
        const struct {
            const char* name;
            uint8_t key;
        } keys[] = {{"F9", f9}, {"F10", f10}, {"DEL", del}, {"BACKSPACE", backspace}, {"ESC", esc}};
        for (const auto& key : keys)
            if (name == key.name)
                lines.push_back({32, 168 + 8 * n, 8 * unsigned(text.size()), 8, key.key});
    });
    return lines;
}
} // namespace stone
