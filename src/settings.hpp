#pragma once
#include "stone.hpp"

namespace stone {
// STONEAGE.ENH beside the player: what the enhanced game keeps between runs
// (ENHANCEMENTS.md, E0 to E2). Never STONEAGE.HI, whose bytes stay the
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
    // there (E2). Without the line, or with any other value, the original.
    enum Part { images, maps, music };
    static const unsigned parts = 3;
    bool original(Part part) const;
    void set_original(Part part, bool on);
    // `reached <crc> <level>`: the furthest level, 1-100, ever started of the
    // level set whose records have this CRC-32 (ENHANCEMENTS.md, E1). 1
    // without the line or with a bad value.
    unsigned reached(uint32_t levels) const;
    void set_reached(uint32_t levels, unsigned level);

private:
    // The value of the first line `key ...`; false when there is none.
    bool find(const std::string& key, std::string& value) const;
    void set(const std::string& key, const std::string& value);
    std::vector<std::string> lines;
};
} // namespace stone
