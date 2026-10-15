#pragma once
#include "stone.hpp"
#include <map>

namespace stone {
class Machine;
class Package;
struct LevelSet;
// The game's graphics, levels, texts and songs as editable files, in place of the
// packed resources inside the original executable: 8-bit BMP pictures with a
// palette and levels as text; names and layouts are listed in assets.cpp.
// The game itself loads them from one scrambled package file (package.hpp).

// Unpack every resource of the original executable into loose files in
// `directory`, and copy the songs found beside the executable.
void export_assets(const std::string& executable, const std::string& directory);
// The same, written straight into a package file: how the player runs with
// the original game's own graphics, levels and music when it finds them.
void pack_original(const std::string& executable, const std::string& package);
// Check the loose files in `directory` and write them as one package file.
void pack_assets(const std::string& directory, const std::string& package);

// The files read back: offset of the resource in the original executable ->
// the bytes its decoder would have produced.
class Assets {
public:
    explicit Assets(const Package& package); // levels.txt of the package for the maps
    // The levels and passwords of `maps` (a level set, ENHANCEMENTS.md E7),
    // the songs' titles and file names from `music`, everything else from
    // `graphics` (Options F2). The song files themselves are read from
    // `music` by the player.
    Assets(const Package& graphics, const LevelSet& maps, const Package& music);
    const Bytes* find(unsigned file_offset) const;
    // Make a machine built without the executable take its resources from here.
    void install(Machine& machine) const;
    // Program bytes by offset in the original file: the sprite directories the
    // original keeps unpacked in its image, the passwords, texts and colours.
    std::map<unsigned, Bytes> program;

private:
    std::map<unsigned, Bytes> resources;
};

// Throws unless the package reproduces every original resource exactly.
void verify_assets(const std::string& executable, const std::string& package);
} // namespace stone
