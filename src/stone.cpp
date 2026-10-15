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

std::string sha256(const Bytes& data) {
    static const std::uint32_t k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4,
        0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe,
        0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f,
        0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
        0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
        0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116,
        0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7,
        0xc67178f2};
    std::uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                          0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    // The message, a 1 bit, zeros and its length in bits fill whole 64-byte blocks.
    Bytes message = data;
    message.push_back(0x80);
    while (message.size() % 64 != 56)
        message.push_back(0);
    const std::uint64_t bits = std::uint64_t(data.size()) * 8;
    for (int i = 7; i >= 0; --i)
        message.push_back(std::uint8_t(bits >> (8 * i)));
    auto rotate = [](std::uint32_t x, int n) { return (x >> n) | (x << (32 - n)); };
    for (std::size_t block = 0; block < message.size(); block += 64) {
        std::uint32_t w[64];
        for (int i = 0; i < 16; ++i)
            w[i] = std::uint32_t(message[block + i * 4]) << 24 |
                   std::uint32_t(message[block + i * 4 + 1]) << 16 |
                   std::uint32_t(message[block + i * 4 + 2]) << 8 | message[block + i * 4 + 3];
        for (int i = 16; i < 64; ++i) {
            const std::uint32_t s0 = rotate(w[i - 15], 7) ^ rotate(w[i - 15], 18) ^ (w[i - 15] >> 3);
            const std::uint32_t s1 = rotate(w[i - 2], 17) ^ rotate(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6],
                      hh = h[7];
        for (int i = 0; i < 64; ++i) {
            const std::uint32_t t1 = hh + (rotate(e, 6) ^ rotate(e, 11) ^ rotate(e, 25)) +
                                     ((e & f) ^ (~e & g)) + k[i] + w[i];
            const std::uint32_t t2 =
                (rotate(a, 2) ^ rotate(a, 13) ^ rotate(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
            hh = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }
        h[0] += a, h[1] += b, h[2] += c, h[3] += d, h[4] += e, h[5] += f, h[6] += g, h[7] += hh;
    }
    std::string hex;
    for (std::uint32_t word : h)
        for (int i = 28; i >= 0; i -= 4)
            hex += "0123456789abcdef"[(word >> i) & 15];
    return hex;
}
} // namespace stone
