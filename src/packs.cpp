// The list of level sets (ENHANCEMENTS.md, E7). Not original: L in the map
// selector, enhanced mode only. The host offers the built-in sets (the free
// maps, and the original's when its files are there) and the packs in
// levels/ (Machine::level_sets); each is listed with its maps and how many of
// them have been reached and cleared, and a pack that does not read with its
// first error. Enter plays the set from now on: it is saved as `pack` in
// STONEAGE.ENH and the host hands out its levels, passwords and score table.
#include "game.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <vector>

namespace stone {
namespace {
// Scan codes.
const uint8_t esc = 0x01, enter = 0x1c, key_l = 0x26, space = 0x39, home = 0x47, arrow_up = 0x48,
              page_up = 0x49, end_key = 0x4f, arrow_down = 0x50, page_down = 0x51;
// The list's own palette, as the selector's (the menu screens use 0-63 only).
const uint8_t text_slot = 1, dim_slot = 2, mark_slot = 3, bar_slot = 4, problem_slot = 5;
// Rows of the list, 18 pixels apart from y=44; columns of the 8x16 font.
const unsigned rows = 6, first_row = 44, row_height = 18;
const unsigned name_column = 1, name_width = 17, maps_column = 19, reached_column = 24,
               cleared_column = 32;
const unsigned help_y = 178, problem_y = 160;

unsigned row_y(unsigned row) { return first_row + row_height * row; }

std::string upper(std::string text) {
    for (char& c : text)
        c = char(std::toupper(static_cast<unsigned char>(c)));
    return text;
}
// `text` right-aligned so that it ends before column `end`.
unsigned right_column(const std::string& text, unsigned end) {
    return end - unsigned(std::min<std::size_t>(text.size(), end));
}

class PackList {
public:
    explicit PackList(const Game& game) : g(game), screen(game.screen) {}
    bool run();

private:
    struct Entry {
        Machine::LevelSetEntry set;
        unsigned reached = 0, cleared = 0;
    };
    void gather();
    void fade(bool in);
    void draw();
    void write(unsigned column, unsigned y, const std::string& text, uint8_t colour) const;
    std::vector<Machine::Line> lines() const;
    // The cursor to `to`, the list scrolled so that it shows.
    void move(int to);

    const Game& g;
    const Screen& screen;
    std::vector<Entry> entries;
    std::array<uint8_t, 768> palette{};
    unsigned cursor = 0, top = 0;
};

bool PackList::run() {
    screen.set_layout(false);
    gather();
    if (entries.empty())
        return false;
    draw();
    LineCursor taps(g, Machine::packs, lines());
    fade(true);
    g.read_scan();
    for (;;) {
        screen.wait(1);
        g.music.tick();
        uint8_t key = g.read_scan();
        const int tapped = g.m.tapped;
        g.m.tapped = -1;
        int to = int(cursor);
        if (tapped >= 0) { // a tap on a row: Enter on the cursor, else the cursor there
            to = int(top) + tapped;
            key = unsigned(to) == cursor ? enter : 0;
        } else if (!g.scan_change() || key >= 0x80)
            continue;
        switch (key) {
        case 0: break;
        case arrow_up: to -= 1; break;
        case arrow_down: to += 1; break;
        case page_up: to -= int(rows); break;
        case page_down: to += int(rows); break;
        case home: to = 0; break;
        case end_key: to = int(entries.size()) - 1; break;
        case enter:
        case space: {
            const Entry& entry = entries[cursor];
            if (!entry.set.problem.empty())
                continue; // a pack that does not read cannot be chosen
            taps.leave();
            fade(false);
            if (entry.set.pack == g.m.pack)
                return false;
            Settings settings = g.load_settings();
            settings.set_pack(entry.set.pack);
            g.save_settings(settings);
            if (g.m.change_assets)
                g.m.change_assets(g.m);
            // Its own score table, or the built-in one the new program
            // bytes hold when it has none yet.
            g.load_scores();
            return true;
        }
        case esc:
        case key_l: taps.leave(); fade(false); return false;
        default: continue;
        }
        const unsigned shown = top;
        move(to);
        if (top != shown)
            taps.replace(lines());
        draw();
    }
}

void PackList::gather() {
    if (!g.m.level_sets)
        return;
    const Settings settings = g.load_settings();
    for (const Machine::LevelSetEntry& set : g.m.level_sets()) {
        Entry entry;
        entry.set = set;
        if (set.problem.empty()) {
            // reached and last are written together, when a level starts.
            if (settings.last(set.crc))
                entry.reached = std::min(settings.reached(set.crc), set.levels);
            for (unsigned level = 1; level <= set.levels; ++level)
                entry.cleared += settings.best(set.crc, level).cleared();
        }
        if (set.pack == g.m.pack)
            cursor = unsigned(entries.size());
        entries.push_back(entry);
    }
    move(int(cursor));
    const auto set = [&](unsigned slot, uint8_t red, uint8_t green, uint8_t blue) {
        palette[3 * slot] = red;
        palette[3 * slot + 1] = green;
        palette[3 * slot + 2] = blue;
    };
    set(text_slot, 63, 56, 36); // as the selector's
    set(dim_slot, 36, 34, 30);
    set(mark_slot, 63, 50, 10);
    set(bar_slot, 14, 12, 9);
    set(problem_slot, 63, 24, 18);
    set(255, 63, 63, 63);
}

void PackList::move(int to) {
    cursor = unsigned(std::min(std::max(to, 0), int(entries.size()) - 1));
    if (cursor < top)
        top = cursor;
    else if (cursor >= top + rows)
        top = cursor - rows + 1;
}

// As the selector's: all 256 colours.
void PackList::fade(bool in) {
    std::array<uint8_t, 768> shown;
    for (unsigned step = 0; step <= 64; ++step) {
        const unsigned level = in ? 4 * step : 256 - 4 * step;
        for (unsigned n = 0; n < 768; ++n)
            shown[n] = uint8_t(palette[n] * level >> 8);
        screen.wait(1);
        screen.set_colours(shown.data(), 0, 255);
    }
}

void PackList::draw() {
    std::fill_n(screen.work_buffer(), 320 * 200, uint8_t(0));
    screen.show_picture();
    screen.fill(0, row_y(cursor - top) - 1, 320, row_height, bar_slot);
    g.unpack(g.font_resource(), g.work_buffer());
    const uint8_t colour = g.text_colour();
    write(1, 2, "LEVEL SETS", text_slot);
    const std::string place = std::to_string(cursor + 1) + " OF " + std::to_string(entries.size());
    write(right_column(place, 39), 2, place, dim_slot);
    write(name_column, 24, "NAME", dim_slot);
    write(maps_column, 24, "MAPS", dim_slot);
    write(reached_column, 24, "REACHED", dim_slot);
    write(cleared_column, 24, "CLEARED", dim_slot);
    for (unsigned row = 0; row < rows && top + row < entries.size(); ++row) {
        const Entry& entry = entries[top + row];
        const unsigned y = row_y(row);
        const bool broken = !entry.set.problem.empty();
        // The set played now in the colour of a cleared map's mark.
        const uint8_t name_colour = broken                       ? dim_slot
                                    : entry.set.pack == g.m.pack ? mark_slot
                                                                 : text_slot;
        write(name_column, y, upper(entry.set.name).substr(0, name_width), name_colour);
        if (broken) {
            write(maps_column, y, "DOES NOT READ", problem_slot);
            continue;
        }
        const std::string maps = std::to_string(entry.set.levels),
                          reached = std::to_string(entry.reached),
                          cleared = std::to_string(entry.cleared);
        write(right_column(maps, maps_column + 4), y, maps, text_slot);
        write(right_column(reached, reached_column + 7), y, reached, text_slot);
        write(right_column(cleared, cleared_column + 7), y, cleared, text_slot);
    }
    // Under the list: what is wrong with the pack under the cursor, in two
    // lines of 40 characters, or the keys.
    const std::string problem = upper(entries[cursor].set.problem);
    if (!problem.empty()) {
        const bool two = problem.size() > 40;
        write(0, two ? problem_y : help_y, problem.substr(0, 40), problem_slot);
        if (two)
            write(0, help_y, problem.substr(40, 40), problem_slot);
    } else
        write(1, help_y, "ENTER PLAY  ESC BACK", dim_slot);
    g.text_colour() = colour;
    screen.flip();
}

void PackList::write(unsigned column, unsigned y, const std::string& text, uint8_t colour) const {
    g.text_colour() = colour;
    screen.move_to(8 * column, y);
    for (const char character : text)
        screen.write(uint8_t(character));
}

// Where a tap acts (E6): the rows shown, by their place on the screen, and
// ESC BACK under them.
std::vector<Machine::Line> PackList::lines() const {
    std::vector<Machine::Line> found;
    for (unsigned row = 0; row < rows && top + row < entries.size(); ++row)
        found.push_back({0, row_y(row) - 1, 320, row_height, 0});
    found.push_back({104, help_y, 64, 16, esc}); // ESC BACK, from column 13
    return found;
}
} // namespace

bool Game::choose_pack() const { return PackList(*this).run(); }
} // namespace stone
