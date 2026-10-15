// Everything around a level: start and end of the program, the menu and its
// screens, the pages between levels, music.
#include "game.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace stone {
namespace {
// Full-screen pictures, by their offset in the original file.
const unsigned menu_picture = 0x28720, password_picture = 0x2c330, options_picture = 0x2eb40,
               jukebox_picture = 0x30f10, scores_picture = 0x328d0, eclipse_picture = 0x38c30,
               bones_park_picture = 0x373e0, dinosaur_picture = 0x396a0, credits_picture = 0x3d770;
// Scan codes.
const uint8_t esc = 0x01, backspace = 0x0e, key_c = 0x2e, f1 = 0x3b, f2 = 0x3c, f3 = 0x3d,
              f4 = 0x3e, f5 = 0x3f, f6 = 0x40, f7 = 0x41, f8 = 0x42, f9 = 0x43, f10 = 0x44,
              del = 0x53;
const uint8_t digit_glyph = 0x1a; // the digit 0 in the sprite font of the pages
// A bird of the score screen: nine words in the work buffer.
enum { bird_x, bird_y, bird_drawn_x, bird_drawn_y, bird_angles, bird_frame = 8 };

// The sprite of a character in the jukebox's letters.
uint8_t jukebox_glyph(uint8_t character) {
    if (character >= 'A' && character <= 'Z')
        return character - 'A';
    if (character >= '0' && character <= '9')
        return character - 10;
    switch (character) {
    case '.': return 0x1a;
    case ',': return 0x1b;
    case '-': return 0x1d;
    case ' ': return 0x1e;
    case '!': return 0x20;
    case '\'': return 0x23;
    case ':': return 0x24;
    default: return character;
    }
}
} // namespace

// --- Music ---

void Music::driver(uint16_t function, uint16_t bx, uint16_t cx, uint16_t dx) const {
    m.ax = function;
    m.bx = bx;
    m.cx = cx;
    m.dx = dx;
    if (m.driver)
        m.driver(m);
}

void Music::play_next() const {
    if (names.adlib_enabled()) {
        if (names.song_ticks())
            return;
        uint8_t song = wrapped(m, names.playlist(), names.playlist_position());
        names.playlist_position() = names.playlist_position() + 1;
        if (song >= 0x80) { // the end of the list: from the top again
            names.playlist_position() = 1;
            song = names.playlist()[0];
        }
        if (song < 0x80) {
            if (song != names.current_song()) {
                stop();
                names.current_song() = song;
                driver(0, m.ds, uint16_t(names.song_files() - &m.data(0) + song * 13)); // load
                driver(0x18, 1);
                driver(1); // play
            }
            names.song_ticks() = 21000;
            return;
        }
    }
    stop();
    names.song_ticks() = 21000;
}

void Music::tick() const {
    if (names.song_ticks())
        names.song_ticks() = names.song_ticks() - 1;
}

void Music::stop() const {
    driver(0x1b, 10); // how fast to fade
    driver(0x19);     // fade out
    screen.wait(70);
    silence_channels();
    screen.wait(10);
}

void Music::silence_channels() const {
    driver(5);
    for (unsigned channel = 0; channel < 12; ++channel)
        driver(0x10, channel, names.driver_channel_values()[channel], 0);
}

void Music::reset() const {
    if (names.adlib_enabled())
        silence_channels();
}

void Music::shut_down() const {
    stop();
    driver(0x13);
}

unsigned Music::playlist_length() const {
    unsigned length = 0;
    while (names.playlist()[length] < 0x80)
        ++length;
    return length;
}

// --- The program ---

void Game::run() const {
    start();
    for (;;) {
        music.play_next();
        attract();
        MenuChoice choice;
        do
            choice = menu();
        while (choice == menu_again);
        screen.fade_out();
        if (choice == quit)
            break;
    }
    music.shut_down();
    m.connect_keyboard(false);
}

void Game::start() const {
    startup_video_mode() = 3; // the text mode DOS was in
    load_scores();
    // CS:1e69 asks the timer of an AdLib whether it is there. Here it always is.
    adlib_status_before() = 0;
    adlib_status_after() = 0xc0;
    adlib_enabled() = 0xff;
    music.reset();
    random_seed() = uint8_t(m.seconds);
    m.video_mode();
    screen.set_layout(false);
    install_keyboard();
    screen.clear_everything();
}

void Game::install_keyboard() const {
    old_keyboard_vector() = 0; // what DOS says its own handler is
    old_keyboard_vector()[1] = 0;
    m.connect_keyboard(true);
}

void Game::attract() const {
    const bool again = attract_seen();
    if (!show_picture(eclipse_picture, 210) && !(again && show_picture(bones_park_picture, 210)) &&
        !show_picture(dinosaur_picture, 280) && again)
        idle_credits();
    attract_seen() = 0xff;
}

bool Game::show_picture(unsigned file, unsigned ticks) const {
    load_picture(file);
    screen.show_picture();
    screen.flip();
    screen.fade_in();
    const bool key = wait_for_key(ticks);
    screen.fade_out();
    return key;
}

Game::MenuChoice Game::menu() const {
    music.play_next();
    screen.set_layout(false);
    load_sprites();
    load_picture(menu_picture);
    LineCursor cursor(*this, Machine::menu, menu_lines(), m.menu_line); // not original (E6)
    draw_menu(&cursor);
    screen.fade_in();
    for (;;) {
        read_scan();
        for (unsigned idle = 3500;;) {
            screen.wait(1);
            music.tick();
            cursor.pulse();
            if (--idle == 0)
                return pictures;
            uint8_t key = read_scan();
            if (!scan_change())
                continue;
            if (key < 0x80) { // not original (E6): the line cursor of enhanced mode
                key = cursor.key(key);
                m.menu_line = cursor.at;
                if (cursor.moved()) {
                    draw_menu(&cursor);
                    break; // the 50 seconds start again
                }
            }
            if (key == key_c)
                return pictures;
            if (key == esc)
                return quit;
            if (key < f1 || key > (m.enhanced ? f8 : f7)) // not original: F8 (E4)
                continue;
            if (key == f6)
                easy_mode() ^= 0xff;
            else if (key == f7)
                background_bank() ^= 1;
            else {
                cursor.leave();
                screen.fade_out();
                switch (key) {
                case f1: run_game(0); break;
                case f2: m.enhanced ? map_selector() : enter_password(); break;
                case f3: show_scores(); break;
                case f4: jukebox(); break;
                case f8: statistics(); break;
                default: options();
                }
                enter_score();
                return menu_again;
            }
            draw_menu(&cursor);
            break; // the 50 seconds start again
        }
    }
}

void Game::draw_menu(const LineCursor* cursor) const {
    // Not original (E4): the original's picture makes room for F8.
    const bool stats_line = m.enhanced && m.original_assets && load_settings().original(Settings::images);
    const unsigned up = stats_line ? 8 : 0;
    if (stats_line)
        add_stats_line();
    screen.show_picture();
    if (stats_line)
        load_picture(menu_picture);
    if (easy_mode() >= 0x80) { // LIGHT, in two sprites
        screen.draw_sprite(183, 218, 52);
        screen.draw_sprite(184, 239, 60);
    }
    screen.draw_sprite(background_bank() & 1 ? 182 : 181, 208, 127 - up); // A or B
    if (cursor)
        cursor->draw();
    screen.flip();
}

// Not original (ENHANCEMENTS.md, E4). The original's picture has no line for
// F8 (the free one has it drawn in). Its seven lines step down 10 pixels and
// right 7, and an eighth would run into the eclipse stone, so they move up 8
// pixels and the eighth, `F8 - STATS`, is put together from letters of the
// lines above it. Its 8 is their 6, closed at the top and pinched in the
// middle. The picture is unpacked again once it is shown.
void Game::add_stats_line() const {
    uint8_t* const picture = m.at(picture_buffer()[1], 0);
    const std::vector<uint8_t> was(picture, picture + 320 * 200);
    const uint8_t black = was[60 * 320 + 72];
    for (unsigned y = 59; y < 134; ++y)
        for (unsigned x = 72; x < 232; ++x)
            picture[y * 320 + x] = y + 8 < 134 ? was[(y + 8) * 320 + x] : black;
    struct Letter { unsigned from_x, from_y, width, to_x; };
    const Letter line[] = {{115, 127, 5, 122}, {116, 117, 7, 130}, {134, 127, 6, 141},
                           {212, 97, 7, 152},  {184, 97, 7, 160},  {140, 107, 7, 168},
                           {184, 97, 7, 176},  {212, 97, 7, 184}};
    const unsigned top = 129;
    for (const Letter& letter : line)
        for (unsigned y = 0; y < 7; ++y)
            for (unsigned x = 0; x < letter.width; ++x)
                picture[(top + y) * 320 + letter.to_x + x] =
                    was[(letter.from_y + y) * 320 + letter.from_x + x];
    uint8_t* const eight = picture + top * 320 + 130;
    for (unsigned y = 1; y < 3; ++y)
        for (unsigned x = 5; x < 7; ++x)
            eight[y * 320 + x] = eight[(y + 3) * 320 + x];
    eight[3 * 320] = eight[3 * 320 + 6] = black;
}

void Game::run_game(uint8_t first_level, bool counts) const {
    level() = first_level;
    load_sprites();
    new_session();
    screen.black();
    show_level_page();
    for (bool first = true;; first = false) {
        music.play_next();
        if (counts && m.enhanced) // not original
            record_level();
        m.recording = counts && m.enhanced && !cheat_mode(); // E3 and E4, rule 6
        m.level_score = score_value();
        m.tally = Machine::Tally();
        start_level();
        appear();
        m.level_ticks = 0;
        const Outcome outcome = play();
        if (outcome == completed) {
            walk_out();
            screen.wipe(false);
            record_stats(outcome, first); // not original (E4)
            if (++level() == 100) {
                award_bonus();
                record_best();
                victory_credits();
                return;
            }
            show_completed_page();
            continue;
        }
        // Giving up costs no life, but ends the game like the last one lost.
        const bool over = outcome == gave_up || --lives() == 0;
        screen.wipe(false);
        record_stats(outcome, first); // not original (E4)
        if (over) {
            show_game_over_page();
            return;
        }
        show_retry_page();
    }
}

void Game::new_session() const {
    std::fill_n(score(), 6, uint8_t(0));
    extra_life_at()[0] = 6;
    extra_life_at()[1] = extra_life_at()[2] = 0;
    lives() = 5;
}

// --- Pages between levels ---

void Game::begin_page() const {
    screen.set_layout(false);
    screen.set_target(glyph_palette(), 48, 16);
}

void Game::end_page(unsigned ticks) const {
    screen.flip();
    screen.fade_in();
    if (ticks)
        wait_for_key(ticks);
    else
        wait_for_key();
    screen.fade_out();
}

void Game::draw_page_text(const uint8_t* text) const {
    for (;; ++text) {
        page_text_x() = text[0] | text[1] << 8;
        page_text_y() = text[2];
        for (text += 3; *text != 0x7f; ++text) {
            if (*text >= 0x80)
                return;
            screen.draw_sprite(uint8_t(*text + 0x86), page_text_x(), page_text_y());
            page_text_x() = page_text_x() + 16;
        }
    }
}

void Game::show_level_page() const {
    begin_page();
    number_to_glyphs(level() + 1);
    std::copy_n(number_glyphs(), 3, level_page_number());
    draw_page_text(level_page());
    end_page(0);
}

void Game::show_retry_page() const {
    begin_page();
    draw_page_text(retry_page());
    end_page(0);
}

void Game::show_game_over_page() const {
    begin_page();
    draw_page_text(game_over_page());
    // CS:0a2c has `mov cx, cx` where the other pages clear CX: the time limit
    // is what drawing the text left there, the y of its last line. So this
    // page goes by itself after 118 ticks.
    end_page(page_text_y());
}

void Game::show_completed_page() const {
    begin_page();
    award_bonus();
    const unsigned improved = record_best(); // not original (E3)
    for (int n = 0; n < 6; ++n)
        completed_score()[5 - n] = score()[n] + digit_glyph;
    std::copy_n(lives() == 1 ? lives_phrase_single() : lives_phrase_plural(), 15,
                completed_lives());
    completed_lives_digit() = lives() + digit_glyph;
    number_to_glyphs(level() + 1); // the level that comes now
    std::copy_n(number_glyphs(), 3, completed_level());
    draw_page_text(completed_page());
    draw_best_line(improved);
    draw_page_text(&m.data(quote_pointers()[random(4)]));
    level_password(level());
    for (int pair = 0; pair < 3; ++pair) // letters to glyphs, two at a time
        Word{password_line_glyphs()}[pair] = Word{text()}[pair] - 0x4141;
    draw_page_text(password_line());
    end_page(490);
}

void Game::draw_best_line(unsigned improved) const {
    if (!improved)
        return;
    const std::string line = improved == (Settings::faster | Settings::higher)
                                 ? "NEW BEST TIME, SCORE"
                             : improved == Settings::faster ? "NEW BEST TIME"
                                                            : "NEW BEST SCORE";
    // A record of draw_page_text, centred between the lives and the quote.
    const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#$?!%.:,][ ";
    const unsigned x = (320 - 16 * unsigned(line.size())) / 2;
    std::vector<uint8_t> record = {uint8_t(x), uint8_t(x >> 8), 71};
    for (const char character : line)
        record.push_back(uint8_t(std::strchr(letters, character) - letters));
    record.push_back(0xff);
    draw_page_text(record.data());
}

void Game::award_bonus() const {
    uint16_t points = 0;
    if (clock.left())
        do
            points += 7;
        while (clock.pass_second());
    if (easy_mode())
        points /= 4;
    points += uint8_t(level() + 1) * 10;
    do { // no points at all count as 65,536
        bool carry = true;
        for (int n = 0; n < 6; ++n)
            carry = decimal_add(score()[n], carry);
    } while (--points);
    const auto thousands = [](const uint8_t* digits) {
        return digits[2] << 16 | digits[1] << 8 | digits[0];
    };
    if (thousands(score() + 3) < thousands(extra_life_at()) || lives() >= 9)
        return;
    ++lives();
    bool carry = decimal_add(extra_life_at()[0], 6);
    carry = decimal_add(extra_life_at()[1], carry);
    decimal_add(extra_life_at()[2], carry);
}

// --- Resources and keys ---

void Game::unpack_at(uint16_t from_segment, uint16_t to_segment) const {
    if (m.supply_resource(from_segment, to_segment))
        return;
    // CS:173b. Header "ICE!", packed and unpacked length (big-endian). Both
    // are read backwards from their last byte; bits come from the top of
    // `bits`, whose lowest set bit marks the end of the byte.
    auto big = [&](uint16_t at) {
        return uint16_t((m.read8(from_segment, at) << 8) | m.read8(from_segment, uint16_t(at + 1)));
    };
    if (big(0) != 0x4943 || big(2) != 0x4521)
        return;
    uint16_t from = uint16_t(big(6) - 1), to = uint16_t(big(10) - 1);
    uint8_t bits = m.read8(from_segment, from--);
    auto bit = [&]() -> unsigned { // CS:17a6
        unsigned out = bits >> 7;
        bits = uint8_t(bits << 1);
        if (!bits) {
            const uint8_t next = m.read8(from_segment, from--);
            bits = uint8_t((next << 1) | out);
            out = next >> 7;
        }
        return out;
    };
    auto number = [&](int16_t width) { // CS:17b0: width + 1 bits
        uint16_t value = 0;
        do
            value = uint16_t((value << 1) | bit());
        while (--width >= 0);
        return value;
    };
    const uint8_t* const tables = ice_tables(); // CS:1841
    auto table_word = [&](unsigned address) {
        return uint16_t(tables[address - 0x1841] | (tables[address - 0x1841 + 1] << 8));
    };
    auto table_byte = [&](unsigned address) {
        return int8_t(tables[address - 0x1841]);
    };
    for (;;) {
        if (bit()) { // CS:1771: literal bytes
            int16_t count = 0;
            if (bit()) {
                // Up to five widths: an all-ones value goes on to the next.
                unsigned at = 0x184b;
                for (int left = 4; left >= 0; --left) {
                    at -= 2;
                    count = int16_t(number(int16_t(table_word(at))));
                    if (uint16_t(count) != table_word(at + 0x14))
                        break;
                }
                count = int16_t(count + table_word(at + 0xa));
            }
            do
                m.write8(to_segment, to--, m.read8(from_segment, from--));
            while (--count >= 0);
        }
        if (to == 0xffff)
            return;
        // CS:17c1: a copy of what is already unpacked, length then distance.
        int kind = 3;
        while (kind >= 0 && bit())
            --kind;
        uint16_t length = 0;
        const int8_t length_width = table_byte(0x186a + kind);
        if (length_width >= 0)
            length = number(length_width);
        length = uint16_t(uint8_t(table_byte(0x186f + kind)) + length);
        uint16_t distance;
        if (length) {
            int width = 1;
            while (width >= 0 && bit())
                --width;
            distance = uint16_t(number(table_byte(0x1874 + width)) +
                                table_word(unsigned(0x1879 + 2 * width)));
            if (int16_t(distance) < 0)
                distance = uint16_t(distance - length);
        } else { // CS:1812: two bytes
            distance = bit() ? uint16_t(number(8) + 0x3f) : uint16_t(number(5) - 1);
        }
        uint16_t source = uint16_t(length + to + 2 + distance);
        m.write8(to_segment, to--, m.read8(to_segment, source--));
        do
            m.write8(to_segment, to--, m.read8(to_segment, source--));
        while (int16_t(--length) >= 0);
    }
}

void Game::load_picture_at(uint16_t segment) const {
    unpack_at(segment, picture_buffer()[1]);
    std::copy_n(m.at(picture_buffer()[1], 320 * 200), 768, palette_target());
}

void Game::load_sprites() const {
    unpack(sprites_resource(), sprite_pixels());
    unpack(sprite_records_resource(), sprite_records());
}

uint8_t Game::wait_for_key() const {
    read_scan();
    return wait_for_next_key();
}

uint8_t Game::wait_for_next_key() const {
    do {
        screen.wait(1);
        music.tick();
        read_scan();
    } while (!scan_change());
    return scan();
}

bool Game::wait_for_key(unsigned ticks) const {
    read_scan();
    for (; ticks; --ticks) {
        read_scan();
        if (scan_change())
            return true;
        screen.wait(1);
        music.tick();
    }
    return false;
}

// --- Password ---

void Game::enter_password() const {
    LineCursor typing(*this, Machine::password, {}); // not original (E6): for the gamepad
    m.connect_keyboard(false); // characters come from DOS here
    unpack(password_sheet_resource(), sprite_pixels());
    unpack(password_records_resource(), sprite_records());
    load_picture(password_picture);
    screen.show_picture();
    screen.flip();
    screen.show_picture();
    screen.save_page();
    screen.fade_in();
    uint8_t* const typed = password_input();
    std::fill_n(typed, 6, uint8_t(0));
    unsigned length = 0;
    for (;;) {
        uint8_t character = m.read_character();
        music.tick();
        if (character == 8) {
            if (length) {
                typed[--length] = 0;
                draw_password_glyph(0, length);
            }
            continue;
        }
        const bool digit = character >= '0' && character <= '9';
        if (!digit)
            character &= 0xdf; // letters to capitals; also makes '-' an Enter
        if (digit || (character >= 'A' && character <= 'Z')) {
            if (length < 6) {
                typed[length] = character;
                draw_password_glyph(digit ? 'X' - '@' : character - '@',
                                    length); // digits show as X
                ++length;
            }
        } else if (character == 13)
            break;
    }
    install_keyboard();
    typing.leave();
    int found = -1;
    bool cheat_level = false;
    if (length == 6) {
        const std::string word(typed, typed + 6);
        const auto digit = [&](int n) {
            return unsigned(typed[n] - '0');
        };
        if (word == "MAYDAY")
            toggle_cheat_mode();
        else if (cheat_mode() && word.substr(0, 3) == "LVL" && digit(3) <= 9 && digit(4) <= 9 &&
                 digit(5) <= 9)
            found = int(digit(3) * 100 + digit(4) * 10 + digit(5)) - 1;
        cheat_level = found >= 0 && found < 100;
        if (!cheat_level) {
            found = -1;
            for (int level = 0; level < 100 && found < 0; ++level) {
                level_password(uint8_t(level));
                if (std::equal(typed, typed + 6, text()))
                    found = level;
            }
        }
    }
    screen.fade_out();
    if (found >= 0) // not original: a level chosen by LVLnnn is not progress
        run_game(uint8_t(found), !cheat_level);
}

void Game::toggle_cheat_mode() const {
    // Colour 0 flashes: red when the hidden functions come on, blue when they go.
    uint8_t& part = flash_colour()[cheat_mode()];
    cheat_mode() ^= 2;
    const auto show = [&](int value) {
        part = uint8_t(value);
        screen.wait(1);
        screen.set_colours(flash_colour(), 0, 0);
    };
    for (int value = 0; value < 64; value += 4)
        show(value);
    screen.wait(2);
    for (int value = 60; value >= 0; value -= 4)
        show(value);
}

void Game::draw_password_glyph(uint8_t glyph, unsigned position) const {
    for (int page = 0; page < 2; ++page) {
        screen.draw_sprite(glyph, 135 + 16 * position, 83);
        screen.flip();
    }
}

void Game::level_password(uint8_t level) const {
    // Three bytes a level; each gives a consonant and a vowel.
    for (int n = 0; n < 3; ++n) {
        const uint8_t code = table_byte(level_passwords(), level * 3 + n);
        text()[2 * n] = password_consonants()[code >> 4];
        text()[2 * n + 1] = password_vowels()[code & 15];
    }
}

// --- Scores ---

void Game::show_scores() const {
    prepare_score_screen();
    screen.move_to(32, 7);
    screen.write(scores_heading()); // the table follows the heading and ends the text
    screen.save_page();
    screen.flip();
    screen.restore_page();
    move_birds();
    screen.flip();
    screen.fade_in();
    read_scan();
    do {
        erase_birds();
        move_birds();
        screen.flip();
        screen.wait(1);
        music.tick();
        read_scan();
    } while (!scan_change());
    screen.fade_out();
}

void Game::prepare_score_screen() const {
    load_picture(scores_picture);
    screen.set_target(red(), 15, 1);
    screen.show_picture();
    // The work buffer holds the font, then the sine table, then the birds.
    unpack(font_resource(), work_buffer());
    unpack_at(sine_resource()[1], uint16_t(work_buffer()[1] + 0x80));
    unpack(bird_sheet_resource(), sprite_pixels());
    std::copy_n(far(bird_records()), 28, far(sprite_records()));
    for (unsigned number = 0; number < 40; ++number) {
        const Word b = bird(number);
        b[bird_x] = b[bird_y] = b[bird_drawn_x] = b[bird_drawn_y] = 0;
        for (int n = 0; n < 4; ++n)
            b[bird_angles + n] = random(0x2000);
        b[bird_frame] = random(0x200);
    }
}

Word Game::bird(unsigned number) const {
    return Word{far(work_buffer()) + 0x4800 + 18 * number};
}

uint16_t Game::sine(uint16_t angle) const {
    return Word{far(work_buffer()) + 0x800}[angle & 0x1fff];
}

void Game::erase_birds() const {
    for (unsigned number = 0; number < 40; ++number) {
        const Word b = bird(number);
        screen.restore(b[bird_drawn_x], b[bird_drawn_y], 7, 4);
        b[bird_drawn_x] = b[bird_x];
        b[bird_drawn_y] = b[bird_y];
    }
}

void Game::move_birds() const {
    // The sum of two sines, each up to a quarter of 65,536, times a radius.
    const auto swing = [&](uint16_t angle, uint16_t other, int radius) {
        return uint16_t(uint32_t(int16_t(sine(angle) + sine(other)) * radius) << 2 >> 16);
    };
    const int speeds[] = {9, 7, 5, 3};
    for (unsigned number = 0; number < 40; ++number) {
        const Word b = bird(number);
        for (int n = 0; n < 4; ++n)
            b[bird_angles + n] = (b[bird_angles + n] + speeds[n]) & 0x1fff;
        b[bird_frame] = (b[bird_frame] + 1) & 0x1ff;
        b[bird_x] = swing(b[bird_angles], b[bird_angles + 1], 78) + 156;
        b[bird_y] = swing(b[bird_angles + 2], b[bird_angles + 3], 40) + 80;
        screen.draw_sprite(bird_sprites()[b[bird_frame] >> 2], b[bird_x], b[bird_y]);
    }
}

void Game::enter_score() const {
    uint8_t* const typed = text();
    // The score as six characters, the last digit first, leading zeros blank.
    bool leading = true;
    for (int n = 5; n >= 0; --n) {
        leading = leading && score()[n] == 0;
        typed[n] = leading ? ' ' : uint8_t(score()[n] + '0');
    }
    unsigned place = 0;
    while (place < 9 && !reaches_row(place))
        ++place;
    if (place < 9) {
        for (int row = 7; row >= int(place); --row) // the rows below move down
            std::copy_n(score_row(row) + 2, 28, score_row(row + 1) + 2);
        for (int n = 0; n < 6; ++n)
            score_row(place)[24 + 5 - n] = typed[n];
        LineCursor typing(*this, Machine::name_entry, {}); // not original (E6): for the gamepad
        m.connect_keyboard(false);
        prepare_score_screen();
        screen.save_page();
        screen.move_to(32, 9);
        screen.write(name_entry_text());
        screen.flip();
        screen.copy_shown_page();
        screen.fade_in();
        screen.move_back(); // onto the cursor the text ends with
        unsigned length = 0;
        for (;;) {
            const uint8_t character = m.read_character();
            music.tick();
            if (character == 8) {
                if (!length)
                    continue;
                typed[--length] = 0;
                // The last character and the cursor go, on both pages.
                screen.restore(text_x() - 8, text_y(), 16, 16);
                screen.flip();
                screen.restore(text_x() - 8, text_y(), 16, 16);
                screen.move_back();
                write_on_both_pages('_');
                screen.move_back();
            } else if (character >= ' ' && character <= 0xa0) {
                if (length >= 21)
                    continue;
                typed[length] = character;
                typed[length + 1] = 0;
                screen.restore(text_x(), text_y(), 8, 16);
                screen.flip();
                screen.restore(text_x(), text_y(), 8, 16);
                write_on_both_pages(character);
                write_on_both_pages('_');
                screen.move_back();
                ++length;
            } else if (character == 13 && length)
                break;
        }
        // The name, two blanks, and dots up to the score.
        uint8_t* const name = score_row(place) + 2;
        unsigned n = 0;
        for (; typed[n]; ++n)
            name[n] = typed[n];
        for (unsigned filler = 0; n < 21; ++n, ++filler)
            name[n] = filler < 2 ? ' ' : '.';
        screen.restore(text_x(), text_y(), 8, 16);
        screen.flip();
        install_keyboard();
        typing.leave();
        screen.fade_out();
        save_scores();
        show_scores();
    }
    std::fill_n(score(), 6, uint8_t(0));
}

bool Game::reaches_row(unsigned row) const {
    const uint8_t* const theirs = score_row(row) + 24; // first digit first
    for (int n = 0; n < 6; ++n)
        if (text()[5 - n] != theirs[n])
            return text()[5 - n] > theirs[n];
    return true;
}

void Game::write_on_both_pages(uint8_t character) const {
    screen.write(character);
    screen.flip();
    screen.move_back();
    screen.write(character);
}

void Game::invert_scores() const {
    for (int n = 0; n < 287; ++n)
        score_table()[n] ^= 0xff;
}

void Game::load_scores() const {
    Bytes file;
    if (!m.load_file || !m.load_file(reinterpret_cast<const char*>(score_file_name()), file))
        return; // the table built into the program stays
    file_handle() = 5;
    std::copy_n(file.begin(), std::min<std::size_t>(file.size(), 287), score_table());
    invert_scores();
}

void Game::save_scores() const {
    invert_scores();
    file_handle() = 5;
    if (m.save_file)
        m.save_file(reinterpret_cast<const char*>(score_file_name()),
                    Bytes(score_table(), score_table() + 287));
    invert_scores();
}

// --- Jukebox ---

void Game::jukebox() const {
    unpack(jukebox_sheet_resource(), sprite_pixels());
    unpack(jukebox_records_resource(), sprite_records());
    load_picture(jukebox_picture);
    screen.show_picture();
    jukebox_letters() = 0;
    screen.move_to(72, 61);
    write_jukebox(jukebox_song_list());
    screen.move_to(32, 168);
    write_jukebox(jukebox_help());
    screen.move_to(72, 134);
    write_jukebox(jukebox_prompt());
    screen.save_page();
    screen.flip();
    LineCursor cursor(*this, Machine::jukebox, jukebox_lines()); // not original (E6)
    const auto redraw = [&] { draw_playlist(&cursor); };
    redraw();
    screen.fade_in();
    for (;;) {
        const unsigned songs = music.playlist_length();
        const uint8_t key = wait_for_line_key(cursor, redraw);
        if (key >= f1 && key <= f8) {
            if (songs >= 20)
                continue;
            playlist()[songs] = key - f1;
            playlist()[songs + 1] = 0xff;
        } else if (key == f9) { // play the last one
            if (!songs)
                continue;
            playlist_position() = songs - 1;
            song_ticks() = 0;
            music.play_next();
        } else if (key == backspace || key == del) {
            if (!songs)
                continue;
            playlist()[songs - 1] = 0xff;
        } else if (key == f10) {
            playlist()[0] = 0xff;
            music.stop();
        } else if (key == esc) {
            playlist_position() = 0;
            song_ticks() = 0;
            cursor.leave();
            screen.fade_out();
            return;
        } else
            continue;
        redraw();
    }
}

void Game::draw_playlist(const LineCursor* cursor) const {
    screen.restore_page();
    jukebox_letters() = 50;
    text_y() = 147;
    const unsigned songs = music.playlist_length();
    if (!songs) {
        text_x() = 144;
        for (const char character : {'N', 'O', 'N', 'E'})
            write_jukebox(uint8_t(character));
    } else {
        text_x() = (20 - songs) * 8 + 4;
        for (unsigned n = 0; n < songs; ++n) {
            if (n)
                write_jukebox('-');
            write_jukebox(uint8_t(playlist()[n] + '1'));
        }
    }
    if (cursor)
        cursor->draw();
    screen.flip();
}

void Game::write_jukebox(const uint8_t* text) const {
    text_line_x() = text_x();
    while (const uint8_t character = *text++)
        write_jukebox(character);
    jukebox_letters() ^= 50;
}

void Game::write_jukebox(uint8_t character) const {
    if (character == '\r') { // the next line, in the other set of letters
        jukebox_letters() ^= 50;
        text_y() = text_y() + 8;
        text_x() = text_line_x();
        return;
    }
    screen.draw_sprite(uint8_t(jukebox_glyph(character) + jukebox_letters()), text_x(), text_y());
    text_x() = text_x() + 8;
}

// --- Options ---

void Game::options() const {
    show_options_screen();
    LineCursor cursor(*this, Machine::options, option_lines(5)); // not original (E6)
    const auto redraw = [&] { draw_options(&cursor); };
    redraw();
    screen.fade_in();
    for (;;) {
        const uint8_t key = wait_for_line_key(cursor, redraw);
        if (key == f1) // not original: the original's F1-F3 follow as F3-F5
            set_enhanced(!m.enhanced);
        else if (key == f2) { // not original
            cursor.hide();
            screen.fade_out();
            asset_options();
            show_options_screen(); // in the graphics now chosen
            redraw();
            screen.fade_in();
            continue;
        } else if (key == f3)
            full_background() ^= 0xff;
        else if (key == f4)
            flat_stone() ^= 0xff;
        else if (key == f5) {
            adlib_enabled() ^= 0xff;
            song_ticks() = 0;
            if (!adlib_enabled())
                music.stop();
            current_song() = 0xff;
            music.play_next();
        } else if (key == f10)
            break;
        redraw();
    }
    cursor.leave();
    screen.fade_out();
}

void Game::show_options_screen() const {
    unpack(options_sheet_resource(), sprite_pixels());
    std::copy_n(far(options_records()), 48, far(sprite_records()));
    load_picture(options_picture);
    screen.show_picture();
    screen.save_page();
}

void Game::draw_options(const LineCursor* cursor) const {
    const char* const labels[] = {"F1. ENHANCED", "F2. SETUP", "F3. FULL BACK",
                                  "F4. FLAT STONE", "F5. ADLIB SOUND"};
    const auto sprite = [](bool on) { return uint8_t(on ? 10 : 11); };
    const uint8_t sprites[] = {sprite(m.enhanced), 0, sprite(full_background() != 0),
                               sprite(flat_stone() != 0), sprite(adlib_enabled() != 0)};
    draw_option_lines(labels, sprites, 5, cursor);
}

// Not original. The picture's lines F1-F5 are covered with the commonest
// colour under them, and lines take their place, 17 pixels apart as the
// original's: in the 8x16 font of the asset set, in the colour the
// picture's lines use most, each with its ON or OFF sprite. F10 stays as
// the picture has it.
void Game::draw_option_lines(const char* const* labels, const uint8_t* sprites,
                             unsigned count, const LineCursor* cursor) const {
    const unsigned left = 88, top = 62, width = 124, height = 86;
    const uint8_t* const picture = m.at(picture_buffer()[1], 0);
    const uint8_t* const palette = picture + 320 * 200;
    unsigned uses[256] = {}, bright[256] = {};
    for (unsigned y = top; y < top + height; ++y)
        for (unsigned x = left; x < left + width; ++x) {
            const uint8_t colour = picture[y * 320 + x];
            ++uses[colour];
            if (palette[3 * colour] + palette[3 * colour + 1] + palette[3 * colour + 2] >= 60)
                ++bright[colour]; // the letters, not the dark background
        }
    const auto commonest = [](const unsigned* counts) {
        return uint8_t(std::max_element(counts, counts + 256) - counts);
    };
    screen.restore_page();
    screen.fill(left, top, width, height, commonest(uses));
    unpack(font_resource(), work_buffer());
    const uint8_t colour = text_colour();
    text_colour() = commonest(bright);
    floor_tile() = flat_stone() ? 'q' : 'B';
    for (unsigned n = 0; n < count; ++n) {
        screen.move_to(91, 64 + 17 * n);
        screen.write(reinterpret_cast<const uint8_t*>(labels[n]));
        if (sprites[n])
            screen.draw_sprite(sprites[n], sprites[n] == 10 ? 217 : 214, 66 + 17 * n);
    }
    text_colour() = colour;
    if (cursor)
        cursor->draw();
    screen.flip();
}

// Not original. Each line ends in FREE or ORIG, within the 15 characters
// the covered lines hold. Without the original's files everything is free
// and stays so, and a line says why. F4 cycles the display preset (E5),
// which the picture has no sixth line for in Options itself.
void Game::asset_options() const {
    const Settings saved = load_settings();
    bool original[Settings::parts];
    for (unsigned n = 0; n < Settings::parts; ++n)
        original[n] = m.original_assets && saved.original(Settings::Part(n));
    show_options_screen();
    LineCursor cursor(*this, Machine::setup, option_lines(Settings::parts + 1)); // not original (E6)
    const auto draw = [&]() {
        const char* const names[] = {"F1. IMAGES ", "F2. MAPS   ", "F3. MUSIC  "};
        std::string lines[Settings::parts + 1];
        const char* labels[Settings::parts + 2];
        const uint8_t sprites[Settings::parts + 2] = {};
        for (unsigned n = 0; n < Settings::parts; ++n) {
            lines[n] = std::string(names[n]) + (original[n] ? "ORIG" : "FREE");
            labels[n] = lines[n].c_str();
        }
        // E5: the display preset, its name ending where FREE and ORIG do.
        std::string name = Settings::display_name(Settings::Display(m.display));
        for (char& c : name)
            c = char(std::toupper(static_cast<unsigned char>(c)));
        lines[Settings::parts] = "F4. VIEW" + std::string(7 - name.size(), ' ') + name;
        labels[Settings::parts] = lines[Settings::parts].c_str();
        labels[Settings::parts + 1] = "NO ORIGINAL";
        draw_option_lines(labels, sprites, Settings::parts + (m.original_assets ? 1 : 2), &cursor);
    };
    draw();
    screen.fade_in();
    for (;;) {
        const uint8_t key = wait_for_line_key(cursor, draw);
        if (key == f10)
            break;
        if (key >= f1 && key < f1 + Settings::parts && m.original_assets) {
            original[key - f1] = !original[key - f1];
            draw();
        } else if (key == f4) { // shown at once: the host reads m.display every frame
            m.display = (m.display + 1) % Settings::displays;
            draw();
        }
    }
    cursor.leave();
    screen.fade_out();
    Settings settings = load_settings();
    if (settings.display() != Settings::Display(m.display)) {
        settings.set_display(Settings::Display(m.display));
        save_settings(settings);
    }
    bool changed = false;
    for (unsigned n = 0; n < Settings::parts; ++n)
        if (m.original_assets && original[n] != saved.original(Settings::Part(n))) {
            settings.set_original(Settings::Part(n), original[n]);
            changed = true;
        }
    if (!changed)
        return;
    save_settings(settings);
    if (m.change_assets)
        m.change_assets(m);
    // The new program bytes hold the built-in score table; the maps chosen
    // may keep their scores in another file.
    load_scores();
    if (original[Settings::music] != saved.original(Settings::music)) {
        song_ticks() = 0; // the song of the playlist, from the songs now chosen
        current_song() = 0xff;
        music.play_next();
    }
}

void Game::set_enhanced(bool on) const {
    m.enhanced = on;
    Settings settings = load_settings();
    settings.set_enhanced(on);
    save_settings(settings);
}

Settings Game::load_settings() const {
    Bytes file;
    if (!m.load_file || !m.load_file(Settings::file_name, file))
        file.clear(); // none yet: every setting as by default
    return Settings(file);
}

void Game::save_settings(const Settings& settings) const {
    if (m.save_file)
        m.save_file(Settings::file_name, settings.bytes());
}

// --- Credits ---

void Game::idle_credits() const {
    const bool swapped = random(0xffff) & 1;
    std::copy_n(credits_swap_second(), 20, swapped ? credits_name_b() : credits_name_a());
    std::copy_n(credits_swap_first(), 20, swapped ? credits_name_a() : credits_name_b());
    credits(credits_idle_text(), 2722);
}

void Game::victory_credits() const {
    screen.wait(35);
    credits(credits_victory_text(), 1748);
}

void Game::credits(const uint8_t* top, unsigned size) const {
    scratch() = uint16_t(top - &m.data(0) + size); // where the text ends
    screen.set_layout(false);
    load_picture(credits_picture);
    screen.set_target(white(), 15, 1);
    screen.show_picture();
    screen.save_page();
    screen.flip();
    screen.restore_page();
    unpack(font_resource(), work_buffer());
    read_scan();
    screen.fade_in();
    text_line_height() = 16;
    text_x() = 80;
    // `top` is the line at the top, y where it stands: one pixel up a frame.
    for (int y = 0;;) {
        screen.restore(80, 0, 160, 200);
        screen.wait(1);
        text_y() = y;
        const uint8_t* line = top;
        for (int n = 0; n < 7; ++n)
            line = screen.write(line);
        screen.wait(1);
        for (int n = 0; n < 7; ++n)
            line = screen.write(line);
        if (--y <= -16) { // it has left the page
            y += 16;
            while (*top++) {
            }
        }
        screen.flip();
        screen.wait(1);
        music.tick();
        if (top >= &m.data(0) + scratch())
            break;
        read_scan();
        if (scan_change())
            break;
    }
    text_line_height() = 14;
    screen.fade_out();
}
} // namespace stone
