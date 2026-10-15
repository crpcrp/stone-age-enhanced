#pragma once
#include "stone.hpp"
#include <functional>

namespace stone {
struct MusicEvent {
    uint8_t note, instrument, volume, effect;
};
struct MusicPattern {
    std::array<MusicEvent, 64 * 12> events;
    int rows;
};
struct Song {
    std::vector<uint16_t> orders;
    int speed, rhythm;
    std::vector<std::array<uint8_t, 69>> instruments;
    std::vector<MusicPattern> patterns;
};
Song parse_song(const Bytes& file);
class MusicDriver {
public:
    explicit MusicDriver(const Song& song);
    void replace_song(const Song& song);
    // The driver's functions 1Bh, 19h and 5 (INT 64h), as the game calls them
    // to end a song. The level of the fade is taken off every note's volume
    // and grows by the step each tick; at 80h the driver stops playing.
    void fade_step(unsigned step);
    void fade_out();
    // All channels silent and the fade gone; `writes` holds what that takes.
    void reset();
    void tick();
    // Register/value writes at the AdLib boundary, in original order.
    std::vector<std::pair<uint8_t, uint8_t>> writes;

private:
    friend struct MusicProbe;
    unsigned instruction_count = 0;
    enum class Op { ADD, ADC, SUB, SBB, AND, OR, XOR, SHL, SHR, RCL, RCR };
    enum class Math { MUL, IMUL, DIV, IDIV };
    Bytes memory;
    std::array<std::pair<uint16_t, uint16_t>, 256> host_vectors{};
    std::pair<uint16_t, uint16_t> timer_handler; // INT 8 while a song plays
    uint8_t speaker_control = 0;
    uint16_t api_flags_extra = 0;
    uint16_t ax = 0, bx = 0, cx = 0, dx = 0, si = 0, di = 0, bp = 0, sp = 0xfffe, cs = 0x1292,
             ds = 0x1481, es = 0, ss = 0x9000;
    bool cf = false, zf = false, sf = false, af = false, of = false, pf = false;
    uint8_t read8(uint16_t seg, uint16_t p) const;
    uint16_t read16(uint16_t seg, uint16_t p) const;
    void write8(uint16_t seg, uint16_t p, unsigned value);
    void write16(uint16_t seg, uint16_t p, unsigned value);
    void push(unsigned value);
    uint16_t pop();
    static uint8_t lo(uint16_t v) { return uint8_t(v); }
    static uint8_t hi(uint16_t v) { return uint8_t(v >> 8); }
    static void set_lo(uint16_t& r, unsigned v) { r = uint16_t((r & 0xff00) | (v & 255)); }
    static void set_hi(uint16_t& r, unsigned v) { r = uint16_t((r & 255) | ((v & 255) << 8)); }
    unsigned alu(unsigned a, unsigned b, int bits, Op operation);
    unsigned incdec(unsigned value, int bits, bool decrement);
    unsigned shift(unsigned value, int bits, Op operation, unsigned count);
    void math(unsigned value, int bits, Math operation);
    void execute(uint16_t entry);
    void step(); // runaway guard, once per lifted instruction
    void adapter(uint16_t address);
    void call(uint16_t entry, const std::vector<unsigned>& arguments);
};
} // namespace stone
