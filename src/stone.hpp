#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace stone {
using Bytes = std::vector<std::uint8_t>;
// A displayed frame: palette indices, the six-bit DAC palette and 24-bit RGB.
struct Image {
    int width = 0, height = 0;
    Bytes rgb, indices;
    std::array<uint8_t, 768> palette{};
};
Bytes read_file(const std::string& path);
void write_file(const std::string& path, const Bytes& bytes);
void write_bmp(const std::string& path, const Image& image);
std::uint32_t crc32(const std::uint8_t* data, std::size_t size);
// SHA-256 (FIPS 180-4) in lowercase hex: how the web page knows the original
// executable from a zip (play.cpp).
std::string sha256(const Bytes& data);
} // namespace stone
