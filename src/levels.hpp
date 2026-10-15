#pragma once
#include "stone.hpp"
#include <stdexcept>

namespace stone {
// Level sets as text (ENHANCEMENTS.md, E7): levels.txt of an asset set and
// the level packs in levels/ beside the player share one format, described
// at the top of every file written (write_levels). The packer, the player
// and the tools all read it here.

// A level set as the game takes it: the records the resource at file offset
// 157e0 unpacks to and the passwords of DS:1f24.
struct LevelSet {
    static const unsigned most = 100, record_size = 227;
    std::string name;  // the `name` line; empty without one
    unsigned count = 0; // levels in the file, 1-100
    // 100 records of 227 bytes (time, theme, palette, background, flag and
    // 220 tiles), so the game's arithmetic stays the same: past `count`, copies
    // of the last level, which the game never reaches.
    Bytes records;
    Bytes passwords; // three bytes a level, 100 levels
    // The CRC-32 of the records, which tells level sets apart
    // (Game::level_set(); ENHANCEMENTS.md, rule 4).
    uint32_t crc() const { return crc32(records.data(), records.size()); }
};

// What is wrong with a file, and on which line (1 for the whole file).
class LevelError : public std::runtime_error {
public:
    LevelError(const std::string& what, unsigned line) : std::runtime_error(what), line(line) {}
    unsigned line;
};

// A levels file read. `pack` false: levels.txt of an asset set, exactly 100
// levels, each with its password. `pack` true: 1 to 100 levels, an optional
// first line `name <text>`, and a level without a password gets one made
// from the set's CRC. Throws a LevelError naming `path`.
LevelSet read_levels(const Bytes& file, const std::string& path, bool pack);
// The file that reads back as `set`: the format's description, the name if
// there is one, and `count` levels with their passwords.
std::string write_levels(const LevelSet& set);
// The six letters of the password of `level` (0-99).
std::string password_letters(const LevelSet& set, unsigned level);
} // namespace stone
