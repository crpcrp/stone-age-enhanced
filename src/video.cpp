// What the game shows: pages of video memory, sprites, colours, text.
#include "game.hpp"
#include <algorithm>

namespace stone {
namespace {
const uint16_t video = 0xa000; // segment of the VGA's memory
const unsigned line = 80;      // bytes of a plane for one line of 320 pixels
} // namespace

uint16_t Random::operator()(uint16_t limit) const {
    // Four words, each added to the next with the carry of the addition before.
    const Word state = names.random_state();
    unsigned sum = limit + state[0];
    const uint16_t first = uint16_t(sum);
    for (int n = 1; n < 4; ++n) {
        sum = state[n] + uint16_t(sum) + (sum >> 16);
        state[n] = sum;
    }
    state[0] = first + uint16_t(sum) + (sum >> 16);
    return uint16_t(unsigned(state[0]) * limit >> 16);
}

void Screen::write_planes(unsigned planes) const {
    m.out(0x3c4, 2);
    m.out(0x3c5, planes);
}

void Screen::read_plane(unsigned plane) const {
    m.out(0x3ce, 4);
    m.out(0x3cf, plane);
}

void Screen::set_copy_mode(bool on) const {
    m.out(0x3ce, 5);
    m.out(0x3cf, (m.in(0x3cf) & 0xfc) | (on ? 1 : 0));
}

void Screen::set_layout(bool level) const {
    names.page_height() = level ? 178 : 200;
    names.page_bytes() = (level ? 178 : 200) * line;
    black();
    // The planes side by side instead of the one byte a pixel of BIOS mode 13h.
    m.out(0x3c4, 4);
    m.out(0x3c5, m.in(0x3c5) & 0xf7);
    m.out(0x3d4, 0x14);
    m.out(0x3d5, m.in(0x3d5) & 0xbf);
    m.out(0x3d4, 0x17);
    m.out(0x3d5, m.in(0x3d5) | 0x40);
    m.out(0x3d4, 0x13);
    m.out(0x3d5, 0x28);
    // The line from which the display shows the start of video memory again:
    // the strip below the board, or none.
    m.out(0x3d4, 0x18);
    m.out(0x3d5, level ? 0x63 : 0x8f);
    m.out(0x3d4, 7);
    m.out(0x3d5, (m.in(0x3d5) & 0xef) | 0x10);
    m.out(0x3d4, 9);
    m.out(0x3d5, m.in(0x3d5) & 0xbf);
    wait(1);
    m.out(0x3c0, 0x10);
    m.out(0x3c0, m.in(0x3c1) | 0x20);
    m.out(0x3c0, 0x20);
    names.hud_page() = 0;
    names.shown_page() = level ? 22 * line : 0;
    names.draw_page() = 200 * line;
    names.restore_page() = level ? 378 * line : 400 * line;
    names.scenery_page() = level ? 556 * line : 600 * line;
    names.copy_page() = level ? names.scenery_page() : names.restore_page();
    clear_everything();
    flip();
}

void Screen::clear_everything() const {
    write_planes(15);
    for (unsigned at = 0; at < 0x10000; ++at)
        m.write8(video, uint16_t(at), 0);
}

void Screen::wait(unsigned ticks) const {
    for (; ticks; --ticks)
        m.wait_tick();
}

void Screen::flip() const {
    const uint16_t shown = names.draw_page();
    names.draw_page() = names.shown_page();
    names.shown_page() = shown;
    m.in(0x3da); // the original waits here for a retrace to end
    m.out(0x3d4, 0x0c);
    m.out(0x3d5, shown >> 8);
    m.out(0x3d4, 0x0d);
    m.out(0x3d5, shown & 255);
}

void Screen::put_pixels(uint16_t to, uint16_t bytes) const {
    const uint8_t* pixels = work_buffer();
    do {
        for (unsigned plane = 0; plane < 4; ++plane) {
            write_planes(1 << plane);
            m.write8(video, to, *pixels++);
        }
        ++to;
    } while (--bytes);
    write_planes(15);
}

void Screen::get_pixels(uint16_t from, uint16_t bytes) const {
    uint8_t* pixels = work_buffer();
    do {
        for (unsigned plane = 0; plane < 4; ++plane) {
            read_plane(plane);
            *pixels++ = m.read8(video, from);
        }
        ++from;
    } while (--bytes);
}

void Screen::clear(uint16_t at, uint16_t bytes) const {
    for (unsigned n = 0; n < (bytes & ~1u); ++n)
        m.write8(video, uint16_t(at + n), 0);
}

void Screen::copy(uint16_t from, uint16_t to, unsigned bytes) const {
    for (unsigned n = 0; n < bytes; ++n) {
        m.read8(video, uint16_t(from + n));
        m.write8(video, uint16_t(to + n), 0);
    }
}

void Screen::save_page() const {
    names.copy_page() = names.restore_page();
    set_copy_mode(true);
    copy(names.draw_page(), names.copy_page(), names.page_bytes());
    set_copy_mode(false);
}

void Screen::save_scenery() const {
    names.copy_page() = names.scenery_page();
    set_copy_mode(true);
    copy(names.draw_page(), names.copy_page(), names.page_bytes());
    set_copy_mode(false);
}

void Screen::copy_shown_page() const {
    set_copy_mode(true);
    copy(names.shown_page(), names.draw_page(), names.page_bytes());
    set_copy_mode(false);
}

void Screen::copy_rectangle(uint16_t from, uint16_t to, unsigned x, unsigned y, unsigned width,
                            unsigned height) const {
    set_copy_mode(true);
    const uint16_t at = uint16_t(y * line + uint16_t(x) / 4);
    const unsigned bytes = uint16_t((x & 3) + width + 3) / 4;
    for (unsigned row = 0; row < height; ++row)
        copy(uint16_t(from + at + row * line), uint16_t(to + at + row * line), bytes);
    set_copy_mode(false);
}

void Screen::restore(unsigned x, unsigned y, unsigned width, unsigned height) const {
    copy_rectangle(names.copy_page(), names.draw_page(), x, y, width, height);
}

void Screen::save_cell(uint8_t column, uint8_t row) const {
    names.copy_page() = names.restore_page();
    copy_rectangle(names.draw_page(), names.copy_page(), column * 16, row * 16, 16, 16);
}

void Screen::draw_sprite(uint8_t sprite, unsigned x, unsigned y) const {
    uint16_t at = names.draw_page();
    if (uint16_t(y) >= names.page_height()) {
        at = names.hud_page();
        y -= 178;
    }
    // Times 80, as the original does it: also for a y that is no line.
    const uint16_t swapped = uint16_t(y << 8 | uint16_t(y) >> 8);
    at += uint16_t(x) / 4 + swapped / 4 + swapped / 16;
    // A record is the offset of the pixels, then width and height. The pixels
    // are stored column by column. A size of 0 counts as 256.
    const Word records = names.sprite_records(), pixels = names.sprite_pixels();
    const uint16_t record = uint16_t(records + sprite * 4);
    uint16_t source = uint16_t(pixels + m.read16(records[1], record));
    const uint8_t height = m.read8(records[1], uint16_t(record + 3));
    uint8_t columns = m.read8(records[1], uint16_t(record + 2));
    unsigned plane = x & 3;
    do {
        write_planes(1 << plane);
        uint16_t to = at;
        uint8_t rows = height;
        do {
            const uint8_t pixel = m.read8(pixels[1], source++);
            if (pixel)
                m.write8(video, to, pixel);
            to += line;
        } while (--rows);
        if (++plane == 4) {
            plane = 0;
            ++at;
        }
    } while (--columns);
    write_planes(15);
}

void Screen::draw_player() const {
    draw_sprite(names.player_sprite(), names.player_x(), names.player_y());
    draw_eyes();
}

void Screen::draw_eyes() const {
    draw_sprite(eye_sprite(0xd1, 0xb6), 0xd1, 0xb6);
    draw_sprite(eye_sprite(0xd7, 0xb6), 0xd7, 0xb6);
}

uint8_t Screen::eye_sprite(unsigned x, unsigned y) const {
    uint16_t across = uint16_t(names.player_x() + 11 - x),
             down = uint16_t(y + 10 - names.player_y());
    unsigned quadrant = 0; // each has six sprites
    if (across & 0x8000) {
        across = uint16_t(-across);
        quadrant += 12;
    }
    if (down & 0x8000) {
        down = uint16_t(-down);
        quadrant += 6;
    }
    const uint16_t slope = across ? uint16_t(down << 7) / across : 0x7fff;
    unsigned steepness = 0;
    while (slope < names.eye_slopes()[steepness])
        ++steepness;
    return uint8_t(names.eye_sprites()[quadrant + steepness]);
}

void Screen::restore_player() const {
    restore(names.player_drawn_x(), names.player_drawn_y());
    names.player_drawn_x() = names.player_x();
    names.player_drawn_y() = names.player_y();
}

void Screen::restore_selector() const {
    restore(names.selector_drawn_x(), names.selector_drawn_y());
    names.selector_drawn_x() = names.selector_x();
    names.selector_drawn_y() = names.selector_y();
}

void Screen::set_target(const uint8_t* colours, unsigned first, unsigned count) const {
    std::copy(colours, colours + 3 * count, names.palette_target() + 3 * first);
}

void Screen::set_colours(const uint8_t* palette, unsigned first, unsigned last) const {
    m.out(0x3c8, first);
    for (unsigned n = 3 * first; n < 3 * (last + 1); ++n)
        m.out(0x3c9, palette[n]);
}

void Screen::black() const {
    m.out(0x3c8, 0);
    for (int part = 0; part < 8; ++part) {
        wait(1);
        for (int n = 0; n < 32 * 3; ++n)
            m.out(0x3c9, 0);
    }
}

void Screen::fade_in() const {
    for (unsigned level = 0; level <= 256; level += 4)
        show_faded(level);
}

void Screen::fade_out() const {
    for (int level = 256; level >= 0; level -= 4)
        show_faded(level);
}

void Screen::show_faded(unsigned level) const {
    for (unsigned n = 0; n < 64 * 3; ++n)
        names.palette_shown()[n] = uint8_t(names.palette_target()[n] * level >> 8);
    wait(1);
    set_colours(names.palette_shown(), 0, 63);
}

void Screen::show_board_faded(unsigned level) const {
    const uint8_t* const target = names.palette_target();
    uint8_t* const shown = names.palette_shown();
    for (unsigned n = 0; n < 16 * 3; ++n) {
        shown[n] = uint8_t(target[n] * level >> 8);
        shown[16 * 3 + n] = target[16 * 3 + n];
        shown[32 * 3 + n] = uint8_t(target[32 * 3 + n] * level >> 8);
    }
    wait(1);
    set_colours(shown, 0, 47);
}

void Screen::wipe(bool in) const {
    const Random random(m);
    const uint8_t* const strip = work_buffer();
    const auto put = [&](unsigned column, unsigned row) {
        m.out(0x3c5, 1 << (column & 3));
        const unsigned pixel = row * 320 + column;
        m.write8(video, uint16_t(names.hud_page() + pixel / 4), in ? strip[pixel] : 0);
    };
    m.out(0x3c4, 2);
    for (unsigned step = 0; step < 640; ++step) {
        names.wipe_step() = step;
        names.wipe_first_column() = step < 320 ? 0 : step - 320;
        names.wipe_last_column() = step < 320 ? step : 319;
        if (step >= 320) // the column the wipe leaves behind, whole
            for (unsigned row = 0; row < 22; ++row)
                put(step - 320, row);
        // Of every column it is passing over, one pixel in four.
        for (unsigned column = names.wipe_first_column(); column <= names.wipe_last_column();
             ++column) {
            names.wipe_column() = column;
            const unsigned row = random(88);
            if (row < 22)
                put(column, row);
        }
        if (step % 5 == 0) {
            const unsigned level = std::min(std::max(int(step / 5) - 31, 0) * 4, 256);
            show_board_faded(in ? level : 256 - level);
        }
    }
    write_planes(15);
}

void Screen::show_through_buffer() const {
    uint8_t* const picture = work_buffer();
    const unsigned pixels = uint16_t(names.page_bytes() * 4);
    m.out(0x3ce, 4);
    for (unsigned n = 0; n < pixels; ++n)
        if (picture[n] == 2) {
            read_plane(n & 3);
            picture[n] = m.read8(video, uint16_t(names.shown_page() + n / 4));
        }
}

void Screen::swap_with_buffer() const {
    uint8_t* const picture = work_buffer();
    uint16_t pixel = 0;
    for (int tick = 0; tick < 128; ++tick) {
        for (int n = 0; n < 512; ++n) {
            pixel = uint16_t(pixel * 5 + 1); // every pixel once in 65,536 steps
            if (pixel >= 176 * 320)
                continue;
            const uint16_t at = uint16_t(names.shown_page() + pixel / 4);
            read_plane(pixel & 3);
            write_planes(1 << (pixel & 3));
            const uint8_t shown = m.read8(video, at);
            m.write8(video, at, picture[pixel]);
            picture[pixel] = shown;
        }
        wait(1);
    }
    write_planes(15);
}

const uint8_t* Screen::write(const uint8_t* text) const {
    names.text_line_x() = names.text_x();
    for (;;) {
        const uint8_t character = *text++;
        switch (character) {
        case 0: return text;
        case '\n': names.text_x() = names.text_line_x(); break;
        case '\r': names.text_y() = names.text_y() + names.text_line_height(); break;
        case 0xe0: names.text_bold() = 4; break;
        case 0xe2: names.text_bold() = 0; break;
        default: write(character);
        }
    }
}

void Screen::write(uint8_t character) const {
    const uint16_t x = names.text_x(), y = names.text_y();
    names.text_x() = x + 8;
    if (character < ' ')
        return;
    // A glyph is eight columns of sixteen rows, the top one in the highest
    // bit. Rows above the page or below it are left out.
    uint16_t rows = 0xffff;
    const int16_t above = int16_t(-y), below = int16_t(y + 16 - names.page_height());
    if (y & 0x8000) {
        if (above >= 16)
            return;
        rows >>= above & 31;
    } else if (below >= 0) {
        if (below >= 16)
            return;
        rows <<= below;
    }
    names.text_row_mask() = rows;
    const uint8_t* glyph = work_buffer() + (character - ' ') * 16;
    uint16_t at = uint16_t(names.draw_page() + y * line + x / 4);
    unsigned plane = x & 3;
    for (int column = 0; column < 8; ++column, glyph += 2) {
        write_planes(1 << plane);
        uint16_t to = at;
        for (uint16_t bits = (glyph[0] | glyph[1] << 8 | names.text_bold()) & rows; bits;
             bits <<= 1, to += line)
            if (bits & 0x8000)
                m.write8(video, to, names.text_colour());
        if (++plane == 4) {
            plane = 0;
            ++at;
        }
    }
    write_planes(15);
}
} // namespace stone
