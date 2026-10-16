#include "stone.hpp"
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace stone {
Bytes read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("Cannot read " + path);
    return Bytes(std::istreambuf_iterator<char>(file), {});
}
void write_file(const std::string& path, const Bytes& bytes) {
    std::ofstream file(path, std::ios::binary);
    if (file)
        file.write(reinterpret_cast<const char*>(bytes.data()), std::streamsize(bytes.size()));
    if (!file)
        throw std::runtime_error("Cannot write " + path);
}
void write_bmp(const std::string& path, const Image& image) {
    if (image.width <= 0 || image.height <= 0 ||
        image.rgb.size() != std::size_t(image.width) * image.height * 3)
        throw std::runtime_error("Invalid RGB image");
    const auto stride = (image.width * 3 + 3) & ~3;
    Bytes bmp(54 + stride * image.height, 0);
    auto put = [&](int offset, std::uint32_t value, int count) {
        for (int i = 0; i < count; ++i)
            bmp[offset + i] = std::uint8_t(value >> (8 * i));
    };
    bmp[0] = 'B';
    bmp[1] = 'M';
    put(2, std::uint32_t(bmp.size()), 4);
    put(10, 54, 4);
    put(14, 40, 4);
    put(18, image.width, 4);
    put(22, image.height, 4);
    put(26, 1, 2);
    put(28, 24, 2);
    put(34, stride * image.height, 4);
    for (int y = 0; y < image.height; ++y)
        for (int x = 0; x < image.width; ++x)
            for (int c = 0; c < 3; ++c)
                bmp[54 + (image.height - 1 - y) * stride + x * 3 + c] =
                    image.rgb[(y * image.width + x) * 3 + 2 - c];
    write_file(path, bmp);
}
namespace {
// table[k][n]: the CRC of byte n followed by k zero bytes, so eight bytes are
// taken at a time (the tests take one of every frame).
struct CrcTables {
    std::uint32_t table[8][256];
    CrcTables() {
        for (std::uint32_t n = 0; n < 256; ++n) {
            std::uint32_t c = n;
            for (int k = 0; k < 8; ++k)
                c = c & 1 ? 0xedb88320u ^ (c >> 1) : c >> 1;
            table[0][n] = c;
        }
        for (int k = 1; k < 8; ++k)
            for (std::uint32_t n = 0; n < 256; ++n)
                table[k][n] = table[0][table[k - 1][n] & 255] ^ (table[k - 1][n] >> 8);
    }
};
} // namespace
std::uint32_t crc32(const std::uint8_t* data, std::size_t size) {
    static const CrcTables tables;
    const auto& t = tables.table;
    std::uint32_t crc = 0xffffffffu;
    std::size_t n = 0;
    for (; n + 8 <= size; n += 8) {
        const std::uint32_t low =
            crc ^ (data[n] | std::uint32_t(data[n + 1]) << 8 | std::uint32_t(data[n + 2]) << 16 |
                   std::uint32_t(data[n + 3]) << 24);
        crc = t[7][low & 255] ^ t[6][(low >> 8) & 255] ^ t[5][(low >> 16) & 255] ^ t[4][low >> 24] ^
              t[3][data[n + 4]] ^ t[2][data[n + 5]] ^ t[1][data[n + 6]] ^ t[0][data[n + 7]];
    }
    for (; n < size; ++n)
        crc = t[0][(crc ^ data[n]) & 255] ^ (crc >> 8);
    return ~crc;
}
} // namespace stone
