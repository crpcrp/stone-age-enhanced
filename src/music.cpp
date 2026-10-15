#include "music.hpp"
#include <algorithm>
#include <stdexcept>
#include <limits>

namespace stone {
#include "music_tables.inc"
Song parse_song(const Bytes& file) {
    if (file.size() < 211 || file[0] != 'A' || file[1] != '.' || file[2] != 'H' || file[3] != '.' ||
        file[4] != 0)
        throw std::runtime_error("Invalid A.H. music signature");
    std::size_t p = 5;
    auto byte = [&]() {
        if (p >= file.size())
            throw std::runtime_error("Truncated A.H. song");
        return file[p++];
    };
    auto word = [&]() {
        unsigned a = byte(), b = byte();
        return uint16_t(a | (b << 8));
    };
    Song song;
    for (int i = 0; i < 99; ++i)
        song.orders.push_back(word());
    int patterns = word();
    song.speed = word();
    song.rhythm = word();
    int instruments = word();
    if (patterns < 1 || patterns > 64 || song.speed < 1 || instruments < 1 || instruments > 64)
        throw std::runtime_error("Unsupported A.H. song dimensions");
    for (int n = 0; n < instruments; ++n) {
        std::array<uint8_t, 69> instrument;
        for (auto& b : instrument)
            b = byte();
        song.instruments.push_back(instrument);
    }
    for (int n = 0; n < patterns; ++n) {
        MusicPattern pattern{};
        for (auto& e : pattern.events)
            e = {0, 0, 127, 32};
        for (;;) {
            auto row = byte();
            if (row == 255)
                break;
            auto channel = byte();
            if (row >= 64 || channel >= 12)
                throw std::runtime_error("Invalid A.H. pattern event");
            auto& event = pattern.events[row * 12 + channel];
            event = {byte(), byte(), byte(), byte()};
        }
        pattern.rows = 64;
        for (int y = 0; y < 64; ++y)
            if (pattern.events[y * 12].note == 1) {
                pattern.rows = y;
                break;
            }
        song.patterns.push_back(pattern);
    }
    if (p != file.size())
        throw std::runtime_error("Unexpected bytes after A.H. patterns");
    return song;
}
uint8_t MusicDriver::read8(uint16_t seg, uint16_t p) const {
    return memory.at(unsigned(seg) * 16 + p);
}
uint16_t MusicDriver::read16(uint16_t seg, uint16_t p) const {
    return uint16_t(read8(seg, p) | (unsigned(read8(seg, uint16_t(p + 1))) << 8));
}
void MusicDriver::write8(uint16_t seg, uint16_t p, unsigned v) {
    memory.at(unsigned(seg) * 16 + p) = uint8_t(v);
}
void MusicDriver::write16(uint16_t seg, uint16_t p, unsigned v) {
    write8(seg, p, v);
    write8(seg, uint16_t(p + 1), v >> 8);
}
void MusicDriver::step() {
    if (++instruction_count > 200000)
        throw std::runtime_error("Music instruction budget exceeded");
}
void MusicDriver::push(unsigned v) {
    sp = uint16_t(sp - 2);
    write16(ss, sp, v);
}
uint16_t MusicDriver::pop() {
    auto v = read16(ss, sp);
    sp = uint16_t(sp + 2);
    return v;
}
unsigned MusicDriver::alu(unsigned a, unsigned b, int bits, Op op) {
    unsigned mask = bits == 8 ? 255 : 65535, sign = 1u << (bits - 1);
    a &= mask;
    b &= mask;
    unsigned r;
    const unsigned carry = cf;
    if (op == Op::ADD || op == Op::ADC) {
        r = a + b + (op == Op::ADC ? carry : 0);
        cf = r > mask;
        of = ((~(a ^ b) & (a ^ r)) & sign) != 0;
        af = (a ^ b ^ r) & 16;
    } else if (op == Op::SUB || op == Op::SBB) {
        const unsigned rhs = b + (op == Op::SBB ? carry : 0);
        r = a - rhs;
        cf = a < rhs;
        of = (((a ^ b) & (a ^ r)) & sign) != 0;
        af = (a ^ b ^ r) & 16;
    } else {
        r = op == Op::AND ? a & b : op == Op::OR ? a | b : a ^ b;
        cf = of = false;
    }
    r &= mask;
    zf = r == 0;
    sf = r & sign;
    unsigned parity = r & 255;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    pf = (parity & 1) == 0;
    return r;
}
unsigned MusicDriver::incdec(unsigned v, int bits, bool decrement) {
    bool saved = cf;
    auto r = alu(v, 1, bits, decrement ? Op::SUB : Op::ADD);
    cf = saved;
    return r;
}
unsigned MusicDriver::shift(unsigned v, int bits, Op op, unsigned count) {
    unsigned mask = bits == 8 ? 255 : 65535, sign = 1u << (bits - 1);
    for (unsigned i = 0; i < (count & 31); ++i) {
        bool old = cf;
        if (op == Op::SHR || op == Op::RCR) {
            cf = v & 1;
            of = op == Op::RCR ? old != bool(v & sign) : bool(v & sign);
            v = (v >> 1) | (op == Op::RCR && old ? sign : 0);
        } else {
            cf = v & sign;
            v = ((v << 1) | (op == Op::RCL ? unsigned(old) : 0)) & mask;
            of = bool(v & sign) != cf;
        }
        if (op != Op::RCR && op != Op::RCL) {
            zf = v == 0;
            sf = v & sign;
            unsigned parity = v & 255;
            parity ^= parity >> 4;
            parity ^= parity >> 2;
            parity ^= parity >> 1;
            pf = (parity & 1) == 0;
        }
    }
    return v;
}
void MusicDriver::math(unsigned v, int bits, Math op) {
    if (bits != 16)
        throw std::runtime_error("Unsupported FM arithmetic width");
    const auto signed_word = [](unsigned word) {
        return int(word & 0x7fff) - int(word & 0x8000);
    };
    if (op == Math::MUL || op == Math::IMUL) {
        int64_t r = op == Math::IMUL ? int64_t(signed_word(ax)) * signed_word(v)
                                     : int64_t(ax) * uint16_t(v);
        ax = uint16_t(r);
        dx = uint16_t(uint64_t(r) >> 16);
        cf = of = op == Math::IMUL ? r != signed_word(ax) : dx != 0;
    } else {
        if (!v)
            throw std::runtime_error("FM sequencer division by zero");
        if (op == Math::IDIV) {
            const int64_t n = int64_t(signed_word(dx)) * 65536 + ax;
            int divisor = signed_word(v);
            int64_t q = n / divisor;
            if (q < -32768 || q > 32767)
                throw std::runtime_error("FM signed division overflow");
            dx = uint16_t(int64_t(n) % divisor);
            ax = uint16_t(q);
        } else {
            unsigned n = (unsigned(dx) << 16) | ax;
            auto q = n / v;
            if (q > 65535)
                throw std::runtime_error("FM division overflow");
            dx = uint16_t(n % v);
            ax = uint16_t(q);
        }
    }
}
void MusicDriver::adapter(uint16_t address) {
    if (address == 0x2928) {
        writes.push_back(
            {uint8_t(read16(ss, uint16_t(sp + 4))), uint8_t(read16(ss, uint16_t(sp + 6)))});
    } else {
        auto argument = [&](unsigned n) {
            return read16(ss, uint16_t(sp + 4 + n * 2));
        };
        auto flags = [&]() {
            return uint16_t(2 | cf | (pf << 2) | (af << 4) | (zf << 6) | (sf << 7) | (of << 11) |
                            api_flags_extra);
        };
        // PC-speaker and PIT port writes (61h/42h/43h) have no host device; only the
        // register results the lifted callers read are kept.
        if (address == 0xf86) {
            auto vector = host_vectors[argument(0) & 255];
            bx = uint16_t(0x3500 | (argument(0) & 255));
            ax = vector.first;
            dx = es = vector.second;
        } else if (address == 0xf9c) {
            ax = uint16_t(0x2500 | (argument(0) & 255));
            dx = argument(1);
            host_vectors[argument(0) & 255] = {dx, argument(2)};
        } else if (address == 0x1ca4) {
            ax = uint16_t(0x1500 | alu(speaker_control, 0xfc, 8, Op::AND));
            speaker_control = lo(ax);
        } else if (address == 0x1c71) {
            bx = argument(0);
            ax = 0x34dd;
            dx = 0x12;
            alu(dx, bx, 16, Op::SUB);
            if (dx < bx) {
                unsigned dividend = (unsigned(dx) << 16) | ax;
                ax = uint16_t(dividend / bx);
                dx = uint16_t(dividend % bx);
                bx = ax;
                set_lo(ax, speaker_control);
                alu(lo(ax), 3, 8, Op::AND);
                if (!(lo(ax) & 3)) {
                    set_lo(ax, alu(lo(ax), 3, 8, Op::OR));
                    speaker_control = lo(ax);
                }
                set_lo(ax, hi(bx));
            }
        } else if (address == 0xfb4) {
            if (argument(0) != 0x21)
                throw std::runtime_error("Unobserved driver interrupt wrapper");
            uint16_t off = argument(1), seg = argument(2);
            alu(0x21, 0x25, 8, Op::SUB);
            uint16_t result_flags = flags();
            ax = read16(seg, off);
            bx = read16(seg, uint16_t(off + 2));
            cx = read16(seg, uint16_t(off + 4));
            dx = read16(seg, uint16_t(off + 6));
            es = read16(seg, uint16_t(off + 16));
            if (hi(ax) != 0x25)
                throw std::runtime_error("Unobserved driver DOS wrapper function");
            host_vectors[lo(ax)] = {dx, read16(seg, uint16_t(off + 14))};
            write16(seg, uint16_t(off + 8),
                    off); // Original stack trampoline loads BP from its saved DI.
            write16(seg, uint16_t(off + 18), result_flags);
            alu(uint16_t(sp - 0x24), 4, 16, Op::ADD); // Original 104f stack cleanup flags.
        } else
            throw std::runtime_error("Unknown sound adapter");
    }
    pop();
    cs = pop();
}
void MusicDriver::call(uint16_t entry, const std::vector<unsigned>& arguments) {
    instruction_count = 0;
    sp = 0xfffe;
    for (auto i = arguments.rbegin(); i != arguments.rend(); ++i)
        push(*i);
    push(cs);
    push(0xffff);
    execute(entry);
}
MusicDriver::MusicDriver(const Song& song) : memory(0x100000) {
    std::copy(std::begin(music_tables), std::end(music_tables), memory.begin() + 0x14810);
    write16(ds, 0x7c, 1); // Initial caller selects looping; the loader preserves this policy.
    replace_song(song);
}
void MusicDriver::fade_step(unsigned step) {
    write16(ds, 0x1a0, step); // CS:45b7
}
void MusicDriver::fade_out() {
    write16(ds, 0x19e, 1); // CS:459b: the level, and that it rises
    write16(ds, 0x17a, 0);
}
void MusicDriver::reset() {
    writes.clear();
    // CS:4315: neutral pitch bend, channel levels, range and OPL reset.
    write16(ds, 0x17a, 0);
    write16(ds, 0x19e, 0);
    call(0x2993, {});
}
void MusicDriver::replace_song(const Song& song) {
    reset();
    write16(ds, 0x40, song.patterns.size());
    write16(ds, 0x42, 0);
    write16(ds, 0x44, song.rhythm);
    write16(ds, 0x48, song.speed);
    write16(ds, 0x50, 1);
    write16(ds, 0x6a, 0);
    write16(ds, 0x6c, 0);
    write16(ds, 0x16e, 32);
    for (std::size_t n = 0; n < song.orders.size(); ++n)
        write16(ds, uint16_t(0x8a + n * 2), song.orders[n]);
    for (std::size_t n = 0; n < song.instruments.size(); ++n)
        std::copy(song.instruments[n].begin(), song.instruments[n].end(),
                  memory.begin() + 0x14810 + 0x1ea + n * 69);
    for (std::size_t n = 0; n < song.patterns.size(); ++n) {
        uint16_t seg = uint16_t(0x6000 + n * 0xd0);
        write16(ds, uint16_t(0x1430 + n * 4), 0);
        write16(ds, uint16_t(0x1432 + n * 4), seg);
        const auto& pattern = song.patterns[n];
        for (int row = 0; row < 64; ++row)
            for (int channel = 0; channel < 12; ++channel) {
                auto e = pattern.events[row * 12 + channel];
                unsigned p = row * 48 + channel * 4;
                write8(seg, uint16_t(p), e.note);
                write8(seg, uint16_t(p + 1), e.instrument);
                write8(seg, uint16_t(p + 2), e.volume);
                write8(seg, uint16_t(p + 3), e.effect);
            }
        write16(seg, 0xc00, 1);
        write16(seg, 0xc02, pattern.rows);
    }
    // Configure YM3812 waveform mode, then use original rhythm-mode setup.
    call(0x2a3f, {unsigned(song.rhythm)});
    call(0x4150, {});
    timer_handler = host_vectors[8];
}
void MusicDriver::tick() {
    instruction_count = 0;
    writes.clear();
    // A fade-out ends with the driver giving INT 8 back (CS:412b): no ticks
    // reach it until the next song is started.
    if (host_vectors[8] != timer_handler)
        return;
    ax = bx = cx = dx = si = di = bp = 0;
    sp = 0xfffe;
    cs = 0x1292;
    ds = 0x1481;
    ss = 0x9000;
    execute(0x3b98);
}
#include "music_lift.inc"
} // namespace stone
