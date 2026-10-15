#pragma once
#include "stone.hpp"
#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace stone {
// A little-endian 16-bit variable in the machine's memory, or with [] a table of them.
struct Word {
    uint8_t* at;
    operator uint16_t() const { return uint16_t(at[0] | (at[1] << 8)); }
    const Word& operator=(unsigned value) const {
        at[0] = uint8_t(value);
        at[1] = uint8_t(value >> 8);
        return *this;
    }
    const Word& operator=(const Word& other) const { return *this = uint16_t(other); }
    Word operator[](unsigned index) const { return Word{at + 2 * index}; }
};
// Real-mode machine the original game runs on: the program's data in 1 MiB of
// memory, CPU registers and a planar VGA. The player runs the readable
// program of game.hpp on it (start()). The members marked below as the
// lifted original are defined only in the tests, which run the original
// translated instruction by instruction to check the readable program; that
// code is not part of this repository.
// The host touches it at these boundaries only: the keyboard scan byte at
// CS:0906, the retrace wait at CS:1df4 (one 70 Hz tick), DOS character input
// and files (INT 21h), the DOS clock that seeds the RNG, and the music
// driver (INT 64h).
class Machine {
public:
    // The game without the original executable: its data tables are compiled
    // in (game_data.inc); graphics and levels come from `resource`.
    Machine();
    // The original executable loaded as DOS would, packed resources included.
    explicit Machine(const Bytes& exe);
    // Unpacked contents of the resource stored at this offset of the original
    // file, or null. When set, the game's ICE decoder (CS:173b) is not run.
    std::function<const Bytes*(unsigned)> resource;
    // Copy the host's resource for the packed one at from_segment:0 to
    // to_segment:0. False when there is no `resource`.
    bool supply_resource(uint16_t from_segment, uint16_t to_segment);
    // A packed resource of the original file unpacked (needs the executable).
    Bytes unpack(unsigned file_offset);
    // Overwrite program bytes, addressed by their offset in the original file.
    void write_image(unsigned file_offset, const Bytes& bytes) {
        std::copy(bytes.begin(), bytes.end(),
                  memory.begin() + load_segment * 16u + file_offset - 512);
    }
    // Run the game from its DOS entry point (Game::run, CS:0000) until it
    // ends through INT 21h/4Ch. It runs inside tick().
    void start();
    bool finished() const { return ip == stop_address; }
    // Latch a scan byte as the INT 9 handler would, then run until the code
    // waits for the next vertical retrace or the game ends.
    void tick(uint8_t scan);
    // What the VGA displays now: 320x200 indices, six-bit DAC palette, RGB.
    Image frame() const;
    // Only the displayed indices, without the RGB.
    void displayed(Bytes& indices) const;
    uint8_t& data(uint16_t offset) { return memory[data_segment * 16u + offset]; }
    uint8_t& code(uint16_t offset) { return memory[load_segment * 16u + offset]; }
    const std::array<uint8_t, 768>& dac() const { return vga.dac; }
    // INT 64h: function in AX, arguments in BX/CX/DX; BX:CX may be a far pointer.
    std::function<void(Machine&)> driver;
    // DOS files by name: load returns false when the file does not exist.
    std::function<bool(const std::string&, Bytes&)> load_file;
    std::function<void(const std::string&, const Bytes&)> save_file;
    unsigned seconds = 0; // DOS clock seconds read once at startup (RNG seed)
    // Enhanced mode (ENHANCEMENTS.md, E0): off, the game is the original's.
    // The player turns it on from the saved setting; the tests leave it off.
    bool enhanced = false;
    // While the game has its INT 9 handler removed (password and name entry)
    // it reads characters through DOS instead of scan bytes. Extended keys
    // arrive as 0 followed by the scan code, as DOS delivers them.
    bool reads_characters() const { return !keyboard_installed; }
    void type(uint8_t character) { characters.push_back(character); }
    std::string text(uint16_t segment, uint16_t offset) const;
    uint16_t ax = 0, bx = 0, cx = 0, dx = 0, si = 0, di = 0, bp = 0, sp = 0;
    uint16_t cs = 0, ds = 0, es = 0, ss = 0;
    bool cf = false, zf = false, sf = false, of = false, pf = false, af = false, df = false,
         intf = true;

    // --- The lifted original (tests only, not in this repository) ---
    // Call an original near routine, lifted. It runs inside tick(), until it
    // returns. start_lifted(0) is the DOS entry point.
    void start_lifted(uint16_t entry);
    // The ICE decoder CS:173b, lifted, on a resource of the original file.
    Bytes unpack_lifted(unsigned file_offset);
    // A lifted routine that has a readable version (src/game.hpp), which runs
    // in its place (the list is READABLE in research/tools/lift_game.py). `results`
    // names the registers and flags its callers read, as in
    // registers_differing(). A `stretch` is part of a larger routine: its
    // run() returns the address to continue at, and it ends at one of the
    // exits listed for it. Readable code may let ticks pass before it ends, as
    // lifted code does: tick() returns in the middle of it and the next
    // tick() goes on from there.
    struct Routine {
        uint16_t entry;
        const char *name, *results;
        bool stretch;
        uint16_t (*run)(Machine&);
    };
    static const std::vector<Routine> routines;
    bool lifted_only = false; // run the lifted originals instead
    // Called before such a routine runs. From here `replay_tests --routines`
    // compares the two versions: only() makes this machine a copy of `state`
    // that runs just that routine, lifted or readable, through tick().
    std::function<void(Machine&, const Routine&)> before_routine;
    void only(const Machine& state, const Routine& routine, bool lifted);
    // al ah bl bh cl ch dl dh si di bp es cf zf sf of pf af df, where different.
    std::vector<std::string> registers_differing(const Machine& other) const;
    // What two machines that ran the same routine do not agree on: memory
    // outside the free stack, the VGA, or one of `results`. Empty when equal.
    // A routine cut short while it waits has its own stack below `entry_sp`,
    // the stack pointer it started with; that and the registers are left out.
    std::string difference(const Machine& other, const std::string& results,
                           uint16_t entry_sp) const;

    // What readable code does in place of the lifted code's port instructions,
    // segmented memory accesses (A000 is the VGA) and DOS and BIOS calls.
    uint8_t in(uint16_t port);
    void out(uint16_t port, unsigned value);
    uint8_t read8(uint16_t segment, uint16_t offset);
    uint16_t read16(uint16_t segment, uint16_t offset);
    void write8(uint16_t segment, uint16_t offset, unsigned value);
    uint8_t* at(uint16_t segment, uint16_t offset) { return &memory[segment * 16u + offset]; }
    void wait_tick();         // CS:1df4: until the next vertical retrace
    uint8_t read_character(); // INT 21h/07h: waits until a character is typed
    void video_mode();        // INT 10h: BIOS mode 13h, everything black
    // INT 21h/25h: the game's INT 9 handler is installed, or the DOS one.
    void connect_keyboard(bool connected) { keyboard_installed = connected; }
    uint8_t al() const { return uint8_t(ax); }
    uint8_t dl() const { return uint8_t(dx); }
    uint8_t dh() const { return uint8_t(dx >> 8); }
    void set_al(unsigned value) { set_lo(ax, value); }

    static constexpr uint16_t load_segment = 0x1000, data_segment = load_segment + 0x31b;

private:
    static constexpr uint16_t stop_address = 0xfff0, waiting_address = 0xfff1;
    enum class Op { ADD, ADC, SUB, SBB, AND, OR, XOR, SHL, SHR, SAR, ROL, ROR, RCL, RCR };
    enum class Math { MUL, IMUL, DIV, IDIV };
    struct Vga {
        std::array<Bytes, 4> planes;
        std::array<uint8_t, 4> latch{};
        std::array<uint8_t, 8> sequencer{};
        std::array<uint8_t, 16> graphics{};
        std::array<uint8_t, 32> crtc{}, attribute{};
        std::array<uint8_t, 768> dac{};
        unsigned sequencer_index = 0, graphics_index = 0, crtc_index = 0, attribute_index = 0;
        unsigned dac_write = 0;
        bool attribute_data = false;
        uint8_t status = 0;
    } vga;
    Bytes memory;
    uint16_t ip = stop_address;
    bool blocked = false, pass_wait = false, keyboard_installed = false;
    std::vector<uint8_t> characters;
    Bytes file;
    std::string file_name;
    std::size_t file_position = 0;
    uint8_t adlib_register = 0, adlib_status = 0;
    unsigned instruction_count = 0;
    // The lifted code runs in tick() while this is set (start_lifted()).
    void (Machine::*lifted)() = nullptr;
    void tick_lifted();
    void execute(uint16_t entry);
    void step();
    bool readable(const Routine& routine);
    uint16_t run(const Routine& routine);
    // Starts `body` on the thread readable code runs on; take_turns() lets it
    // run. It returns where lifted code continues after it.
    void begin_thread(std::function<uint16_t()> body);
    uint16_t take_turns();
    void give_turn_back();
    uint16_t continue_at = 0, left_at = 0; // after a readable stretch; where only() stopped
    unsigned leaving = 0; // only(): the stretch (index + 1) whose exits end the run
    void write16(uint16_t segment, uint16_t offset, unsigned value);
    void push(unsigned value);
    uint16_t pop();
    uint16_t flags() const;
    void set_flags(uint16_t value);
    static uint8_t lo(uint16_t v) { return uint8_t(v); }
    static uint8_t hi(uint16_t v) { return uint8_t(v >> 8); }
    static void set_lo(uint16_t& r, unsigned v) { r = uint16_t((r & 0xff00) | (v & 255)); }
    static void set_hi(uint16_t& r, unsigned v) { r = uint16_t((r & 255) | ((v & 255) << 8)); }
    void result_flags(unsigned result, int bits);
    unsigned alu(unsigned a, unsigned b, int bits, Op operation);
    unsigned incdec(unsigned value, int bits, bool decrement);
    unsigned shift(unsigned value, int bits, Op operation, unsigned count);
    void math(unsigned value, int bits, Math operation);
    void movs(int size, uint16_t source_segment, bool repeat);
    void stos(int size, bool repeat);
    void lods(int size, uint16_t source_segment);
    void decimal_adjust(bool subtract);
    bool unpack_resource(); // supply_resource() at CS:173b, with the registers the decoder leaves
    // False when the routine cannot continue yet; ip then says where to resume.
    bool interrupt(unsigned number, uint16_t address);
    // The thread readable code runs on (machine.cpp). A copy of the machine
    // does not get it. Last member: it ends before the rest does.
    struct Turns;
    struct Waiting {
        std::shared_ptr<Turns> turns;
        Waiting() = default;
        Waiting(const Waiting&) {}
        Waiting& operator=(const Waiting&) {
            turns.reset(); // a routine of its own that was waiting ends
            return *this;
        }
    } waiting;
};
} // namespace stone
