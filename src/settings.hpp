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
    // `images`, `maps` or `music` followed by `original` or `free`: which
    // asset set that part comes from when the original game's files are
    // there (Options F2). Without the line, or with any other value, the
    // original.
    enum Part { images, maps, music };
    static const unsigned parts = 3;
    bool original(Part part) const;
    void set_original(Part part, bool on);
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

private:
    // The value of the first line `key ...`; false when there is none.
    bool find(const std::string& key, std::string& value) const;
    void set(const std::string& key, const std::string& value);
    // The level, 1-100, of the line `key <crc>`; 0 when there is none.
    unsigned level(const char* key, uint32_t levels) const;
    std::vector<std::string> lines;
};
} // namespace stone
