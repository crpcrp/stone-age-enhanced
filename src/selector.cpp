// The map selector (ENHANCEMENTS.md, E1 to E3). Not original: enhanced mode
// only, in place of the password screen on F2 in the menu. All maps of the
// level set (100, or fewer in a pack, E7), 20 to a page, as miniatures of
// their boards; the ones never reached are a grey
// "unknown" map, the cleared ones have a mark, and the header shows the best
// time and points of the map under the cursor. Progress is kept in
// STONEAGE.ENH, one line per level set and one per cleared map.
#include "game.hpp"
#include "level_edit.hpp"
#include <algorithm>
#include <array>
#include <string>
#include <vector>

namespace stone {
namespace {
// Scan codes.
const uint8_t esc = 0x01, tab = 0x0f, key_p = 0x19, enter = 0x1c, key_l = 0x26, key_n = 0x31,
              space = 0x39,
              home = 0x47,
              arrow_up = 0x48, page_up = 0x49, arrow_left = 0x4b, arrow_right = 0x4d,
              end_key = 0x4f, arrow_down = 0x50, page_down = 0x51;
// Records in the level resource: a pack of fewer is padded (E7).
const unsigned records = 100, per_page = 20, per_row = 5;
// A thumbnail is the 20x11 board at three pixels a cell.
const unsigned thumb_width = 3 * Board::columns, thumb_height = 3 * Board::rows;
// The selector's own palette (the menu screens use 0-63 only).
const uint8_t text_slot = 1, dim_slot = 2, mark_slot = 3, theme_slots = 64, shade_slots = 144,
              highlight_slot = 149, grey_slots = 150;
// A board cell of each tile, sampled at these pixels of each axis.
const unsigned samples[3] = {2, 7, 12};

unsigned thumb_x(unsigned map) { return 8 + 62 * (map % per_row); }
unsigned thumb_y(unsigned map) { return 22 + 44 * (map % per_page / per_row); }

// L SETS in the header (E7), right of the best and left of the page marks;
// for the editor N NEW (E9) left of it.
const unsigned sets_x = 212, new_pack_x = 156;

// Where a tap acts (ENHANCEMENTS.md, E6): each miniature of the page of
// `map` with its number, by its place on the page, of the `count` maps;
// after them L SETS, which sends L, and for the editor N NEW, which sends N.
std::vector<Machine::Line> page_lines(unsigned map, unsigned count, bool editing) {
    std::vector<Machine::Line> lines;
    const unsigned first = map / per_page * per_page;
    for (unsigned n = first; n < std::min(first + per_page, count); ++n)
        lines.push_back({thumb_x(n), thumb_y(n), thumb_width, thumb_height + 8, 0});
    lines.push_back({sets_x, 2, 48, 14, key_l});
    if (editing)
        lines.push_back({new_pack_x, 2, 40, 14, key_n});
    return lines;
}

class MapSelector {
public:
    // For the editor (E9) every map is shown, and a pack of fewer than 100
    // has an empty slot after its last.
    MapSelector(const Game& game, bool editing)
        : g(game), screen(game.screen), editing(editing),
          maps(std::min(game.m.levels + (editing && pack_slot(game) ? 1 : 0), records)),
          pages((maps + per_page - 1) / per_page) {}
    // True when it is to open again: another level set was chosen.
    bool run();
    // For the editor: the map chosen, Game::edit_left or edit_new_pack.
    int chosen = Game::edit_left;

private:
    void gather();
    void build_palette();
    void fade(bool in);
    // A page with the cursor on `map`, onto the page being drawn; the
    // highlight frame `shift` pixels to the side.
    void draw(unsigned map, int shift = 0);
    void draw_thumbnail(uint8_t* pixels, unsigned map) const;
    void draw_unknown(uint8_t* pixels, unsigned map) const;
    // A cleared map: a triangle in the miniature's top-right corner (E3).
    void draw_mark(uint8_t* pixels, unsigned map) const;
    std::string header(unsigned map) const;
    void write(unsigned x, unsigned y, const std::string& text) const;
    // From the page of `from` to the page of `to`, 40 pixels a tick.
    void slide(unsigned from, unsigned to);
    void shake(unsigned map);
    bool reached(unsigned map) const { return editing || map < furthest; }
    // The empty slot of the editor.
    bool slot(unsigned map) const { return map >= g.m.levels; }
    static bool pack_slot(const Game& game) {
        return game.m.pack != Settings::free_maps && game.m.pack != Settings::original_maps;
    }

    const Game& g;
    const Screen& screen;
    const bool editing;
    const unsigned maps, pages; // in the level set, with the editor's slot
    std::vector<uint8_t> boards; // 220 tiles a map
    uint8_t themes[records] = {};
    uint8_t tile_pixels[5][40][9] = {}; // [theme][sprite][column * 3 + row]
    std::array<uint8_t, 768> palette{};
    unsigned furthest = 1; // 1-100
    unsigned last = 0;     // 1-100, 0 for none
    Settings::Best bests[records];
    uint32_t set_crc = 0; // Game::level_set(), which takes the work buffer
};

bool MapSelector::run() {
    screen.set_layout(false);
    gather();
    build_palette();
    // On the map last started (E2), or the furthest when that is unknown.
    unsigned cursor = std::min(last && reached(last - 1) ? last - 1 : furthest - 1, maps - 1);
    draw(cursor);
    screen.flip();
    // A tap on a miniature moves the cursor there; on the cursor it plays.
    if (editing)
        cursor = std::min(last ? last - 1 : 0u, maps - 1);
    LineCursor taps(g, Machine::selector, page_lines(cursor, maps, editing));
    fade(true);
    g.read_scan();
    for (unsigned ticks = 0;;) {
        screen.wait(1);
        g.music.tick();
        if (++ticks % 16 == 0) { // the frame pulses
            const uint8_t level = ticks / 16 % 2 ? 40 : 63;
            std::fill_n(&palette[3 * highlight_slot], 3, level);
            screen.set_colours(palette.data(), highlight_slot, highlight_slot);
        }
        uint8_t key = g.read_scan();
        int to = int(cursor);
        const int tapped = g.m.tapped;
        g.m.tapped = -1;
        if (tapped >= 0) { // the key is the tap's: Enter on the cursor, or none
            const unsigned map = cursor / per_page * per_page + unsigned(tapped);
            key = map == cursor ? enter : 0;
            to = int(map);
        } else if (!g.scan_change() || key >= 0x80)
            continue;
        switch (key) {
        case 0: break; // a tap on another map
        case arrow_left: to -= 1; break;
        case arrow_right: to += 1; break;
        case arrow_up: to -= per_row; break;
        case arrow_down: to += per_row; break;
        case page_up: to -= per_page; break;
        case page_down: to += per_page; break;
        case home: to = 0; break;
        case end_key: to = int(editing ? maps : furthest) - 1; break;
        case enter:
        case space:
            if (!reached(cursor)) {
                shake(cursor);
                continue;
            }
            taps.leave();
            fade(false);
            if (editing)
                chosen = int(cursor);
            else
                g.run_game(uint8_t(cursor)); // as a password start
            return false;
        case key_n: // E9
            if (!editing)
                continue;
            taps.leave();
            fade(false);
            chosen = Game::edit_new_pack;
            return false;
        case tab:
        case key_p:
            if (editing)
                continue;
            taps.leave();
            fade(false);
            g.enter_password();
            return false;
        case key_l: // E7
            taps.leave();
            fade(false);
            return g.choose_pack();
        case esc: fade(false); return false;
        default: continue;
        }
        to = std::min(std::max(to, 0), int(maps) - 1);
        if (unsigned(to) / per_page != cursor / per_page) {
            slide(cursor, unsigned(to));
            taps.replace(page_lines(unsigned(to), maps, editing));
        }
        cursor = unsigned(to);
        draw(cursor);
        screen.flip();
    }
}

// The level records, as load_level (CS:0ef2) reads them, the tiles of the
// five themes, and how far this level set has been played.
void MapSelector::gather() {
    const Settings settings = g.load_settings();
    const uint32_t set = set_crc = g.level_set();
    furthest = settings.reached(set);
    last = settings.last(set);
    for (unsigned map = 0; map < maps; ++map)
        if (!slot(map))
            bests[map] = settings.best(set, map + 1);
    const uint8_t* const records = g.far(g.work_buffer());
    const unsigned size = g.level_record_size();
    boards.resize(maps * Board::columns * Board::rows);
    for (unsigned map = 0; map < maps; ++map) {
        const uint8_t theme = uint8_t(records[map * size + 3] - '0');
        themes[map] = theme < 5 ? theme : 3;
        std::copy_n(slot(map) ? LevelRecord::empty().bytes.data() + 7 : records + map * size + 7,
                    Board::columns * Board::rows, &boards[map * Board::columns * Board::rows]);
        if (slot(map))
            themes[map] = 0;
    }
    // A theme's tiles are the first 40 sprites (CS:0fe9), column by column.
    const uint8_t* const sprite_records = g.far(g.sprite_records());
    for (unsigned theme = 0; theme < 5; ++theme) {
        g.unpack_at(g.theme_sheet_segments()[theme], g.sprite_pixels()[1]);
        for (unsigned sprite = 0; sprite < 40; ++sprite) {
            const uint8_t* const record = sprite_records + 4 * sprite;
            const unsigned width = record[2] ? record[2] : 256, height = record[3] ? record[3] : 256;
            const uint8_t* const pixels = g.far(g.sprite_pixels()) + (record[0] | record[1] << 8);
            for (unsigned x = 0; x < 3; ++x)
                for (unsigned y = 0; y < 3; ++y)
                    if (samples[x] < width && samples[y] < height)
                        tile_pixels[theme][sprite][x * 3 + y] =
                            pixels[samples[x] * height + samples[y]];
        }
    }
}

void MapSelector::build_palette() {
    const auto set = [&](unsigned slot, uint8_t red, uint8_t green, uint8_t blue) {
        palette[3 * slot] = red;
        palette[3 * slot + 1] = green;
        palette[3 * slot + 2] = blue;
    };
    set(text_slot, 63, 56, 36);
    set(dim_slot, 22, 20, 16);
    set(mark_slot, 63, 50, 10);
    for (unsigned theme = 0; theme < 5; ++theme) {
        const uint8_t* const colours = g.theme_palettes() + theme * 48;
        std::copy_n(colours, 48, &palette[3 * (theme_slots + 16 * theme)]);
        // The darkest background shade of its colour 14 that background set
        // A uses (colour 33), as a level makes it (CS:1938).
        for (unsigned part = 0; part < 3; ++part) {
            const unsigned scaled = uint16_t(colours[14 * 3 + part] * g.shade_factors()[6]);
            palette[3 * (shade_slots + theme) + part] =
                uint8_t(std::min(scaled * g.brightness() >> 8 & 255, 63u));
        }
    }
    set(highlight_slot, 63, 63, 63);
    for (unsigned n = 0; n < 8; ++n)
        set(grey_slots + n, uint8_t(10 + 5 * n), uint8_t(10 + 5 * n), uint8_t(10 + 5 * n));
    set(255, 63, 63, 63); // 254 stays black
}

// As fade_in and fade_out (CS:18b6, 18c5), but all 256 colours.
void MapSelector::fade(bool in) {
    std::array<uint8_t, 768> shown;
    for (unsigned step = 0; step <= 64; ++step) {
        const unsigned level = in ? 4 * step : 256 - 4 * step;
        for (unsigned n = 0; n < 768; ++n)
            shown[n] = uint8_t(palette[n] * level >> 8);
        screen.wait(1);
        screen.set_colours(shown.data(), 0, 255);
    }
}

void MapSelector::draw(unsigned cursor, int shift) {
    const unsigned first = cursor / per_page * per_page;
    uint8_t* const pixels = screen.work_buffer();
    std::fill_n(pixels, 320 * 200, uint8_t(0));
    const unsigned last_shown = std::min(first + per_page, maps);
    for (unsigned map = first; map < last_shown; ++map)
        if (reached(map)) {
            draw_thumbnail(pixels, map);
            if (bests[map].cleared())
                draw_mark(pixels, map);
        } else
            draw_unknown(pixels, map);
    screen.show_picture();
    // Text over it, in the 8x16 font of the asset set.
    g.unpack(g.font_resource(), g.work_buffer());
    const uint8_t colour = g.text_colour();
    g.text_colour() = text_slot;
    write(8, 2, header(cursor));
    g.text_colour() = grey_slots + 6; // E7: the list of level sets
    write(sets_x, 2, "L SETS");
    if (editing) // E9
        write(new_pack_x, 2, "N NEW");
    g.text_colour() = grey_slots + 7;
    for (unsigned map = first; map < last_shown; ++map)
        if (!reached(map))
            write(thumb_x(map) + 26, thumb_y(map) + 9, "?");
        else if (editing && slot(map))
            write(thumb_x(map) + 18, thumb_y(map) + 9, "NEW");
    g.text_colour() = colour;
    for (unsigned page = 0; page < pages; ++page) {
        const unsigned x = 268 + 10 * page;
        if (page == cursor / per_page)
            screen.fill(x, 7, 6, 6, text_slot);
        else {
            screen.fill(x, 7, 6, 6, dim_slot);
            screen.fill(x + 1, 8, 4, 4, 0);
        }
    }
    // The number under each thumbnail, in the digits of the status bar.
    for (unsigned map = first; map < last_shown; ++map) {
        const std::string digits = std::to_string(map + 1);
        const unsigned x = thumb_x(map) + thumb_width / 2 - 4 * unsigned(digits.size());
        for (unsigned n = 0; n < digits.size(); ++n)
            screen.draw_sprite(uint8_t(0x7c + digits[n] - '0'), x + 8 * n, thumb_y(map) + 35);
    }
    const unsigned x = unsigned(int(thumb_x(cursor)) - 2 + shift), y = thumb_y(cursor) - 2;
    screen.fill(x, y, thumb_width + 4, 2, highlight_slot);
    screen.fill(x, y + thumb_height + 2, thumb_width + 4, 2, highlight_slot);
    screen.fill(x, y + 2, 2, thumb_height, highlight_slot);
    screen.fill(x + thumb_width + 2, y + 2, 2, thumb_height, highlight_slot);
}

// Each cell three by three pixels of its tile in the map's theme. What is
// transparent shows a dark shade of the theme, like the level's background.
void MapSelector::draw_thumbnail(uint8_t* pixels, unsigned map) const {
    const unsigned theme = themes[map];
    const uint8_t* const board = &boards[map * Board::columns * Board::rows];
    uint8_t* const corner = pixels + thumb_y(map) * 320 + thumb_x(map);
    for (unsigned row = 0; row < Board::rows; ++row)
        for (unsigned column = 0; column < Board::columns; ++column) {
            uint8_t tile = board[row * Board::columns + column];
            if (tile == 'a' && g.full_background() >= 0x80) // as Board::draw
                tile = 'A';
            const uint8_t sprite = g.tile_sprite(tile);
            for (unsigned x = 0; x < 3; ++x)
                for (unsigned y = 0; y < 3; ++y) {
                    const uint8_t pixel = sprite < 40 ? tile_pixels[theme][sprite][x * 3 + y] : 0;
                    corner[(3 * row + y) * 320 + 3 * column + x] =
                        pixel == 0    ? uint8_t(shade_slots + theme)
                        : pixel < 16  ? uint8_t(theme_slots + 16 * theme + pixel)
                                      : pixel;
                }
        }
}

// Grey ground with a few grey rocks of three by three cells, placed from the
// map's number left and right of the middle, where a ? is written later.
void MapSelector::draw_unknown(uint8_t* pixels, unsigned map) const {
    uint8_t* const corner = pixels + thumb_y(map) * 320 + thumb_x(map);
    for (unsigned y = 0; y < thumb_height; ++y)
        std::fill_n(corner + y * 320, thumb_width, uint8_t(grey_slots + 1));
    uint32_t seed = map * 2654435761u + 12345;
    const auto next = [&](unsigned limit) {
        seed = seed * 1103515245u + 12345;
        return unsigned(seed >> 16) % limit;
    };
    for (unsigned rock = 0, rocks = 2 + next(3); rock < rocks; ++rock) {
        const unsigned column = next(12), left = 3 * (column < 6 ? column : column + 6),
                       top = 3 * next(Board::rows - 2);
        for (unsigned y = 0; y < 9; ++y)
            for (unsigned x = 0; x < 9; ++x)
                corner[(top + y) * 320 + left + x] =
                    uint8_t(grey_slots + (x == 0 || y == 0 ? 5 : x == 8 || y == 8 ? 2 : 3));
    }
}

void MapSelector::draw_mark(uint8_t* pixels, unsigned map) const {
    const unsigned size = 7, right = thumb_x(map) + thumb_width - 1;
    for (unsigned y = 0; y < size; ++y)
        for (unsigned x = 0; x < size - y; ++x)
            pixels[(thumb_y(map) + y) * 320 + right - x] =
                x == size - 1 - y ? uint8_t(254) : mark_slot; // a black edge
}

// The map, its bests (E3) and its par (E8): MAP 12  41.3s  1250  PAR 38.0s,
// closer together and shorter where that is too long for the room left of
// L SETS.
std::string MapSelector::header(unsigned map) const {
    if (editing) // E9
        return (slot(map) ? "NEW MAP " : "EDIT MAP ") + std::to_string(map + 1);
    const std::string number = "MAP " + std::to_string(map + 1);
    // Read anew each time: the player finds pars while the game goes on.
    const Settings::Par par = g.load_settings().par(set_crc, map + 1);
    std::string par_text;
    if (!reached(map))
        ; // unknown, as its miniature
    else if (par.kind == Settings::Par::solvable)
        par_text = "PAR " + tick_seconds(par.ticks);
    else if (par.kind == Settings::Par::unsolvable)
        par_text = "UNSOLVABLE";
    const Settings::Best& best = bests[map];
    if (!best.cleared())
        return par_text.empty() ? number : number + "  " + par_text + (par.ticks ? "s" : "");
    const std::string time = tick_seconds(best.ticks), points = std::to_string(best.points);
    if (par_text.empty())
        return number + "  " + time + "s  " + points;
    const unsigned room = (sets_x - 8) / 8;
    for (const std::string& line :
         {number + "  " + time + "s  " + points + "  " + par_text + (par.ticks ? "s" : ""),
          number + " " + time + "s " + points + " " + par_text + (par.ticks ? "s" : ""),
          number + " " + time + " " + points + " " + par_text})
        if (line.size() <= room)
            return line;
    return number + " " + time + " " + par_text;
}

void MapSelector::write(unsigned x, unsigned y, const std::string& text) const {
    screen.move_to(x, y);
    for (const char character : text)
        screen.write(uint8_t(character));
}

// Both pages side by side, 640 pixels wide; the view slides from one to the
// other in eight ticks. Keys pressed meanwhile are read after it.
void MapSelector::slide(unsigned from, unsigned to) {
    const bool forward = to > from;
    std::vector<uint8_t> both(640 * 200);
    for (const unsigned map : {from, to}) {
        draw(map);
        screen.get_pixels(g.draw_page(), g.page_bytes());
        const unsigned left = (map == to) == forward ? 320 : 0;
        for (unsigned y = 0; y < 200; ++y)
            std::copy_n(screen.work_buffer() + y * 320, 320, &both[y * 640 + left]);
    }
    for (unsigned step = 1; step <= 8; ++step) {
        const unsigned left = forward ? 40 * step : 320 - 40 * step;
        for (unsigned y = 0; y < 200; ++y)
            std::copy_n(&both[y * 640 + left], 320, screen.work_buffer() + y * 320);
        screen.show_picture();
        screen.flip();
        screen.wait(1);
        g.music.tick();
    }
}

// Enter on a map not reached: the frame shakes.
void MapSelector::shake(unsigned map) {
    for (const int shift : {3, -3, 2, -2, 1, -1, 0}) {
        draw(map, shift);
        screen.flip();
        for (unsigned tick = 0; tick < 2; ++tick) {
            screen.wait(1);
            g.music.tick();
        }
    }
}
} // namespace

std::string tick_seconds(unsigned ticks) {
    const unsigned tenths = (ticks + 3) / 7;
    return std::to_string(tenths / 10) + '.' + std::to_string(tenths % 10);
}

void Game::map_selector() const {
    while (MapSelector(*this, false).run()) {
    }
}

int Game::choose_map_to_edit() const {
    for (;;) {
        MapSelector selector(*this, true);
        if (!selector.run())
            return selector.chosen;
    }
}

uint32_t Game::level_set() const {
    unpack(levels_resource(), work_buffer());
    return crc32(far(work_buffer()), records * level_record_size());
}

void Game::record_level() const {
    const uint32_t set = level_set();
    Settings settings = load_settings();
    const unsigned started = level() + 1u;
    if (started <= settings.reached(set) && started == settings.last(set))
        return; // nothing new: no write
    if (started > settings.reached(set))
        settings.set_reached(set, started);
    settings.set_last(set, started); // E2
    save_settings(settings);
}

unsigned Game::score_value() const {
    unsigned value = 0;
    for (int n = 5; n >= 0; --n)
        value = value * 10 + score()[n] % 10;
    return value;
}

unsigned Game::record_best() const {
    if (!m.recording)
        return 0;
    // The score has six digits and goes round past 999,999.
    const unsigned points = (score_value() + 1000000 - m.level_score) % 1000000;
    Settings settings = load_settings();
    const unsigned improved = settings.improve_best(level_set(), level(), m.level_ticks, points);
    if (improved)
        save_settings(settings);
    return improved;
}
} // namespace stone
