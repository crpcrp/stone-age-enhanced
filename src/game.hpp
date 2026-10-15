#pragma once
#include "game_names.hpp"
#include "settings.hpp"

namespace stone {
// The game in readable form, on the memory the lifted code uses: game.cpp is
// a level, screens.cpp everything around it, video.cpp the drawing. A method
// that names a CS: address does what that lifted code does; READABLE in
// research/tools/lift_game.py lists them, and `replay_tests --routines` compares the
// two versions.

// A way to go. `bit` is the original's code for it, in walk_direction and
// platform_direction, and its bit in stone_neighbours.
struct Direction {
    uint8_t bit;
    int columns, rows;
};
const Direction up = {1, 0, -1}, down = {2, 0, 1}, left = {4, -1, 0}, right = {8, 1, 0};

// A table indexed by a word, as the original reads it: an index past the end
// gives whatever lies there in the data segment.
inline uint8_t& wrapped(Machine& m, uint8_t* table, uint16_t index) {
    return m.data(uint16_t(table - &m.data(0) + index));
}
// The original keeps time and score as one decimal digit a byte and adjusts
// them with the AAA and AAS instructions. These leave a digit as those do,
// also for a byte that was not a digit. True on a carry into the next digit.
bool decimal_add(uint8_t& digit, unsigned amount);
bool decimal_subtract(uint8_t& digit, unsigned amount);

// The original's random numbers (CS:211e): one sequence for the whole session,
// started from the seconds of the DOS clock.
class Random {
public:
    explicit Random(Machine& machine) : names(machine) {}
    // A number below `limit`.
    uint16_t operator()(uint16_t limit) const;

private:
    GameNames names;
};

// The VGA as the game sets it up: 320 pixels a line, four to a byte of video
// memory, one in each of four planes. That leaves room for several pages: the
// one shown, the one being drawn, one the picture is restored from, and in a
// level the scenery under the tiles and a strip of 22 lines below the board.
class Screen {
public:
    explicit Screen(Machine& machine) : m(machine), names(machine) {}

    // --- Pages ---
    // CS:1fe1 and CS:2075. Everything black and empty, laid out for a level
    // (178 lines and the strip) or for any other screen (200 lines).
    void set_layout(bool level) const;
    // CS:1fcf. All video memory to colour 0.
    void clear_everything() const;
    // CS:1df4. Lets this many ticks pass.
    void wait(unsigned ticks) const;
    // CS:1e0c. Shows the page that was drawn; the other one is drawn next.
    void flip() const;
    // CS:2493. The picture in the work buffer, a byte a pixel, onto the page
    // being drawn; put_pixels (CS:249d) for any part of video memory.
    void show_picture() const { put_pixels(names.draw_page(), names.page_bytes()); }
    void put_pixels(uint16_t to, uint16_t bytes) const;
    // CS:246b. The other way: video memory into the work buffer.
    void get_pixels(uint16_t from, uint16_t bytes) const;
    // CS:2464.
    void clear(uint16_t at, uint16_t bytes) const;
    // CS:1cfe, 1d04, 1d34. Whole pages: the drawn one kept to restore from,
    // or kept as scenery, and the shown one copied to the drawn one.
    void save_page() const;
    void save_scenery() const;
    void copy_shown_page() const;
    // CS:1a48. The page being drawn as it was saved.
    void restore_page() const { restore(0, 0, 320, names.page_height(), names.restore_page()); }

    // --- Rectangles: whole bytes, so x is rounded down to four pixels ---
    // CS:1a9a. From the page copied from last to the one being drawn.
    void restore(unsigned x, unsigned y, unsigned width, unsigned height) const;
    void restore(unsigned x, unsigned y, unsigned width, unsigned height, uint16_t from) const {
        names.copy_page() = from;
        restore(x, y, width, height);
    }
    // CS:1a62. 16 by 16 pixels from the saved page.
    void restore(unsigned x, unsigned y) const { restore(x, y, 16, 16, names.restore_page()); }
    // CS:1b0c. A drawn cell to the saved page.
    void save_cell(uint8_t column, uint8_t row) const;
    // CS:1a7b. A cell of the scenery.
    void draw_scenery(uint8_t column, uint8_t row) const {
        restore(column * 16, row * 16, 16, 16, names.scenery_page());
    }

    // --- Sprites ---
    // CS:1c69. Colour 0 is transparent. Lines from the page's height down
    // are in the strip below the board.
    void draw_sprite(uint8_t sprite, unsigned x, unsigned y) const;
    // CS:1c3d. The player where it is, and the two eyes of the strip, which
    // look toward it (CS:0df2).
    void draw_player() const;
    void draw_eyes() const;
    // CS:0d95. The eye that looks from a place in the strip toward the player.
    uint8_t eye_sprite(unsigned x, unsigned y) const;
    // CS:1a36. Puts the board back where the player was last drawn.
    void restore_player() const;
    // CS:1a2a. The same where the selector was last drawn.
    void restore_selector() const;
    // One digit of the strip below the board, at pixel column x.
    void draw_hud_digit(uint8_t digit, unsigned x) const {
        draw_sprite(0x7c + (digit & 15), x, 0xb9);
    }

    // --- Colours: six bits each of red, green and blue ---
    // CS:19dd. Colours of the palette at full brightness.
    void set_target(const uint8_t* colours, unsigned first, unsigned count) const;
    // CS:243b. Colours `first` to `last` of a palette to the VGA.
    void set_colours(const uint8_t* palette, unsigned first, unsigned last) const;
    // CS:2411. All 256 colours black, 32 a tick.
    void black() const;
    // CS:18b6, 18c5. Colours 0-63 from black to the palette and back, in 65
    // ticks; CS:18d3 for one step, `level` out of 256.
    void fade_in() const;
    void fade_out() const;
    void show_faded(unsigned level) const;
    // CS:187d. The same for a level: colours 0-15 and 32-47 fade, 16-31 stay.
    void show_board_faded(unsigned level) const;

    // --- Effects ---
    // CS:2232, 2326. 128 ticks: the board's colours fade in while the strip,
    // kept in the work buffer, appears column by column behind a scatter of
    // its pixels; or they fade out and the strip goes the same way.
    void wipe(bool in) const;
    // CS:21c1. 128 ticks: pixels of the shown page and of the work buffer
    // change places, 512 a tick. A second run puts them back.
    void swap_with_buffer() const;
    // CS:216b. Where the picture in the work buffer has colour 2, it gets
    // the pixel of the shown page.
    void show_through_buffer() const;
    // Not original: a rectangle of the page being drawn in one colour.
    void fill(unsigned x, unsigned y, unsigned width, unsigned height, uint8_t colour) const;

    // --- Text in the 8 by 16 font of the work buffer ---
    // CS:27f7. Text up to its NUL: LF goes back to the column the text
    // started in, CR down a line, E0h and E2h switch bold on and off. Returns
    // what follows the NUL.
    const uint8_t* write(const uint8_t* text) const;
    // CS:2845. One character at the cursor, which moves on.
    void write(uint8_t character) const;
    void move_to(unsigned x, unsigned y) const {
        names.text_x() = x;
        names.text_y() = y;
    }
    void move_back() const { names.text_x() = names.text_x() - 8; }

    uint8_t* work_buffer() const { return m.at(names.work_buffer()[1], names.work_buffer()); }

private:
    // The VGA's registers. A write goes to the planes selected; a read comes
    // from one. In copy mode a write stores, in all four planes at once, the
    // byte read last.
    void write_planes(unsigned planes) const;
    void read_plane(unsigned plane) const;
    void set_copy_mode(bool on) const;
    void copy(uint16_t from, uint16_t to, unsigned bytes) const;
    void copy_rectangle(uint16_t from, uint16_t to, unsigned x, unsigned y, unsigned width,
                        unsigned height) const;
    Machine& m;
    GameNames names;
};

// The tiles of the level being played, as letters (REFERENCE.md, "Tiles").
class Board {
public:
    static constexpr int columns = 20, rows = 11;
    explicit Board(Machine& machine)
        : names(machine), screen(machine), tiles(GameNames(machine).board()) {}
    // The tile at a cell. Everything off the board is wall.
    uint8_t at(int column, int row) const {
        const bool inside = column >= 0 && column < columns && row >= 0 && row < rows;
        return inside ? tiles[row * columns + column] : uint8_t('L');
    }
    // CS:1bad. The sprite of a tile: A is sprite 0, a is sprite 20.
    uint8_t sprite(uint8_t tile) const;
    // CS:1be0. A tile on the scenery of its cell.
    void draw(uint8_t tile, uint8_t column, uint8_t row) const;
    // CS:1bd7. Changes a cell and draws it, on the saved page too.
    void put(uint8_t tile, uint8_t column, uint8_t row) const;
    // CS:1916. The whole board, and the palette of the level.
    void draw_all() const;

private:
    GameNames names;
    Screen screen;
    uint8_t* tiles;
};

// The time left for the level: four decimal digits of seconds.
class Countdown {
public:
    explicit Countdown(Machine& machine) : names(machine), screen(machine) {}
    // CS:1d5c. One tick; a second passes every 70. False when no time is left.
    bool tick() const;
    // CS:1d68. A second less. False when none is left (CS:1d90).
    bool pass_second() const;
    bool left() const;
    // CS:1044. The time a level starts with.
    void set(uint16_t seconds) const;
    // The J tile.
    void add_fifty_seconds() const;
    // CS:0946.
    void draw() const;

private:
    GameNames names;
    Screen screen;
};

// Floors that crumble behind the player: four can run at once.
class Crumbling {
public:
    explicit Crumbling(Machine& machine) : names(machine), board(machine) {}
    // CS:0869. With all four slots busy the floor stays as it is.
    void start(uint8_t column, uint8_t row, uint8_t frames) const;
    // CS:010c. Each running slot shows its next tile, or ends.
    void animate() const;

private:
    GameNames names;
    Board board;
};

// The music driver (INT 64h) and the playlist of the jukebox.
class Music {
public:
    explicit Music(Machine& machine) : m(machine), names(machine), screen(machine) {}
    // CS:1f30. Once the time of a song is up, or at once when song_ticks is
    // 0: the next song of the playlist, or silence without one or without an
    // AdLib.
    void play_next() const;
    // CS:1f23. A tick of that time.
    void tick() const;
    // CS:1ebb. Fades the song out in 70 ticks and silences the channels.
    void stop() const;
    // CS:1efc. Silences the channels at once.
    void reset() const;
    // CS:1ef3. Stops, and takes the driver's timer away.
    void shut_down() const;
    // CS:2d5c. Songs in the playlist.
    unsigned playlist_length() const;

private:
    void driver(uint16_t function, uint16_t bx = 0, uint16_t cx = 0, uint16_t dx = 0) const;
    void silence_channels() const;
    Machine& m;
    GameNames names;
    Screen screen;
};

struct Game : GameNames {
    explicit Game(Machine& machine)
        : GameNames(machine), screen(machine), board(machine), clock(machine), crumbling(machine),
          music(machine), random(machine) {}
    Screen screen;
    Board board; // in place of the raw bytes GameNames::board() gives
    Countdown clock;
    Crumbling crumbling;
    Music music;
    Random random;

    uint8_t table_byte(uint8_t* table, uint16_t index) const { return wrapped(m, table, index); }
    // What a far pointer of the original points at.
    uint8_t* far(Word pointer) const { return m.at(pointer[1], pointer); }

    // --- The program (screens.cpp) ---
    // CS:0000. From DOS to DOS.
    void run() const;
    // CS:1fa8. Score file, sound, random numbers, video mode, keyboard.
    void start() const;
    // CS:0907. Scan bytes come from the keyboard again (CS:0920 hands it
    // back to DOS, for typing).
    void install_keyboard() const;
    // CS:3065. Two pictures, shown until a key or for three and four
    // seconds. From the second time on a third picture between them, and
    // the credits after them.
    void attract() const;
    // CS:3051. A picture for so many ticks. True when a key ended it.
    bool show_picture(unsigned file, unsigned ticks) const;
    // CS:2545. The menu, until it is left for one of its screens (which
    // then has been shown), for the pictures (C, or 50 seconds without a
    // key), or for DOS (Esc).
    enum MenuChoice { menu_again, pictures, quit };
    MenuChoice menu() const;
    // CS:2509. The menu picture with LIGHT, if it is on, and A or B.
    void draw_menu() const;
    // CS:0010. A game from this level until all lives are lost, Esc, or
    // the last level is completed. Not original: in enhanced mode each level
    // started is recorded as reached and as the last started, unless `counts`
    // is false (a start by LVLnnn, ENHANCEMENTS.md E1 and E2).
    void run_game(uint8_t first_level, bool counts = true) const;
    // CS:0ed7. No points, five lives.
    void new_session() const;

    // --- The menu's screens ---
    // CS:25e3. Six letters or digits and Enter. The password of a level
    // starts a game there. MAYDAY switches the hidden functions on or off
    // with a red or blue flash; with them on, LVL001 to LVL100 start a game
    // at that level.
    void enter_password() const;
    // CS:2694.
    void toggle_cheat_mode() const;
    // CS:25c6. A letter of the password, on both pages.
    void draw_password_glyph(uint8_t glyph, unsigned position) const;
    // CS:0b92. The password of a level into `text`.
    void level_password(uint8_t level) const;
    // CS:2916. The score table, with birds, until a key.
    void show_scores() const;
    // CS:295b. Its picture, the font, the sine table and the birds.
    void prepare_score_screen() const;
    // CS:2a05, 2a3d. The birds: 40, each on a path of four sines.
    Word bird(unsigned number) const;
    void erase_birds() const;
    void move_birds() const;
    uint16_t sine(uint16_t angle) const;
    // CS:2b1d. After every screen of the menu: a score that belongs in the
    // table is put there under a name typed in, the table is saved and
    // shown. The score is 0 afterwards.
    void enter_score() const;
    // CS:2d0e. Whether the score in `text` reaches the one in this row.
    bool reaches_row(unsigned row) const;
    uint8_t* score_row(unsigned row) const { return score_table() + 32 * row; }
    // CS:2b0e. A character on both pages.
    void write_on_both_pages(uint8_t character) const;
    // CS:2cc9, 2ca4. STONEAGE.HI: the table with every byte inverted.
    void load_scores() const;
    void save_scores() const;
    void invert_scores() const;
    // CS:2e6d. The jukebox: F1-F8 add a song to the playlist, F9 plays the
    // last one, Del or Backspace removes it, F10 empties the list, Esc leaves.
    void jukebox() const;
    // CS:2d6c. The playlist as numbers.
    void draw_playlist() const;
    // CS:2dd0, 2dea. Text in the jukebox's own letters; CR starts a line.
    void write_jukebox(const uint8_t* text) const;
    void write_jukebox(uint8_t character) const;
    // CS:2fb6. Options, changed in both modes (ENHANCEMENTS.md, E0):
    // F1 enhanced mode, F2 the assets, F3 full background, F4 flat stone, F5
    // sound, F10 leaves. The original's F1-F3 were the last three; its
    // brightness keys F4 and F5 are gone, brightness stays as the game starts.
    void options() const;
    // The options picture with its sprites, shown.
    void show_options_screen() const;
    // CS:2f4f, the lines drawn by code over the picture's.
    void draw_options() const;
    // Not original. Lines of `count` over those of the options picture,
    // each with sprite `sprites[n]` (ON or OFF) after it, or none for 0.
    void draw_option_lines(const char* const* labels, const uint8_t* sprites,
                           unsigned count) const;
    // Not original, F2 in Options: F1 images, F2 maps, F3 music, each
    // free or the original's, F10 back to Options. They change only with
    // the original game's files there; the choices are saved and the game
    // takes its assets from them when this is left.
    void asset_options() const;
    // Enhanced mode on or off, saved at once.
    void set_enhanced(bool on) const;
    // STONEAGE.ENH as it is saved (every default without one), and saved.
    Settings load_settings() const;
    void save_settings(const Settings& settings) const;

    // --- Map selector (selector.cpp; ENHANCEMENTS.md, E1), enhanced mode only ---
    // F2 in the menu: the 100 maps, 20 to a page; Enter on a reached one
    // starts a game there, Tab or P opens the password screen, Esc leaves.
    void map_selector() const;
    // The CRC-32 of the level records, which tells level sets apart.
    uint32_t level_set() const;
    // Level level() + 1 started: saved as the last one started (E2), and as
    // reached if it is further than that.
    void record_level() const;
    // CS:30f5. Text scrolling up over a picture, a pixel every three ticks,
    // until its end or a key. CS:30b7 before the game, with two names in an
    // order left to chance; CS:30a9 after the last level.
    void credits(const uint8_t* text, unsigned size) const;
    void idle_credits() const;
    void victory_credits() const;

    // --- Pages between levels: text in the sprite font ---
    // CS:09cf, 09fd, 0a17, 0ad5. Before the first level, after a lost life,
    // at the end of the game and after a level: that one adds the bonus and
    // shows score, lives, a quote left to chance and the next password.
    void show_level_page() const;
    void show_retry_page() const;
    void show_game_over_page() const;
    void show_completed_page() const;
    void begin_page() const;
    // CS:0b7a. Shows the page until a key, or for at most so many ticks.
    void end_page(unsigned ticks) const;
    // CS:0bcc. Records of x, y and glyphs: 7Fh starts the next record, a
    // byte from 80h up ends the text.
    void draw_page_text(const uint8_t* text) const;
    // CS:0a31. Seven points a second left (a quarter of it with LIGHT) and
    // ten a level, counted up one by one; a life at every 6,000 points.
    void award_bonus() const;

    // --- Resources ---
    // CS:173b: the ICE-packed resource at from_segment:0 unpacked to
    // to_segment:0 (the decoder ignores the offset its callers pass), or the
    // host's unpacked copy of it when the machine has `resource`.
    void unpack_at(uint16_t from_segment, uint16_t to_segment) const;
    void unpack(Word resource, Word to) const { unpack_at(resource[1], to[1]); }
    // CS:170f. A picture of 320 by 200 pixels and its palette into the work
    // buffer; the palette becomes the target.
    void load_picture(unsigned file) const { load_picture_at(resource_segment(file)); }
    void load_picture_at(uint16_t segment) const;
    // CS:0d2c. The sprites of the game and the menu.
    void load_sprites() const;
    // The segment a packed resource lies in, from its offset in the original file.
    static uint16_t resource_segment(unsigned file) {
        return uint16_t(Machine::load_segment + (file - 512) / 16);
    }

    // --- Keys outside a level ---
    // CS:24c9, 24cc. Waits for a key to go down or up; the first also
    // forgets a change that is already there. Returns the scan byte.
    uint8_t wait_for_key() const;
    uint8_t wait_for_next_key() const;
    // CS:1dd7. The same for at most so many ticks. True when a key came.
    bool wait_for_key(unsigned ticks) const;

    // --- A level (game.cpp) ---
    // CS:0fd2. Loads the level, draws board and strip and wipes them in.
    void start_level() const;
    // CS:0ef2. The level's record: time, theme, palette, background, tiles.
    void load_level() const;
    // CS:2158. P: the picture dissolves into the pause picture and stays
    // so until a key changes; then it comes back.
    void pause() const;
    // CS:0e5a. The player appears where the level starts, in eight frames of
    // nine ticks. Up to two exits are pointed out meanwhile.
    void appear() const;
    enum Outcome { playing, lost_life, gave_up, completed };
    // CS:0028. The level, tick after tick, until it ends in one of the
    // other three ways.
    Outcome play() const;
    // CS:002b. One tick of it.
    Outcome play_tick() const;
    // CS:03b7-0642. What the keys do once the mover stands on a cell.
    void obey_keys(uint8_t column, uint8_t row) const;
    // CS:0e38. The player leaves through the exit in twelve frames of seven
    // ticks.
    void walk_out() const;
    // The player where it stands, in one sprite after another.
    void animate_player(const uint8_t* sprites, int frames, unsigned ticks) const;
    // CS:002b. What a tick starts with: the countdown, and the board put
    // back where the player and the selector were drawn. False when the time
    // is up.
    bool begin_tick() const;
    // CS:082a. What a tick ends with: the things that move are drawn and the
    // page is shown.
    void show_tick() const;

    // --- Keyboard ---
    // CS:24da. The scan byte the keyboard sent last; scan_change is non-zero
    // when it is not the one read before.
    uint8_t read_scan() const;
    // CS:0cd7. Sets key_up, key_down, key_left, key_right and key_select for
    // this tick. Returns the scan byte if it is none of those keys, else 0.
    uint8_t read_keys() const;
    // The key bytes hold 0 while the key is down.
    static bool pressed(uint8_t key) { return key < 0x80; }
    uint8_t key(const Direction& way) const {
        return way.bit == 1   ? key_up()
               : way.bit == 2 ? key_down()
               : way.bit == 4 ? key_left()
                              : key_right();
    }

    // --- Tiles ---
    uint8_t tile_sprite(uint8_t tile) const { return board.sprite(tile); }
    // Whether a tile stops the player, the selector and platforms.
    bool blocks(uint8_t tile) const;
    // CS:0c54. Looks at the cell something is about to move onto.
    struct Probe {
        uint8_t tile;
        bool open;
    };
    Probe probe(int column, int row) const;
    Probe probe_beside(uint8_t column, uint8_t row, const Direction& way) const {
        return probe(int8_t(column + way.columns), int8_t(row + way.rows));
    }
    // CS:0bff. Sets stone_neighbours for the cells beside this one.
    void find_stone_neighbours(int column, int row) const;
    // CS:01a2. The mover has reached its cell: a K tile the player has left
    // starts to crumble, and tile_under is the tile here. `key` is the scan
    // byte read_keys returned; F10 after MAYDAY puts an exit under the player.
    // False while the mover is still on its way.
    bool arrive(uint8_t key) const;
    // CS:02f4. Arriving on a K tile: it crumbles once the player has left
    // it. True when the tile has nothing more to do: it is K, or the selector
    // is out, which sets nothing off.
    bool passes_over(uint8_t tile, uint8_t column, uint8_t row) const;
    // CS:0311. On an exit: true once no floor is crumbling any more.
    bool exit_open() const;
    // CS:0345. What the tile the player has arrived on gives or takes.
    // True when it is a teleport that now starts.
    bool step_on(uint8_t tile, uint8_t column, uint8_t row) const;

    // --- Movement ---
    // CS:0e07. Puts the player, standing, on the pixels of its cell.
    void place_player() const;
    // CS:08ca, 08d9, 08e8, 08f7. Makes the next cell that way the target.
    void aim(const Direction& way) const;
    // CS:0642. What the direction keys do this tick: the selector flies to
    // the next cell, the player walks or sets off on the platform under it.
    void steer(uint8_t column, uint8_t row) const;
    void walk(const Direction& way, uint8_t column, uint8_t row) const;
    // CS:0068. Moves the player, or the selector or platform while one is
    // out, `speed` pixels toward the target and sets `arrived`.
    void move_toward_target() const;
    // CS:07b7. Every fourth tick: the next walking frame of the player.
    void animate_walk() const;
    // CS:0d82. The selector ready to be steered.
    void show_selector() const;

    // --- Platforms: tiles C to I, which travel over stone ---
    // C, D, F and E go one way (up, down, left, right), G up and down,
    // H left and right, I all four.
    static bool platform_goes(uint8_t tile, const Direction& way);
    // CS:19e9. The platform under the player sets off: its cell turns to
    // stone and its tile travels with the player.
    void launch_platform(uint8_t direction) const;
    void launch(const Direction& way) const;
    // CS:0d47. The moving platform becomes a tile of the board again.
    void land_platform() const;
    // CS:03b7. A moving platform goes on while there is stone ahead; the key
    // of the opposite way turns it round. True while it keeps moving.
    bool ride_platform(uint8_t column, uint8_t row) const;
    // CS:04ba. A selected platform with more than one way to go waits for a
    // direction key. True when it has set off.
    bool steer_waiting_platform(uint8_t column, uint8_t row) const;
    // CS:084c. A tile drawn where the player, or the selector, is.
    void draw_at_mover(uint8_t tile) const;

    // --- Teleports: Q tiles ---
    // A level with two takes the player from one to the other. With more,
    // the player shows up in one after another until Space is held.
    // What follows an animation: teleport_continue holds the address of the
    // original's code for it.
    enum Then : uint16_t { find_other = 0x263, come_out = 0x27c, find_next = 0x2b7, offer = 0x2d1 };
    // CS:0221. True while the player is in a teleport, which takes the tick.
    bool teleport() const;
    // CS:023c. Seven frames of six ticks on the cell in selector_cell.
    void start_teleport_animation(Then then) const;
    bool teleport_animates() const { return teleport_animating() >= 0x80; }
    // CS:0150. One tick of it, and after the last what follows. True when
    // the player has come out of the teleport.
    bool animate_teleport() const;
    bool after_teleport_animation() const;
    // CS:0258. The next Q tile after a cell, row by row and round to the top.
    uint16_t next_teleport(uint16_t cell) const;

    // --- Selector ---
    bool selector_out() const { return selecting() >= 0x80; }
    // CS:0543. Space or Enter with the selector in: it appears on the player
    // in six frames of four ticks each. Nothing else happens meanwhile: the
    // countdown stands still too.
    void bring_out_selector(uint8_t column, uint8_t row) const;
    // CS:0593. Space or Enter with the selector out: a platform under it sets
    // off or waits for a key, anything else calls the selector back. False
    // when the direction keys still steer this tick.
    bool drop_selector(uint8_t column, uint8_t row) const;
    // CS:0891. The selector gives way to the player again.
    void withdraw_selector() const;

    // --- The strip below the board ---
    // CS:0967.
    void draw_score() const;
    // CS:0991. The two key counts; more than nine show as 9.
    void draw_inventory() const;
    // CS:09b5. number_glyphs becomes the three decimal digits of `value`.
    void number_to_glyphs(uint8_t value) const;
};
} // namespace stone
