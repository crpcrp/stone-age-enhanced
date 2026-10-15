#pragma once
#include "stone.hpp"
#include <map>

namespace stone {
// Zip archives (PKWARE's APPNOTE), as much as the web page needs: the saves it
// exports and imports, and the original game's files a player adds
// (BUILDING.md, Web version). Stored and deflated entries are read; anything
// else, a damaged zip or one that is not a zip, throws std::runtime_error.
// Entry names are kept as the zip gives them, folders included; folder
// entries themselves are left out.
std::map<std::string, Bytes> read_zip(const Bytes& zip);
// A zip of the files, stored without compression.
Bytes write_zip(const std::map<std::string, Bytes>& files);
} // namespace stone
