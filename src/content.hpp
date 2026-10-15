#pragma once
#include "stone.hpp"
#include <map>

namespace stone {
class Package;
// The original's own texts and colours, which the program image holds rather
// than its packed resources (the content ranges of research/tools/data_map.py),
// as two editable asset files: texts.txt (credits, jukebox, song files, score
// screens, the pages between levels) and colours.txt (tile themes, level
// palettes, page letters). Each section fills a fixed area of the data segment;
// the format is described at the top of each file.

// Both files, made from the original executable.
std::map<std::string, Bytes> content_files(const Bytes& exe);
// The program bytes the files in `package` give, by offset in the original
// file. The program holds none of them: both files are required.
void content_program(const Package& package, std::map<unsigned, Bytes>& program);
// Of those, the jukebox texts and the song file names of `package`, over the
// ones in `program`: the songs of another asset set than the rest.
void song_program(const Package& package, std::map<unsigned, Bytes>& program);
// The eight song file names the game opens (DS:15ec): those of an executable,
// or those of texts.txt in `package`.
std::vector<std::string> song_files(const Bytes& exe);
std::vector<std::string> song_files(const Package& package);
} // namespace stone
