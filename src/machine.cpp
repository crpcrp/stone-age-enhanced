#include "machine.hpp"
#include "game.hpp"
#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <thread>

namespace stone {
#include "game_data.inc"
// Readable code may wait for ticks in the middle of a routine, so it runs on a
// thread of its own. That thread and the one calling tick() take turns: only
// one of them ever runs.
struct Machine::Turns {
    std::thread thread;
    std::mutex mutex;
    std::condition_variable changed;
    bool routine_runs = false, done = false, abandoned = false;
    uint16_t next = 0; // where the code continues after the routine
    std::exception_ptr failure;
    // Gives the turn to the other side and waits for it to come back.
    void pass(bool to_routine) {
        std::unique_lock<std::mutex> lock(mutex);
        routine_runs = to_routine;
        changed.notify_all();
        changed.wait(lock, [&] { return routine_runs != to_routine; });
    }
    ~Turns() {
        if (!done) { // the machine goes away in the middle of the routine
            abandoned = true;
            pass(true);
        }
        thread.join();
    }
};
namespace {
struct Abandoned {}; // thrown through a waiting routine to end its thread
} // namespace
Machine::Machine() : memory(0x110000) {
    // Records of offset, length, bytes; relocated for load_segment by the generator.
    for (const uint8_t* p = program_data;;) {
        const unsigned offset = p[0] | (p[1] << 8) | (p[2] << 16) | (unsigned(p[3]) << 24);
        const unsigned length = p[4] | (p[5] << 8);
        if (!length)
            break;
        std::copy(p + 6, p + 6 + length, memory.begin() + load_segment * 16u + offset);
        p += 6 + length;
    }
    cs = load_segment;
    ds = data_segment;
    es = uint16_t(load_segment - 0x10);
    ss = uint16_t(load_segment + program_stack_segment);
    sp = uint16_t(program_stack_pointer);
    video_mode();
}
Machine::Machine(const Bytes& exe) : memory(0x110000) {
    if (exe.size() != 452788)
        throw std::runtime_error("Unsupported Stone Age executable");
    auto word = [&](std::size_t p) {
        return unsigned(exe[p]) | (unsigned(exe[p + 1]) << 8);
    };
    // MZ load: the image follows the 512-byte header; each relocation adds the
    // load segment to a segment word. Buffers the game addresses through its
    // static far pointers lie inside this image.
    std::copy(exe.begin() + 512, exe.end(), memory.begin() + load_segment * 16u);
    for (unsigned n = 0; n < word(6); ++n) {
        std::size_t at =
            load_segment * 16u + word(word(24) + n * 4 + 2) * 16u + word(word(24) + n * 4);
        unsigned value = (memory[at] | (unsigned(memory[at + 1]) << 8)) + load_segment;
        memory[at] = uint8_t(value);
        memory[at + 1] = uint8_t(value >> 8);
    }
    cs = load_segment;
    ds = data_segment;
    es = uint16_t(load_segment - 0x10);
    ss = uint16_t(load_segment + word(14));
    sp = uint16_t(word(16));
    video_mode();
}
void Machine::video_mode() {
    // Register state BIOS mode 13h leaves behind, as far as the game reads it.
    vga = Vga();
    for (auto& plane : vga.planes)
        plane.assign(65536, 0);
    vga.sequencer[2] = 0x0f;
    vga.sequencer[4] = 0x0e;
    vga.graphics[5] = 0x40;
    vga.graphics[6] = 0x05;
    vga.crtc[0x07] = 0x1f;
    vga.crtc[0x09] = 0x41;
    vga.crtc[0x14] = 0x40;
    vga.crtc[0x17] = 0xa3;
    vga.crtc[0x18] = 0xff;
    vga.attribute[0x10] = 0x41;
}
bool Machine::supply_resource(uint16_t from_segment, uint16_t to_segment) {
    if (!resource)
        return false;
    const unsigned file_offset = (from_segment - load_segment) * 16u + 512;
    const Bytes* data = resource(file_offset);
    if (!data || data->size() > 65536)
        throw std::runtime_error("Missing game resource at file offset " +
                                 std::to_string(file_offset));
    std::copy(data->begin(), data->end(), memory.begin() + to_segment * 16u);
    return true;
}
Bytes Machine::unpack(unsigned file_offset) {
    const unsigned at = load_segment * 16u + file_offset - 512;
    const unsigned size = (unsigned(memory.at(at + 8)) << 24) | (memory[at + 9] << 16) |
                          (memory[at + 10] << 8) | memory[at + 11];
    if (file_offset % 16 || std::string(memory.begin() + at, memory.begin() + at + 4) != "ICE!" ||
        size > 65536)
        throw std::runtime_error("Not a packed resource");
    const uint16_t free_segment = 0x9000; // free memory above the program
    Game(*this).unpack_at(uint16_t(at / 16), free_segment);
    return Bytes(memory.begin() + free_segment * 16u, memory.begin() + free_segment * 16u + size);
}
void Machine::start() {
    if (!finished())
        throw std::runtime_error("The game started while it runs");
    ip = waiting_address;
    begin_thread([this] {
        Game(*this).run();
        return stop_address; // CS:0cd3: INT 21h/4Ch
    });
}
void Machine::tick(uint8_t scan) {
    if (finished())
        return;
    if (keyboard_installed)
        GameNames(*this).keyboard_scan() = scan; // as CS:092c, the INT 9 handler, does
    if (lifted)
        (this->*lifted)();
    else if (take_turns() == stop_address)
        ip = stop_address;
}
void Machine::begin_thread(std::function<uint16_t()> body) {
    waiting.turns = std::make_shared<Turns>();
    Turns* const turns = waiting.turns.get();
    turns->thread = std::thread([body, turns] {
        {
            std::unique_lock<std::mutex> lock(turns->mutex);
            turns->changed.wait(lock, [&] { return turns->routine_runs; });
        }
        try {
            if (!turns->abandoned)
                turns->next = body();
        } catch (const Abandoned&) {
        } catch (...) {
            turns->failure = std::current_exception();
        }
        std::lock_guard<std::mutex> lock(turns->mutex);
        turns->done = true;
        turns->routine_runs = false;
        turns->changed.notify_all();
    });
}
// Lets the waiting routine run until it waits for the next tick or ends.
uint16_t Machine::take_turns() {
    Turns& turns = *waiting.turns;
    turns.pass(true);
    if (!turns.done)
        return waiting_address;
    const uint16_t next = turns.next;
    const std::exception_ptr failure = turns.failure;
    waiting.turns.reset();
    if (failure)
        std::rethrow_exception(failure);
    return next;
}
// Readable code waits: tick() returns, and the next tick() goes on from here.
void Machine::give_turn_back() {
    Turns* const turns = waiting.turns.get();
    if (!turns || !turns->routine_runs)
        throw std::runtime_error("Readable code waited outside its thread");
    turns->pass(false);
    if (turns->abandoned)
        throw Abandoned();
}
void Machine::wait_tick() {
    in(0x3da); // the original polls the retrace bit here, which this does not have
    give_turn_back();
}
uint8_t Machine::read_character() {
    while (characters.empty())
        give_turn_back();
    const uint8_t character = characters.front();
    characters.erase(characters.begin());
    return character;
}
Image Machine::frame() const {
    Image image;
    image.width = 320;
    image.height = 200;
    displayed(image.indices);
    image.rgb.resize(192000);
    image.palette = vga.dac;
    for (unsigned n = 0; n < 64000; ++n)
        for (unsigned c = 0; c < 3; ++c) {
            const unsigned six = vga.dac[image.indices[n] * 3 + c];
            image.rgb[n * 3 + c] = uint8_t((six << 2) | (six >> 4));
        }
    return image;
}
void Machine::displayed(Bytes& indices) const {
    indices.resize(64000);
    const unsigned start = (unsigned(vga.crtc[0x0c]) << 8) | vga.crtc[0x0d];
    // Scan lines are doubled; rows below the line compare restart at offset 0.
    const unsigned compare =
        vga.crtc[0x18] | ((vga.crtc[0x07] & 0x10u) << 4) | ((vga.crtc[0x09] & 0x40u) << 3);
    const unsigned split = (compare + 1) / 2;
    for (unsigned y = 0; y < 200; ++y) {
        const unsigned row = y < split ? start + y * 80 : (y - split) * 80;
        uint8_t* out = &indices[y * 320];
        for (unsigned column = 0; column < 80; ++column, out += 4) {
            const unsigned at = (row + column) & 65535; // four pixels, one from each plane
            for (unsigned plane = 0; plane < 4; ++plane)
                out[plane] = vga.planes[plane][at];
        }
    }
}
std::string Machine::text(uint16_t segment, uint16_t offset) const {
    std::string result;
    for (unsigned n = 0; n < 128; ++n) {
        char c = char(memory[segment * 16u + uint16_t(offset + n)]);
        if (!c)
            break;
        result += c;
    }
    return result;
}
uint8_t Machine::read8(uint16_t segment, uint16_t offset) {
    const unsigned at = segment * 16u + offset;
    if ((at >> 16) == 0xa) {
        for (unsigned plane = 0; plane < 4; ++plane)
            vga.latch[plane] = vga.planes[plane][at & 65535];
        return vga.latch[vga.graphics[4] & 3];
    }
    return memory[at];
}
uint16_t Machine::read16(uint16_t segment, uint16_t offset) {
    unsigned low = read8(segment, offset);
    return uint16_t(low | (unsigned(read8(segment, uint16_t(offset + 1))) << 8));
}
void Machine::write8(uint16_t segment, uint16_t offset, unsigned value) {
    const unsigned at = segment * 16u + offset;
    if ((at >> 16) == 0xa) {
        const unsigned mode = vga.graphics[5] & 3;
        if (mode > 1)
            throw std::runtime_error("Unsupported VGA write mode");
        for (unsigned plane = 0; plane < 4; ++plane)
            if (vga.sequencer[2] & (1u << plane))
                vga.planes[plane][at & 65535] = mode ? vga.latch[plane] : uint8_t(value);
        return;
    }
    memory[at] = uint8_t(value);
}
uint8_t Machine::in(uint16_t port) {
    switch (port) {
    case 0x388: return adlib_status; // CS:1e69 detection reads the timer flags
    case 0x3c1: return vga.attribute[vga.attribute_index & 31];
    case 0x3c5: return vga.sequencer[vga.sequencer_index & 7];
    case 0x3cf: return vga.graphics[vga.graphics_index & 15];
    case 0x3d5: return vga.crtc[vga.crtc_index & 31];
    case 0x3da:
        // Reading the status resets the attribute flip-flop. The retrace bit
        // alternates so the original polling loops at CS:1df7/1e03 terminate;
        // real time advances only at the CS:1df4 yield.
        vga.attribute_data = false;
        vga.status ^= 8;
        return vga.status;
    default: throw std::runtime_error("Unsupported port input " + std::to_string(port));
    }
}
void Machine::out(uint16_t port, unsigned value) {
    const uint8_t v = uint8_t(value);
    switch (port) {
    case 0x388: adlib_register = v; break;
    case 0x389:
        // An OPL chip is always present: timer 1 started with register 2 = FFh
        // has expired by the time the detection reads the status.
        if (adlib_register == 4)
            adlib_status = (v & 0x80) || v == 0x60 ? 0 : (v & 1) ? 0xc0 : adlib_status;
        break;
    case 0x3c0:
        if (vga.attribute_data)
            vga.attribute[vga.attribute_index & 31] = v;
        else
            vga.attribute_index = v;
        vga.attribute_data = !vga.attribute_data;
        break;
    case 0x3c4: vga.sequencer_index = v; break;
    case 0x3c5: vga.sequencer[vga.sequencer_index & 7] = v; break;
    case 0x3c8: vga.dac_write = v * 3u; break;
    case 0x3c9:
        vga.dac[vga.dac_write] = v & 63;
        vga.dac_write = (vga.dac_write + 1) % 768;
        break;
    case 0x3ce: vga.graphics_index = v; break;
    case 0x3cf: vga.graphics[vga.graphics_index & 15] = v; break;
    case 0x3d4: vga.crtc_index = v; break;
    case 0x3d5: vga.crtc[vga.crtc_index & 31] = v; break;
    default: throw std::runtime_error("Unsupported port output " + std::to_string(port));
    }
}
} // namespace stone
