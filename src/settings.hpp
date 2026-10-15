#pragma once
#include "stone.hpp"

namespace stone {
// STONEAGE.ENH beside the player: what the enhanced game keeps between runs
// (ENHANCEMENTS.md). Never STONEAGE.HI, whose bytes stay the
// original's. A text file of lines `key value...`; a line this version does
// not know is kept as it is when the file is written again.
class Settings {
public:
    static const char* const file_name;
    Settings() = default; // as a missing file: every default
    explicit Settings(const Bytes& file);
    Bytes bytes() const;
    // `enhanced 1` or `enhanced 0`. Without the line, or with any other
    // value, enhanced mode is on.
    bool enhanced() const;
    void set_enhanced(bool on);
    // `light 1` or `light 0`: LIGHT (menu F6), kept in enhanced mode (E0).
    // Without the line, or with any other value, off.
    bool light() const;
    void set_light(bool on);
    // `images` or `music` followed by `original` or `free`: which asset set
    // that part comes from when the original game's files are there
    // (Options F2). Without the line, or with any other value, the original.
    enum Part { images, music };
    static const unsigned parts = 2;
    bool original(Part part) const;
    void set_original(Part part, bool on);
    // `pack <name>`: the level set played (ENHANCEMENTS.md, E7): `free` or
    // `original` for the maps of an asset set, otherwise the file name of a
    // pack in levels/. Without the line, `maps free` of earlier versions
    // gives `free` and anything else `original`.
    static const char* const free_maps;
    static const char* const original_maps;
    std::string pack() const;
    void set_pack(const std::string& name);
    // `display sharp|crt|smooth`: how the host shows the picture in enhanced
    // mode (ENHANCEMENTS.md, E5). Without the line, or with any other
    // value, sharp.
    enum Display { sharp, crt, smooth };
    static const unsigned displays = 3;
    static const char* display_name(Display display);
    Display display() const;
    void set_display(Display display);
    // `reached <crc> <level>`: the furthest level, 1-100, ever started of the
    // level set whose records have this CRC-32 (ENHANCEMENTS.md, E1). 1
    // without the line or with a bad value.
    unsigned reached(uint32_t levels) const;
    void set_reached(uint32_t levels, unsigned level);
    // `last <crc> <level>`: the level, 1-100, last started of that level set
    // (ENHANCEMENTS.md, E2). 0 without the line or with a bad value.
    unsigned last(uint32_t levels) const;
    void set_last(uint32_t levels, unsigned level);
    // `best <crc> <level> <ticks> <points>`: the best result on a map of that
    // level set (ENHANCEMENTS.md, E3), the fewest ticks and the most points,
    // each from any run that cleared it. A map with one has been cleared.
    struct Best {
        unsigned ticks = 0, points = 0; // ticks 0: none
        bool cleared() const { return ticks != 0; }
    };
    Best best(uint32_t levels, unsigned level) const;
    // A run that cleared the map, kept where it beats the best. Returns which
    // parts it beat: `faster`, `higher`, both or 0.
    static const unsigned faster = 1, higher = 2;
    unsigned improve_best(uint32_t levels, unsigned level, unsigned ticks, unsigned points);
    // `par <crc> <level> <ticks>|unsolvable|unknown`: what the solver found
    // for a map of that level set (ENHANCEMENTS.md, E8): the fewest ticks to
    // clear it, that it cannot be cleared in its time, or that the search
    // stopped at its limit. `none` without the line or with a bad value.
    struct Par {
        enum Kind { none, solvable, unsolvable, unknown };
        Kind kind = none;
        unsigned ticks = 0; // solvable only
    };
    Par par(uint32_t levels, unsigned level) const;
    void set_par(uint32_t levels, unsigned level, const Par& par);
    // `stats <crc> <level> <attempts> <clears> <lives lost> <give-ups> <ticks>`:
    // what has happened on a map of that level set (ENHANCEMENTS.md, E4).
    // Every attempt ends in one of the other three. All 0 without the line
    // or with a bad value.
    struct MapStats {
        uint64_t attempts = 0, clears = 0, lives_lost = 0, give_ups = 0, ticks = 0;
    };
    MapStats map_stats(uint32_t levels, unsigned level) const;
    void set_map_stats(uint32_t levels, unsigned level, const MapStats& stats);
    // `totals <ticks> <games> <cleared> <steps> <hourglasses> <keys> <gates>
    // <teleports> <platforms>`: the same over every level set (E4): the
    // ticks played, games started, levels cleared, steps walked, hourglasses
    // taken, keys collected, gates opened, teleports used and platforms set
    // off. All 0 without the line or with a bad value.
    struct Totals {
        uint64_t ticks = 0, games = 0, cleared = 0, steps = 0, hourglasses = 0, keys = 0,
                 gates = 0, teleports = 0, platforms = 0;
    };
    Totals totals() const;
    void set_totals(const Totals& totals);

private:
    // The value of the first line `key ...`; false when there is none.
    bool find(const std::string& key, std::string& value) const;
    void set(const std::string& key, const std::string& value);
    // The level, 1-100, of the line `key <crc>`; 0 when there is none.
    unsigned level(const char* key, uint32_t levels) const;
    std::vector<std::string> lines;
};
} // namespace stone
