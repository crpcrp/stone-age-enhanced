#include "game.hpp"
#include <algorithm>
#include <stdexcept>

namespace stone {
bool decimal_add(uint8_t& digit, unsigned amount) {
    const unsigned sum = digit + amount;
    const bool carry = (sum & 15) > 9 || (digit & 15u) + amount > 15;
    digit = uint8_t((carry ? sum + 6 : sum) & 15);
    return carry;
}
bool decimal_subtract(uint8_t& digit, unsigned amount) {
    const unsigned rest = digit - amount;
    const bool borrow = (rest & 15) > 9 || (digit & 15u) < amount;
    digit = uint8_t((borrow ? rest - 6 : rest) & 15);
    return borrow;
}
namespace {
// One coordinate `speed` pixels closer to where it is heading.
void approach(Word position, uint16_t target, uint16_t speed) {
    if (position != target)
        position = position < target ? position + speed : position - speed;
}
} // namespace

uint8_t Board::sprite(uint8_t tile) const {
    if (tile == 'B')
        tile = names.floor_tile();
    const bool lowercase = tile & 0x20;
    return uint8_t((tile & 0x1f) - 1 + (lowercase ? 20 : 0));
}

void Board::draw(uint8_t tile, uint8_t column, uint8_t row) const {
    if (tile == 'a' && names.full_background() >= 0x80)
        tile = 'A';
    screen.draw_scenery(column, row);
    screen.draw_sprite(sprite(tile), column * 16, row * 16);
}

void Board::put(uint8_t tile, uint8_t column, uint8_t row) const {
    if (tile == 'q')
        tile = 'B';
    tiles[row * columns + column] = tile;
    draw(tile, column, row);
    screen.save_cell(column, row);
}

void Board::draw_all() const {
    for (uint8_t row = 0; row < rows; ++row)
        for (uint8_t column = 0; column < columns; ++column) {
            const uint8_t tile = tiles[row * columns + column];
            if (tile == 'R') // where the player starts
                names.player_cell() = uint16_t(column | row << 8);
            draw(tile, column, row);
        }
    // Colours 0-15 are the theme's, for tiles and sprites.
    const uint8_t* const theme = names.theme_palettes() + uint8_t(names.theme()) * 48;
    screen.set_target(theme, 0, 16);
    // 32-39 are eight shades of its colour 14, for the background, by the
    // brightness of the options. The pictures of bank B have eight of their
    // own there, and those of bank A after them in 40-47.
    const int shades = names.background_bank() & 1 ? 16 : 8;
    for (int shade = 0; shade < shades; ++shade)
        for (int part = 0; part < 3; ++part) {
            const uint16_t scaled =
                uint16_t(theme[14 * 3 + part] * names.shade_factors()[shades - 1 - shade]);
            names.palette_target()[(32 + shade) * 3 + part] =
                uint8_t(std::min(scaled * names.brightness() >> 8 & 255, 63));
        }
    // 16-31 are the scenery's, 48-63 those of the letters, 254 and 255 black and white.
    screen.set_target(names.scenery_palettes() + names.scenery_palette() * 48, 16, 16);
    screen.set_target(names.flash_colour(), 254, 2);
    screen.set_target(names.glyph_palette(), 48, 16);
}

bool Countdown::tick() const {
    if (--names.countdown_ticks() != 0)
        return left();
    names.countdown_ticks() = 70;
    return pass_second();
}

bool Countdown::pass_second() const {
    ++names.timer_changed();
    bool borrow = true;
    for (int n = 0; n < 4; ++n)
        borrow = decimal_subtract(names.countdown()[n], borrow);
    return left();
}

bool Countdown::left() const {
    const uint8_t* const digits = names.countdown();
    return (digits[0] | digits[1] | digits[2] | digits[3]) != 0;
}

void Countdown::set(uint16_t seconds) const {
    uint8_t* const digits = names.countdown();
    std::fill(digits, digits + 4, uint8_t(0));
    do { // counted up one by one; from 0 that goes all the way round to 5,536
        bool carry = true;
        for (int n = 0; n < 4; ++n)
            carry = decimal_add(digits[n], carry);
    } while (--seconds);
    names.scratch() = 0; // the original's counter
}

void Countdown::add_fifty_seconds() const {
    uint8_t* const digits = names.countdown();
    bool carry = decimal_add(digits[1], 5);
    carry = decimal_add(digits[2], carry);
    decimal_add(digits[3], carry);
}

void Countdown::draw() const {
    for (int n = 0; n < 4; ++n)
        screen.draw_hud_digit(names.countdown()[3 - n], 0x10 + 8 * n);
}

void Crumbling::start(uint8_t column, uint8_t row, uint8_t frames) const {
    names.stone_neighbours() = frames; // the original parks its argument here
    for (int slot = 3; slot >= 0; --slot)
        if (!names.crumble_active()[slot]) {
            names.crumble_column()[slot] = column;
            names.crumble_row()[slot] = row;
            names.crumble_frame()[slot] = frames;
            names.crumble_active()[slot] = 0xff;
            return;
        }
}

void Crumbling::animate() const {
    for (int slot = 3; slot >= 0; --slot) {
        if (!names.crumble_active()[slot])
            continue;
        names.scratch() = slot;
        const uint8_t frame = uint8_t(names.crumble_frame()[slot] - 1);
        if (frame >= 0x80) { // it was the last one
            names.crumble_active()[slot] = 0;
            continue;
        }
        names.crumble_frame()[slot] = frame;
        board.put(names.crumble_tiles()[frame], names.crumble_column()[slot],
                  names.crumble_row()[slot]);
    }
}

void Game::start_level() const {
    screen.set_layout(true);
    std::fill_n(m.at(level_state_pointer()[1], level_state_pointer()), 350, uint8_t(0));
    arrived() = 0xff;
    speed() = 1;
    walk_delay() = 4;
    countdown_ticks() = 70;
    load_level();
    // The theme's tiles are the first 40 sprites.
    unpack_at(theme_sheet_segments()[theme()], sprite_pixels()[1]);
    load_picture_at(background_segments()[background() + (background_bank() & 1 ? 5 : 0)]);
    screen.put_pixels(draw_page(), 176 * 80);
    screen.save_scenery();
    board.draw_all();
    screen.save_page();
    screen.flip();
    screen.restore_page();
    // A quarter more than the level's record says; twice that with LIGHT.
    const uint16_t seconds = uint16_t(level_time() + level_time() / 4);
    clock.set(easy_mode() >= 0x80 ? uint16_t(seconds * 2) : seconds);
    screen.black();
    // The strip is drawn where it belongs, taken into the work buffer and
    // cleared again: the wipe brings it back with the board's colours.
    unpack_at(resource_segment(0x25530), picture_buffer()[1]);
    screen.put_pixels(hud_page(), 22 * 80);
    draw_inventory();
    draw_score();
    clock.draw();
    screen.draw_eyes();
    screen.get_pixels(hud_page(), 22 * 80);
    screen.clear(hud_page(), 22 * 80);
    screen.set_colours(palette_target(), 255, 255);
    screen.wipe(true);
}

void Game::load_level() const {
    unpack(levels_resource(), work_buffer());
    const uint16_t segment = work_buffer()[1];
    uint16_t at = uint16_t(level() * level_record_size());
    const auto next = [&] {
        return m.read8(segment, at++);
    };
    // Three digits of seconds. A middle "digit" from 26 up would count twice.
    const uint8_t hundreds = next() - '0', tens = next() - '0', ones = next() - '0';
    loaded_level_time() = hundreds * 100 + tens * 10 + (tens * 10 & 0xff00) + ones;
    const uint8_t loaded_theme = next() - '0', palette = next();
    loaded_theme_palette() = loaded_theme | palette << 8;
    loaded_background() = next() & 7;
    ++at; // a byte nothing reads
    std::copy_n(m.at(segment, at), Board::columns * Board::rows, GameNames::board());
    level_time() = loaded_level_time();
    theme().at[0] = loaded_theme < 5 ? loaded_theme : 3;
    scenery_palette() = palette;
    background().at[0] = loaded_background();
    // Exits and teleports are counted. The first two exits found, from the
    // bottom right, get a marker while the player appears: below an exit of
    // the upper rows, above one of the lower.
    exit_count() = teleport_count() = 0;
    for (int row = Board::rows - 1; row >= 0; --row)
        for (int column = Board::columns - 1; column >= 0; --column) {
            const uint8_t tile = probe(column, row).tile;
            if (tile == 'Q')
                ++teleport_count();
            if (tile != 'S' || ++exit_count() > 2)
                continue;
            const unsigned marker = exit_count() - 1;
            exit_marker_x()[marker] = column * 16;
            exit_marker_sprite()[marker] = row < 5 ? 0x27 : 0x13;
            exit_marker_y()[marker] = row < 5 ? row * 16 + 11 : row * 16 - 6;
        }
}

void Game::pause() const {
    unpack(pause_mask_resource(), work_buffer());
    screen.show_through_buffer();
    screen.swap_with_buffer();
    wait_for_key();
    screen.swap_with_buffer();
}

void Game::appear() const {
    place_player();
    player_sprite() = selector_sprite() = selecting() = walk_direction() = 0;
    const int markers = exit_count() > 2 ? 0 : exit_count() == 2 ? 2 : 1;
    for (int n = markers - 1; n >= 0; --n)
        screen.draw_sprite(uint8_t(exit_marker_sprite()[n]), exit_marker_x()[n],
                           exit_marker_y()[n]);
    animate_player(arrival_sprites(), 8, 9);
    for (int n = markers - 1; n >= 0; --n)
        screen.restore(exit_marker_x()[n], exit_marker_y()[n]);
}

Game::Outcome Game::play() const {
    for (;;) {
        screen.wait(1);
        const Outcome outcome = play_tick();
        if (outcome != playing)
            return outcome;
    }
}

Game::Outcome Game::play_tick() const {
    if (!begin_tick())
        return lost_life;
    move_toward_target();
    crumbling.animate();
    bool walking = true; // whether the walk animation goes on this tick
    if (teleport_animates())
        walking = animate_teleport();
    else {
        const uint8_t key = read_keys();
        if (key == 0x10) // Q: try the level again
            return lost_life;
        if (key == 0x01) // Esc
            return gave_up;
        if (key == 0x19 && scan_change()) // P
            pause();
        if (arrive(key)) {
            const uint8_t tile = tile_under(), column = player_column(), row = player_row();
            bool taken = teleport(); // whether the tile takes the tick from the keys
            if (!taken && !passes_over(tile, column, row)) {
                if (tile != 'S')
                    taken = step_on(tile, column, row);
                else if (exit_open())
                    return completed;
                else
                    taken = true;
            }
            if (!taken)
                obey_keys(column, row);
            walking = !taken;
        }
    }
    if (walking)
        animate_walk();
    show_tick();
    return playing;
}

void Game::obey_keys(uint8_t column, uint8_t row) const {
    if (ride_platform(column, row) || steer_waiting_platform(column, row))
        return;
    if (pressed(key_select())) {
        if (!selector_out())
            bring_out_selector(column, row);
        else if (drop_selector(column, row))
            return;
    }
    steer(column, row);
}

void Game::walk_out() const {
    animate_player(exit_sprites(), 12, 7);
}

void Game::animate_player(const uint8_t* sprites, int frames, unsigned ticks) const {
    for (int n = 0; n < frames; ++n) {
        screen.wait(ticks);
        screen.restore_player();
        player_sprite() = sprites[n];
        screen.draw_player();
        screen.flip();
    }
}

bool Game::begin_tick() const {
    ++m.level_ticks; // not original (ENHANCEMENTS.md, E3)
    if (song_ticks()) // CS:1f23: the time until the next song of the playlist
        song_ticks() = song_ticks() - 1;
    const bool time_left = clock.tick();
    if (timer_changed()) {
        clock.draw();
        timer_changed() = 0;
    }
    if (!time_left)
        return false;
    screen.restore_selector();
    if (!selector_out())
        screen.restore_player();
    return true;
}

void Game::show_tick() const {
    if (platform_moving() >= 0x80)
        draw_at_mover(platform_tile());
    if (!selector_out())
        screen.draw_player();
    screen.draw_sprite(selector_sprite(), selector_x(), selector_y());
    screen.flip();
}

uint8_t Game::read_scan() const {
    scan_before() = scan();
    scan() = keyboard_scan();
    scan_change() = scan() ^ scan_before();
    return scan();
}

uint8_t Game::read_keys() const {
    const uint8_t released = 0xff, pressed = 0;
    key_select_before() = key_select();
    key_up() = key_down() = key_left() = key_right() = key_select() = released;
    const uint8_t key = read_scan();
    switch (key) {
    case 0x48: key_up() = pressed; return 0;
    case 0x50: key_down() = pressed; return 0;
    case 0x4b: key_left() = pressed; return 0;
    case 0x4d: key_right() = pressed; return 0;
    case 0x39: // Space
    case 0x1c: // Enter: only on the tick the key goes down
        if (scan_change()) {
            key_select() = pressed;
            return 0;
        }
    }
    return key;
}

bool Game::blocks(uint8_t tile) const {
    switch (tile) {
    case 'A': // stone
    case 'L': // wall
        return true;
    case 'M': // the gates open for a key of their kind
        return keys_a() == 0;
    case 'N': return keys_b() == 0;
    default: return tile >= 'a' && tile <= 'm';
    }
}

Game::Probe Game::probe(int column, int row) const {
    const uint8_t tile = board.at(column, row);
    const bool open = !blocks(tile);
    // Being stopped arms the teleports again, except while the selector is out.
    if (!open && selecting() < 0x80)
        teleport_armed() = 0xff;
    probed_tile() = tile;
    return {tile, open};
}

void Game::find_stone_neighbours(int column, int row) const {
    // Stone is A or a. The probes run in the original's order: each one
    // overwrites probed_tile.
    const auto stone = [&](int c, int r) {
        return (probe(c, r).tile & 0xdf) == 'A';
    };
    const bool up = stone(column, row - 1), down = stone(column, row + 1);
    const bool left = stone(column - 1, row), right = stone(column + 1, row);
    stone_neighbours() = uint8_t(up * 1 + down * 2 + left * 4 + right * 8);
}

bool Game::arrive(uint8_t key) const {
    if (arrived() < 0x80)
        return false;
    player_column() += step_column();
    player_row() += step_row();
    step_column() = step_row() = 0;
    if (cheat_mode() && key == 0x44)
        board.put('S', player_column(), player_row());
    const uint16_t left = crumble_cell();
    if (crumble_pending() >= 0x80 && !selector_out() && player_cell() != left) {
        crumbling.start(uint8_t(left), uint8_t(left >> 8), 23);
        board.put('k', uint8_t(left), uint8_t(left >> 8));
        crumble_pending() = 0;
    }
    tile_under() = probe(int8_t(player_column()), int8_t(player_row())).tile;
    if (tile_under() != 'Q' && !selector_out())
        teleport_armed() = 0xff;
    return true;
}

bool Game::passes_over(uint8_t tile, uint8_t column, uint8_t row) const {
    if (selector_out())
        return true;
    if (tile != 'K')
        return false;
    crumble_pending() = 0xff;
    crumble_cell() = uint16_t(column | row << 8);
    return true;
}

bool Game::exit_open() const {
    exit_reached() = 0xff;
    for (int slot = 3; slot >= 0; --slot)
        if (crumble_active()[slot])
            return false;
    return true;
}

bool Game::step_on(uint8_t tile, uint8_t column, uint8_t row) const {
    switch (tile) {
    case 'O': // a key; the 256th would wrap the count and is left lying
        if (++keys_a() == 0)
            return false;
        ++m.tally.keys; // not original (ENHANCEMENTS.md, E4), as below
        break;
    case 'P':
        ++keys_b();
        ++m.tally.keys;
        break;
    case 'M': // a gate takes its key
        --keys_a();
        ++m.tally.gates;
        break;
    case 'N':
        --keys_b();
        ++m.tally.gates;
        break;
    case 'J':
        ++m.tally.hourglasses;
        clock.add_fifty_seconds();
        clock.draw();
        board.put(floor_tile(), column, row);
        return false;
    case 'Q': // a teleport, once the player has left one or been stopped
        if (!teleport_armed())
            return false;
        player_sprite() = 0;
        selector_cell() = uint16_t(column | row << 8);
        --teleporting();
        ++m.tally.teleports;
        return true;
    default: return false;
    }
    draw_inventory();
    board.put(floor_tile(), column, row);
    return false;
}

void Game::place_player() const {
    player_x() = target_x() = player_drawn_x() = player_column() * 16;
    player_y() = target_y() = player_drawn_y() = player_row() * 16;
}

void Game::aim(const Direction& way) const {
    unused_220f() = unused_20e8(); // CS:08bd copies two bytes nothing reads
    unused_2210() = unused_20e9();
    if (way.columns) {
        target_x() = target_x() + 16 * way.columns;
        step_column() = uint8_t(way.columns);
    } else {
        target_y() = target_y() + 16 * way.rows;
        step_row() = uint8_t(way.rows);
    }
}

void Game::steer(uint8_t column, uint8_t row) const {
    if (platform_waits() >= 0x80)
        return;
    if (selector_out()) {
        // The selector flies over everything; only the edge of the board stops it.
        if (pressed(key_up())) {
            if (row != 0)
                aim(up);
        } else if (pressed(key_down()) && row != Board::rows - 1)
            aim(down);
        if (pressed(key_left())) {
            if (column != 0)
                aim(left);
        } else if (!pressed(key_right()))
            walk_direction() = 0;
        else if (column != Board::columns - 1)
            aim(right);
        return;
    }
    for (const Direction& way : {up, down, left, right})
        if (pressed(key(way))) {
            walk(way, column, row);
            return;
        }
    walk_direction() = 0;
}

void Game::walk(const Direction& way, uint8_t column, uint8_t row) const {
    if (walk_direction() != way.bit) {
        walk_direction() = way.bit;
        walk_frame() = 0;
    }
    const Probe ahead = probe_beside(column, row, way);
    if (ahead.open) {
        ++m.tally.steps; // not original (ENHANCEMENTS.md, E4)
        aim(way);
        return;
    }
    walk_frame() = 0;
    // Stopped by stone while standing on a platform that goes this way.
    if ((ahead.tile & 0x5f) == 'A' && platform_goes(tile_under(), way))
        launch(way);
}

bool Game::platform_goes(uint8_t tile, const Direction& way) {
    const uint8_t one_way = way.bit == 1 ? 'C' : way.bit == 2 ? 'D' : way.bit == 4 ? 'F' : 'E';
    return tile == one_way || tile == (way.rows ? 'G' : 'H') || tile == 'I';
}

void Game::launch_platform(uint8_t direction) const {
    ++m.tally.platforms; // not original (ENHANCEMENTS.md, E4)
    platform_direction() = direction;
    --platform_moving();
    speed() = 2;
    platform_tile() = probe(int8_t(player_column()), int8_t(player_row())).tile;
    board.put('A', player_column(), player_row());
    draw_at_mover(probed_tile());
}

void Game::launch(const Direction& way) const {
    launch_platform(way.bit);
    aim(way);
}

void Game::land_platform() const {
    const uint8_t tile = platform_tile();
    board.put(tile, player_column(), player_row());
    platform_tile() = platform_moving() = platform_direction() = 0;
    speed() = 1;
    if (!selector_out())
        return;
    if (tile == 'G' || tile == 'H' || tile == 'I')
        platform_waits() = 0xff; // it can go on another way: wait for a key
    else
        show_selector();
}

bool Game::ride_platform(uint8_t column, uint8_t row) const {
    const uint8_t going = platform_direction(), tile = platform_tile();
    const bool turns_vertically = tile == 'G' || tile == 'I';
    const bool turns_sideways = tile == 'H' || tile == 'I';
    const auto go_on = [&](const Direction& way) {
        if ((probe_beside(column, row, way).tile & 0xdf) == 'A') {
            aim(way);
            return true;
        }
        land_platform();
        return false;
    };
    const auto turn_sideways = [&](const Direction& way) {
        if (!selector_out())
            walk_direction() = way.bit;
        platform_direction() = way.bit;
        aim(way);
    };
    if (going & up.bit) {
        if (pressed(key_down()) && turns_vertically) {
            walk_direction() = platform_direction() = down.bit;
            aim(down);
            return true;
        }
        return go_on(up);
    }
    if (going & down.bit) {
        // Turning back up takes effect a tick later: this tick it still goes down.
        if (pressed(key_up()) && turns_vertically)
            walk_direction() = platform_direction() = up.bit;
        return go_on(down);
    }
    if (going & left.bit) {
        if (pressed(key_right()) && turns_sideways) {
            turn_sideways(right);
            return true;
        }
        return go_on(left);
    }
    if (going & right.bit) {
        if (pressed(key_left()) && turns_sideways) {
            turn_sideways(left);
            return true;
        }
        return go_on(right);
    }
    return false;
}

bool Game::steer_waiting_platform(uint8_t column, uint8_t row) const {
    if (platform_waits() < 0x80)
        return false;
    const uint8_t tile = probe(int8_t(column), int8_t(row)).tile;
    find_stone_neighbours(int8_t(column), int8_t(row));
    for (const Direction& way : {up, down, left, right})
        if (pressed(key(way))) {
            const bool turns = tile == (way.rows ? 'G' : 'H') || tile == 'I';
            if (!turns || !(stone_neighbours() & way.bit))
                return false;
            launch(way);
            return true;
        }
    return false;
}

void Game::draw_at_mover(uint8_t tile) const {
    const bool selector = selector_out();
    screen.draw_sprite(tile_sprite(tile), selector ? selector_x() : player_x(),
                       selector ? selector_y() : player_y());
}

bool Game::teleport() const {
    if (teleporting() < 0x80) {
        teleport_cycling() = 0;
        return false;
    }
    if (teleport_count() <= 2)
        start_teleport_animation(find_other);
    else if (teleport_cycling() < 0x80) {
        --teleport_cycling();
        start_teleport_animation(find_next);
    } else {
        selector_cell() = next_teleport(selector_cell());
        start_teleport_animation(offer);
    }
    return true;
}

void Game::start_teleport_animation(Then then) const {
    teleport_continue() = then;
    --teleport_animating();
    player_sprite() = 0;
    teleport_frame_ticks() = 6;
    teleport_frame() = 6;
}

bool Game::animate_teleport() const {
    const uint16_t cell = selector_cell();
    const uint8_t tile = table_byte(teleport_sprites(), teleport_frame());
    screen.draw_sprite(tile_sprite(tile), (cell & 255) * 16, (cell >> 8) * 16);
    if (--teleport_frame_ticks())
        return false;
    teleport_frame_ticks() = 6;
    teleport_frame() = teleport_frame() - 1;
    if (int16_t(teleport_frame()) >= 0)
        return false;
    teleport_animating() = 0;
    return after_teleport_animation();
}

bool Game::after_teleport_animation() const {
    switch (teleport_continue()) {
    case find_other: // the player has gone: now the animation where it comes out
        selector_cell() = next_teleport(selector_cell());
        start_teleport_animation(come_out);
        return false;
    case find_next:
        selector_cell() = next_teleport(selector_cell());
        start_teleport_animation(offer);
        return false;
    case offer:
        if (read_scan() != 0x39) { // Space is not held: on to the next teleport
            teleporting() = teleport_armed() = unused_2223() = 0xff;
            player_sprite() = 0;
            return false;
        }
        // fall through
    case come_out:
        player_cell() = selector_cell();
        place_player();
        teleporting() = teleport_armed() = unused_2223() = 0;
        walk_direction() = 0;
        return true;
    default: throw std::runtime_error("Unknown teleport step");
    }
}

uint16_t Game::next_teleport(uint16_t cell) const {
    uint8_t column = uint8_t(cell), row = uint8_t(cell >> 8);
    for (int tried = 0; tried < 2 * Board::columns * Board::rows; ++tried) {
        if (++column >= Board::columns) {
            column = 0;
            if (++row >= Board::rows)
                row = 0;
        }
        if (probe(int8_t(column), int8_t(row)).tile == 'Q')
            return uint16_t(column | row << 8);
    }
    throw std::runtime_error("No teleport on the board"); // the original looks for ever
}

void Game::bring_out_selector(uint8_t column, uint8_t row) const {
    // The player becomes part of the board's picture while the selector is out.
    screen.draw_player();
    screen.save_cell(column, row);
    selector_cell() = player_cell();
    selector_x() = selector_drawn_x() = player_x();
    selector_y() = selector_drawn_y() = player_y();
    for (int frame = 5; frame >= 0; --frame) {
        screen.wait(4);
        screen.restore_player();
        screen.draw_player();
        selector_sprite() = selector_flash_sprites()[frame];
        screen.draw_sprite(selector_sprite(), selector_x(), selector_y());
        screen.flip();
    }
    selecting() = 0xff;
    speed() = 2;
}

bool Game::drop_selector(uint8_t column, uint8_t row) const {
    if (platform_waits() >= 0x80) {
        show_selector();
        return false;
    }
    selector_sprite() = 0;
    const uint8_t tile = probed_tile();
    const bool moved = uint16_t(column | row << 8) != selector_cell();
    if (!moved || tile < 'C' || tile > 'I') {
        withdraw_selector();
        return true;
    }
    find_stone_neighbours(int8_t(column), int8_t(row));
    // The platform sets off if stone lies on exactly one of its ways.
    uint8_t ways = 0;
    for (const Direction& way : {up, down, left, right})
        if (platform_goes(tile, way))
            ways |= way.bit;
    for (const Direction& way : {up, down, left, right})
        if ((stone_neighbours() & ways) == way.bit) {
            launch(way);
            return true;
        }
    const bool one_way = tile >= 'C' && tile <= 'F';
    if (one_way) {
        withdraw_selector();
        return true;
    }
    platform_waits() = 0xff;
    return false;
}

void Game::withdraw_selector() const {
    target_x() = player_x();
    target_y() = player_y();
    player_cell() = selector_cell();
    selecting() = 0;
    speed() = 1;
    const uint8_t tile = probe(int8_t(player_column()), int8_t(player_row())).tile;
    screen.draw_sprite(tile_sprite(tile), player_x(), player_y());
    screen.save_cell(player_column(), player_row());
}

void Game::move_toward_target() const {
    const bool selector = selecting() >= 0x80;
    const Word x = selector ? selector_x() : player_x(), y = selector ? selector_y() : player_y();
    approach(x, target_x(), speed());
    approach(y, target_y(), speed());
    arrived() = x == target_x() && y == target_y() ? 0xff : 0;
}

void Game::animate_walk() const {
    if (--walk_delay())
        return;
    walk_delay() = 4;
    if (platform_direction())
        walk_frame() = 0;
    const uint8_t direction = walk_direction();
    if (!direction) {
        player_sprite() = 0x38; // standing
        return;
    }
    const uint8_t* const frames = direction & 1   ? walk_up_sprites()
                                  : direction & 2 ? walk_down_sprites()
                                  : direction & 4 ? walk_left_sprites()
                                                  : walk_right_sprites();
    const uint8_t frame = walk_frame();
    player_sprite() = frames[frame];
    walk_frame() = frame + 1 < 8 ? frame + 1 : 0;
}

void Game::show_selector() const {
    speed() = 2;
    selector_sprite() = 0x62;
    platform_waits() = 0;
}

void Game::draw_score() const {
    for (int n = 0; n < 6; ++n)
        screen.draw_hud_digit(score()[5 - n], 0x60 + 8 * n);
}

void Game::draw_inventory() const {
    screen.draw_hud_digit(std::min<uint8_t>(keys_b(), 9), 0x101);
    screen.draw_hud_digit(std::min<uint8_t>(keys_a(), 9), 0x133);
}

void Game::number_to_glyphs(uint8_t value) const {
    const unsigned zero = 0x1a; // the digit 0 in the sprite font
    number_glyphs()[0] = uint8_t(zero + value / 100);
    number_glyphs()[1] = uint8_t(zero + value % 100 / 10);
    number_glyphs()[2] = uint8_t(zero + value % 10);
}
} // namespace stone
