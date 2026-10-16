// The level editor (ENHANCEMENTS.md, E9). Not original: enhanced mode only,
// F9 in the menu. A map of the level set played, or an empty slot after a
// pack's last map, is chosen in the map selector; N there starts a new pack.
// The board is shown as in a level, with a cursor, and the strip below it
// shows the tile in hand, the level's settings and the last check. A level
// is played from here as in a game, recording nothing, checked with the
// solver (E8) and saved as a pack in levels/ through E7's format, which then
// becomes the level set played. A map of a built-in set is saved as a copy,
// into a new pack.
#include "game.hpp"
#include "level_edit.hpp"
#include "solver.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

namespace stone {
namespace {
using namespace scan;
const uint8_t leave_key = f9; // the menu's ESC LEAVE: Esc itself closes the menu
// Colours: the cursor's own (the level uses 0-63, 254 and 255), and of the
// level's palette, which keeps these in every set (ASSETS.md): white, grey,
// green and red.
const uint8_t cursor_slot = 253, black = 254, white = 255, grey = 24, green = 27, red = 28;
// The tiles the number keys pick: 1 floor, 2 wall, 3 empty space, 4 start,
// 5 exit, 6 cracked floor, 7 platform of all four ways, 8 teleport, 9
// hourglass, 0 the outside.
const uint8_t common_tiles[10] = {'B', 'L', 'A', 'R', 'S', 'K', 'I', 'Q', 'J', 'a'};
const unsigned board_height = 176, strip_height = 22;
// The strip: the tiles before and after the one in hand, its settings from
// `fields_x`, the check from `result_x`, a message over both.
const unsigned previous_x = 4, hand_x = 24, next_x = 44, tile_y = 3, fields_x = 64,
               unsaved_x = 196, result_x = 208;
// Ticks an arrow is held before it repeats, and between repeats.
const unsigned first_repeat = 18, repeat_every = 4;
const std::size_t undo_depth = 500;

class Editor {
public:
    // `set` is the level set the map belongs to, saved as levels/`file`;
    // without a file (a built-in set) the map is saved as a copy, into a
    // new pack. A map at `set.count` is a new one, started empty.
    Editor(const Game& game, LevelSet set, unsigned map, std::string file)
        : g(game), screen(game.screen), set(std::move(set)), map(map), file(std::move(file)),
          level(map < this->set.count ? LevelRecord(&this->set.records[map * LevelSet::record_size])
                                      : LevelRecord::empty()),
          saved(level) {}
    void run();

private:
    void show();         // everything drawn anew, and faded in
    void draw_board();   // the board of `level` onto the pages, as a level does it
    void redraw();       // the board with the cursor, shown
    void draw_strip();
    void on_strip(const std::function<void()>& draw) const;
    void write(unsigned x, unsigned y, const std::string& text, uint8_t colour) const;
    void pulse();
    std::vector<Machine::Line> lines() const;
    // A key that went down: false when the editor is left.
    bool handle(uint8_t key);
    void move(int columns, int rows);
    void place(uint8_t tile);
    // The level changed to `next`, undoably: within a stroke of painting, all
    // of it is one change.
    void change(const LevelRecord& next);
    void apply(const LevelRecord& next);
    void undo(std::vector<LevelRecord>& from, std::vector<LevelRecord>& to);
    void cycle_tile(int by);
    void test();
    void check();
    void save();
    void help();
    // The editor's menu (L, a gamepad's Y): its actions on the line cursor
    // of E6. The key of the action chosen, or 0.
    uint8_t menu();
    // A name typed for a new pack; empty when given up.
    std::string enter_name();
    void message(const std::string& text, uint8_t colour) {
        note = text;
        note_colour = colour;
        draw_strip();
    }
    bool unsaved() const { return level != saved; }

    const Game& g;
    const Screen& screen;
    LevelSet set;
    unsigned map;
    std::string file;
    LevelRecord level, saved;
    std::vector<LevelRecord> undone, redone;
    unsigned column = 0, row = 0, hand = 1; // hand: in LevelRecord::tiles()
    bool shift = false, control = false, painting = false, stroke = false, leaving = false;
    unsigned held = 0, held_ticks = 0, ticks = 0;
    // The last check: of which level, its verdict and what the strip says.
    LevelRecord checked;
    bool has_check = false;
    Settings::Par par;
    std::string result = "NOT CHECKED", note;
    uint8_t result_colour = grey, note_colour = white;
};

void Editor::run() {
    // The cursor starts on the start.
    for (unsigned cell = 0; cell < LevelRecord::columns * LevelRecord::rows; ++cell)
        if (level.at(cell % LevelRecord::columns, cell / LevelRecord::columns) == 'R') {
            column = cell % LevelRecord::columns;
            row = cell / LevelRecord::columns;
        }
    LineCursor taps(g, Machine::editor, lines());
    show();
    g.read_scan();
    for (;;) {
        screen.wait(1);
        g.music.tick();
        pulse();
        uint8_t key = g.read_scan();
        const int tapped = g.m.tapped;
        g.m.tapped = -1;
        if (tapped >= 0) { // a cell: the cursor there, or on the cursor Enter
            const unsigned to_column = unsigned(tapped) % LevelRecord::columns,
                           to_row = unsigned(tapped) / LevelRecord::columns;
            if (to_column == column && to_row == row)
                key = enter;
            else {
                move(int(to_column) - int(column), int(to_row) - int(row));
                continue;
            }
        } else if (!g.scan_change()) {
            // An arrow held repeats.
            if (held && ++held_ticks >= first_repeat && (held_ticks - first_repeat) % repeat_every == 0)
                handle(uint8_t(held));
            continue;
        }
        switch (key) { // what is held
        case left_shift:
        case right_shift: shift = true; continue;
        case left_shift | released:
        case right_shift | released: shift = false; continue;
        case ctrl: control = true; continue;
        case ctrl | released: control = false; continue;
        case space | released:
        case enter | released: painting = stroke = false; continue;
        }
        held = key == arrow_up || key == arrow_down || key == arrow_left || key == arrow_right
                   ? key
                   : 0;
        held_ticks = 0;
        if (key >= released)
            continue;
        if (key == space || key == enter)
            painting = true;
        if (!handle(key))
            break;
    }
    taps.leave();
    screen.fade_out();
}

bool Editor::handle(uint8_t key) {
    const bool was_leaving = leaving;
    leaving = false;
    if (key == esc || key == leave_key) {
        if (unsaved() && !was_leaving) {
            leaving = true;
            message("NOT SAVED: ESC AGAIN LEAVES", red);
            return true;
        }
        return false;
    }
    if (!note.empty() && key != f3 && key != f10) {
        note.clear();
        draw_strip();
    }
    if (control && key == key_z)
        undo(undone, redone);
    else if (control && key == key_y)
        undo(redone, undone);
    else
        switch (key) {
        case arrow_up: move(0, -1); break;
        case arrow_down: move(0, 1); break;
        case arrow_left: move(-1, 0); break;
        case arrow_right: move(1, 0); break;
        case space:
        case enter: place(LevelRecord::tiles()[hand]); break;
        case del: place('A'); break;
        case tab: cycle_tile(shift ? -1 : 1); break;
        case page_up: cycle_tile(-1); break;
        case page_down: cycle_tile(1); break;
        case f1: help(); break;
        case f2: test(); break;
        case f3: check(); break;
        case f4:
        case f5:
        case f6:
        case f7: {
            LevelRecord next = level;
            const int by = shift ? -1 : 1;
            const auto step = [&](unsigned value, unsigned count, int amount) {
                return unsigned((int(value) + amount + int(count)) % int(count));
            };
            if (key == f4)
                next.set_time(step(level.time() / 10 * 10, 1000, 10 * by));
            else if (key == f5)
                next.set_theme(step(std::min(level.theme(), 4u), 5, by));
            else if (key == f6)
                next.set_palette(step(std::min(level.palette(), 5u), 6, by));
            else
                next.set_background(step(std::min(level.background(), 4u), 5, by));
            stroke = false;
            change(next);
            break;
        }
        case f10: save(); break;
        case key_l: {
            const uint8_t chosen = menu();
            if (chosen == key_z || chosen == key_y) {
                undo(chosen == key_z ? undone : redone, chosen == key_z ? redone : undone);
            } else if (chosen)
                return handle(chosen);
            break;
        }
        default:
            if (key >= digit_1 && key <= digit_0) {
                const uint8_t tile = common_tiles[key - digit_1];
                const auto& tiles = LevelRecord::tiles();
                hand = unsigned(std::find(tiles.begin(), tiles.end(), tile) - tiles.begin());
                draw_strip();
            }
        }
    return true;
}

void Editor::move(int columns, int rows) {
    const int to_column = int(column) + columns, to_row = int(row) + rows;
    if (to_column < 0 || to_row < 0 || to_column >= int(LevelRecord::columns) ||
        to_row >= int(LevelRecord::rows))
        return;
    column = unsigned(to_column);
    row = unsigned(to_row);
    // Space or Enter held paints.
    if (painting)
        place(LevelRecord::tiles()[hand]);
    else
        redraw();
}

void Editor::place(uint8_t tile) {
    LevelRecord next = level;
    if (next.place(column, row, tile))
        change(next);
    if (painting)
        stroke = true;
}

void Editor::change(const LevelRecord& next) {
    if (next == level)
        return;
    if (!stroke) {
        undone.push_back(level);
        if (undone.size() > undo_depth)
            undone.erase(undone.begin());
    }
    redone.clear();
    apply(next);
}

void Editor::undo(std::vector<LevelRecord>& from, std::vector<LevelRecord>& to) {
    if (from.empty())
        return;
    to.push_back(level);
    const LevelRecord next = from.back();
    from.pop_back();
    stroke = false;
    apply(next);
}

void Editor::apply(const LevelRecord& next) {
    const LevelRecord before = level;
    level = next;
    if (before.theme() != level.theme() || before.palette() != level.palette() ||
        before.background() != level.background()) {
        draw_board();
        screen.set_colours(g.palette_target(), 0, 63);
    } else
        for (unsigned r = 0; r < LevelRecord::rows; ++r)
            for (unsigned c = 0; c < LevelRecord::columns; ++c)
                if (before.at(c, r) != level.at(c, r))
                    g.board.put(level.at(c, r), uint8_t(c), uint8_t(r));
    draw_strip();
    redraw();
}

void Editor::cycle_tile(int by) {
    const int count = int(LevelRecord::tiles().size());
    hand = unsigned((int(hand) + by + count) % count);
    draw_strip();
}

void Editor::show() {
    screen.set_layout(true);
    g.load_sprites(); // the digits and the selector's sprites, under the theme's tiles
    draw_board();
    screen.set_colours(g.palette_target(), black, white);
    draw_strip();
    redraw();
    screen.fade_in();
    ticks = 0;
}

void Editor::draw_board() {
    g.m.test_level.assign(level.bytes.begin(), level.bytes.end());
    g.load_level(); // theme, palette, background and the board, from the record
    g.m.test_level.clear();
    g.unpack_at(g.theme_sheet_segments()[g.theme()], g.sprite_pixels()[1]);
    g.load_picture_at(g.background_segments()[g.background() + (g.background_bank() & 1 ? 5 : 0)]);
    screen.put_pixels(g.draw_page(), board_height * 80);
    screen.save_scenery();
    g.board.draw_all();
    screen.save_page();
}

void Editor::redraw() {
    screen.restore_page();
    const unsigned x = 16 * column, y = 16 * row;
    screen.fill(x, y, 16, 1, cursor_slot);
    screen.fill(x, y + 15, 16, 1, cursor_slot);
    screen.fill(x, y + 1, 1, 14, cursor_slot);
    screen.fill(x + 15, y + 1, 1, 14, cursor_slot);
    screen.flip();
}

// The cursor brighter and darker by turns, as the selector's frame.
void Editor::pulse() {
    if (ticks++ % 16)
        return;
    uint8_t palette[3 * (cursor_slot + 1)] = {};
    std::fill_n(palette + 3 * cursor_slot, 3, uint8_t(ticks / 16 % 2 ? 40 : 63));
    screen.set_colours(palette, cursor_slot, cursor_slot);
}

void Editor::on_strip(const std::function<void()>& draw) const {
    const uint16_t page = g.draw_page(), height = g.page_height();
    g.draw_page() = g.hud_page();
    g.page_height() = strip_height;
    draw();
    g.draw_page() = page;
    g.page_height() = height;
}

void Editor::write(unsigned x, unsigned y, const std::string& text, uint8_t colour) const {
    const uint8_t was = g.text_colour();
    g.text_colour() = colour;
    screen.move_to(x, y);
    for (const char character : text)
        screen.write(uint8_t(character));
    g.text_colour() = was;
}

void Editor::draw_strip() {
    g.unpack(g.font_resource(), g.work_buffer());
    on_strip([&] {
        screen.fill(0, 0, 320, strip_height, black);
        const auto& tiles = LevelRecord::tiles();
        const unsigned count = unsigned(tiles.size());
        screen.fill(hand_x - 2, tile_y - 2, 20, 20, white);
        screen.fill(hand_x - 1, tile_y - 1, 18, 18, black);
        const struct {
            unsigned x, index;
        } shown[] = {{previous_x, (hand + count - 1) % count}, {hand_x, hand},
                     {next_x, (hand + 1) % count}};
        for (const auto& tile : shown)
            screen.draw_sprite(g.tile_sprite(tiles[tile.index]), tile.x, tile_y);
        if (!note.empty()) {
            write(fields_x, tile_y, note.substr(0, 32), note_colour);
            return;
        }
        char fields[32];
        std::snprintf(fields, sizeof fields, "T%03u TH%u PA%u BG%u", level.time(), level.theme(),
                      level.palette(), level.background());
        write(fields_x, tile_y, fields, white);
        if (unsaved())
            write(unsaved_x, tile_y, "*", white);
        const bool current = has_check && checked == level;
        write(result_x, tile_y, current ? result.substr(0, 14) : "NOT CHECKED",
              current ? result_colour : grey);
    });
}

// Where a tap acts (E6): each cell of the board (by its number, row by row),
// then in the strip the tiles before and after the one in hand, and the one
// in hand, which opens the menu.
std::vector<Machine::Line> Editor::lines() const {
    std::vector<Machine::Line> found;
    for (unsigned r = 0; r < LevelRecord::rows; ++r)
        for (unsigned c = 0; c < LevelRecord::columns; ++c)
            found.push_back({16 * c, 16 * r, 16, 16, 0});
    const unsigned y = 178 + tile_y;
    found.push_back({previous_x, y, 16, 16, page_up});
    found.push_back({next_x, y, 16, 16, page_down});
    found.push_back({hand_x, y, 16, 16, key_l});
    return found;
}

// The level played as in a game, from its arrival to its end, recording
// nothing (rule 6 and E3, E4: Machine::recording is off).
void Editor::test() {
    screen.fade_out();
    {
        LineCursor none(g, Machine::other, {});
        g.m.test_level.assign(level.bytes.begin(), level.bytes.end());
        g.m.recording = false;
        g.m.tally = Machine::Tally();
        g.load_sprites();
        g.start_level();
        g.appear();
        g.m.level_ticks = 0;
        const Game::Outcome outcome = g.play();
        if (outcome == Game::completed)
            g.walk_out();
        g.screen.wipe(false);
        g.m.test_level.clear();
        if (outcome == Game::completed)
            note = "CLEARED IN " + tick_seconds(g.m.level_ticks) + "s";
        else
            note = outcome == Game::gave_up ? "GAVE UP" : "LOST: TIME OUT OR Q";
        note_colour = outcome == Game::completed ? green : red;
    }
    show();
}

// The checks of the level, then the solver: its par, or that the level
// cannot be cleared in time. Warnings and errors go to the message.
void Editor::check() {
    const LevelRecord::Check found = level.check();
    std::vector<std::string> said = found.errors;
    said.insert(said.end(), found.warnings.begin(), found.warnings.end());
    has_check = true;
    checked = level;
    par = Settings::Par();
    if (!found.errors.empty()) {
        result = found.errors[0];
        result_colour = red;
    } else {
        message("CHECKING...", white);
        screen.wait(1); // shown before the search holds the game up
        const Solution solution = solve(level.bytes.data());
        switch (solution.verdict) {
        case Solution::solvable:
            par.kind = Settings::Par::solvable;
            par.ticks = solution.ticks;
            result = "PAR " + tick_seconds(solution.ticks) + "s";
            result_colour = green;
            break;
        case Solution::too_slow:
            par.kind = Settings::Par::unsolvable;
            result = "TOO SLOW";
            result_colour = red;
            break;
        case Solution::unsolvable:
            par.kind = Settings::Par::unsolvable;
            result = "UNSOLVABLE";
            result_colour = red;
            break;
        case Solution::unknown:
            par.kind = Settings::Par::unknown;
            result = "UNKNOWN";
            result_colour = grey;
        }
    }
    std::string text;
    for (const std::string& part : said)
        text += (text.empty() ? "" : ", ") + part;
    note = text;
    note_colour = red;
    draw_strip();
}

void Editor::save() {
    if (!has_check || checked != level)
        check();
    const LevelRecord::Check found = level.check();
    if (!found.errors.empty()) {
        message("NOT SAVED: " + found.errors[0], red);
        return;
    }
    std::string to = file;
    LevelSet into = set;
    unsigned at = map;
    if (to.empty()) { // a built-in map: a copy, into a new pack
        const std::string name = enter_name();
        show();
        if (name.empty()) {
            message("NOT SAVED", red);
            return;
        }
        to = pack_file_name(name);
        if (g.m.level_sets)
            for (const Machine::LevelSetEntry& entry : g.m.level_sets())
                if (upper(entry.pack) == upper(to)) {
                    message("LEVELS/" + upper(to) + " IS THERE", red);
                    return;
                }
        into = LevelSet();
        into.name = name;
        at = 0;
    }
    const unsigned size = LevelSet::record_size;
    const bool added = at >= into.count;
    if (added)
        at = into.count++;
    into.records.resize(into.count * size);
    into.passwords.resize(into.count * 3);
    std::copy(level.bytes.begin(), level.bytes.end(), into.records.begin() + at * size);
    pad_levels(into);
    if (added) {
        new_password(into, at);
        pad_levels(into);
    }
    // Through E7's parser: what is saved reads back as this.
    const std::string text = write_levels(into);
    const Bytes bytes(text.begin(), text.end());
    try {
        const LevelSet back = read_levels(bytes, to, true);
        if (back.count != into.count || back.records != into.records ||
            back.passwords != into.passwords || back.name != into.name)
            throw std::runtime_error("does not read back the same");
    } catch (const std::exception& e) {
        message("NOT SAVED: " + upper(e.what()), red);
        return;
    }
    const std::string problem = g.m.save_pack ? g.m.save_pack(to, bytes) : "nowhere to save";
    if (!problem.empty()) {
        message("NOT SAVED: " + upper(problem), red);
        return;
    }
    // The pack is the level set played from now on; the map's par goes with
    // it when the check was of this level.
    Settings settings = g.load_settings();
    settings.set_pack(to);
    if (has_check && checked == level && par.kind != Settings::Par::none)
        settings.set_par(into.crc(), at + 1, par);
    g.save_settings(settings);
    if (g.m.change_assets)
        g.m.change_assets(g.m);
    g.load_scores(); // the pack's own table
    draw_board(); // in the program bytes of the assets as installed again
    screen.set_colours(g.palette_target(), 0, 63);
    redraw();
    set = into;
    map = at;
    file = to;
    saved = level;
    message("SAVED, MAP " + std::to_string(at + 1) + " OF " + upper(to), green);
}

void Editor::help() {
    const char* const text[] = {
        "ARROWS MOVE     SPACE ENTER PLACE",
        "HOLD SPACE OR ENTER WHILE MOVING: PAINT",
        "TAB PGUP PGDN TILE   DEL EMPTY SPACE",
        "1 FLOOR 2 WALL 3 SPACE 4 START 5 EXIT",
        "6 CRACKED 7 PLATFORM 8 TELEPORT",
        "9 HOURGLASS 0 OUTSIDE",
        "F2 PLAY   F3 CHECK   F10 CHECK, SAVE",
        "F4 TIME F5 THEME F6 PALETTE F7 BACKGR.",
        "SHIFT WITH TAB OR F4-F7: BACKWARDS",
        "CTRL Z UNDO  CTRL Y REDO  L MENU",
        "ESC LEAVE",
    };
    const unsigned lines = sizeof text / sizeof *text;
    screen.fill(0, 0, 320, board_height, black);
    for (unsigned n = 0; n < lines; ++n)
        write(0, 16 * n, text[n], white);
    write(176, 16 * (lines - 1), "ANY KEY: BACK", grey);
    screen.flip();
    g.read_scan();
    do { // until a key goes down, not as F1 goes up
        screen.wait(1);
        g.music.tick();
        g.read_scan();
    } while (!g.scan_change() || g.scan() >= released);
    shift = control = painting = false;
    redraw();
}

uint8_t Editor::menu() {
    const struct {
        const char* text;
        uint8_t key;
    } actions[] = {{"F1  HELP", f1},        {"F2  PLAY", f2},      {"F3  CHECK", f3},
                   {"F4  TIME", f4},        {"F5  THEME", f5},     {"F6  PALETTE", f6},
                   {"F7  BACKGROUND", f7},  {"UNDO", key_z},       {"REDO", key_y},
                   {"F10 SAVE", f10},       {"ESC LEAVE", leave_key}};
    const unsigned count = sizeof actions / sizeof *actions, left = 96, top = 0;
    std::vector<Machine::Line> lines;
    for (unsigned n = 0; n < count; ++n)
        lines.push_back({left, top + 16 * n, 8 * unsigned(std::string(actions[n].text).size()), 16,
                         actions[n].key});
    LineCursor cursor(g, Machine::editor, lines);
    const auto draw = [&] {
        screen.fill(0, 0, 320, board_height, black);
        for (unsigned n = 0; n < count; ++n)
            write(left, top + 16 * n, actions[n].text, white);
        write(216, top, "L, ESC: BACK", grey);
        cursor.draw();
        screen.flip();
    };
    draw();
    uint8_t chosen = 0;
    for (;;) {
        const uint8_t key = g.wait_for_line_key(cursor, draw);
        if (key == esc || key == key_l)
            break;
        if (std::any_of(lines.begin(), lines.end(), [&](const Machine::Line& line) { return line.key == key; })) {
            chosen = key;
            break;
        }
    }
    cursor.leave();
    ticks = 0;
    shift = control = painting = false;
    redraw();
    return chosen;
}

std::string Editor::enter_name() {
    screen.fade_out();
    LineCursor typing(g, Machine::name_entry, {}); // for the gamepad's letters (E6)
    screen.set_layout(false);
    uint8_t palette[3 * 256] = {};
    std::fill_n(palette + 3 * 1, 3, uint8_t(63));
    std::fill_n(palette + 3 * 2, 3, uint8_t(36));
    g.unpack(g.font_resource(), g.work_buffer());
    std::string name;
    const auto draw = [&] {
        screen.fill(0, 0, 320, 200, 0);
        write(8, 40, "A NEW PACK FOR THE MAP", 1);
        write(8, 64, "ITS NAME:", 2);
        write(8, 88, upper(name) + "_", 1);
        write(8, 178, "ENTER SAVE  ESC GIVE UP", 2);
        screen.flip();
    };
    draw();
    screen.set_colours(palette, 0, 255);
    g.m.connect_keyboard(false); // characters come from DOS here, as in name entry
    for (;;) {
        const uint8_t character = g.m.read_character();
        g.music.tick();
        if (character == 27) {
            name.clear();
            break;
        }
        if (character == 13 && name.find_first_not_of(' ') != std::string::npos)
            break;
        if (character == 8 && !name.empty())
            name.pop_back();
        else if (name.size() < 16 && (std::isalnum(character) || character == ' ' ||
                                      character == '-' || character == '_'))
            name += char(character);
        else
            continue;
        draw();
    }
    g.install_keyboard();
    typing.leave();
    screen.black();
    // Leading and trailing blanks go.
    const std::size_t first = name.find_first_not_of(' ');
    return first == std::string::npos ? "" : name.substr(first, name.find_last_not_of(' ') - first + 1);
}

// The level set played, as the game holds it: its records, passwords and name.
LevelSet played_set(const Game& g) {
    LevelSet set;
    g.unpack(g.levels_resource(), g.work_buffer());
    const uint8_t* const records = g.far(g.work_buffer());
    set.records.assign(records, records + LevelSet::most * LevelSet::record_size);
    set.passwords.assign(g.level_passwords(), g.level_passwords() + 3 * LevelSet::most);
    set.count = std::min(g.m.levels, unsigned(LevelSet::most));
    set.name = g.m.pack_name;
    return set;
}
} // namespace

void Game::level_editor() const {
    for (;;) {
        const int chosen = choose_map_to_edit();
        if (chosen == edit_left)
            return;
        const bool built_in = m.pack == Settings::free_maps || m.pack == Settings::original_maps;
        LevelSet set = played_set(*this);
        std::string file = built_in ? "" : m.pack;
        unsigned map = unsigned(std::max(chosen, 0));
        if (chosen == edit_new_pack) { // a pack of no levels yet, named when saved
            set = LevelSet();
            map = 0;
            file.clear();
        }
        Editor(*this, set, map, file).run();
    }
}
} // namespace stone
