#pragma once
#include "stone.hpp"
#include <map>

namespace stone {
// The game's asset package: one file in the game's own scrambled format
// (package.cpp). The tools can also read a directory holding the same files
// loose, which is how the assets are edited.
class Package {
public:
    explicit Package(const std::string& path); // a package file or a directory
    bool packed() const { return directory.empty(); }
    bool read(const std::string& name, Bytes& data) const;
    Bytes file(const std::string& name) const; // throws when missing

private:
    std::string directory;
    std::map<std::string, Bytes> files;
};
// Raw deflate data (RFC 1951) of `size` bytes from `at`, unpacked; throws
// when it is damaged. Zip entries use it too (zip.cpp).
Bytes inflate(const Bytes& in, std::size_t at, std::size_t size);
// Write a package file.
void write_package(const std::string& path, const std::map<std::string, Bytes>& files);
} // namespace stone
