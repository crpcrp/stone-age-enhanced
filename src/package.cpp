#include "package.hpp"
#include <algorithm>
#include <fstream>
#include <stdexcept>

// The package is the game's own format: a file table and deflate-compressed
// (RFC 1951) contents, with every byte scrambled so that it cannot be opened
// or replaced casually. It is no protection against a determined reader.
namespace stone {
namespace {
unsigned get(const Bytes& b, std::size_t at, int count) {
    unsigned value = 0;
    for (int i = 0; i < count; ++i)
        value |= unsigned(b.at(at + i)) << (8 * i);
    return value;
}
void put(Bytes& b, unsigned value, int count) {
    for (int i = 0; i < count; ++i)
        b.push_back(uint8_t(value >> (8 * i)));
}

const short length_base[] = {3,  4,  5,  6,  7,  8,  9,  10, 11,  13,  15,  17,  19,  23, 27,
                             31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
const short length_extra[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2,
                              2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
const short distance_base[] = {1,    2,    3,    4,    5,    7,    9,    13,    17,    25,
                               33,   49,   65,   97,   129,  193,  257,  385,   513,   769,
                               1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
const short distance_extra[] = {0, 0, 0, 0, 1, 1, 2, 2,  3,  3,  4,  4,  5,  5,  6,
                                6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

struct BitReader {
    const Bytes& in;
    std::size_t at, end;
    unsigned buffer = 0, count = 0;
    unsigned bits(unsigned need) {
        while (count < need) {
            if (at >= end)
                throw std::runtime_error("Truncated compressed data");
            buffer |= unsigned(in[at++]) << count;
            count += 8;
        }
        const unsigned value = buffer & ((1u << need) - 1);
        buffer >>= need;
        count -= need;
        return value;
    }
};
struct Huffman {
    short count[16], symbol[288];
    Huffman(const short* lengths, int n) {
        std::fill(count, count + 16, short(0));
        for (int i = 0; i < n; ++i)
            ++count[lengths[i]];
        count[0] = 0;
        short offset[16];
        offset[1] = 0;
        for (int len = 1; len < 15; ++len)
            offset[len + 1] = short(offset[len] + count[len]);
        for (int i = 0; i < n; ++i)
            if (lengths[i])
                symbol[offset[lengths[i]]++] = short(i);
    }
    int decode(BitReader& s) const {
        int code = 0, first = 0, index = 0;
        for (int len = 1; len <= 15; ++len) {
            code |= int(s.bits(1));
            if (code - count[len] < first)
                return symbol[index + (code - first)];
            index += count[len];
            first = (first + count[len]) << 1;
            code <<= 1;
        }
        throw std::runtime_error("Invalid compressed data");
    }
};
void inflate_block(BitReader& s, Bytes& out, const Huffman& literals, const Huffman& distances) {
    for (;;) {
        int symbol = literals.decode(s);
        if (symbol < 256)
            out.push_back(uint8_t(symbol));
        else if (symbol == 256)
            return;
        else {
            symbol -= 257;
            if (symbol >= 29)
                throw std::runtime_error("Invalid compressed data");
            unsigned length = length_base[symbol] + s.bits(length_extra[symbol]);
            const int code = distances.decode(s);
            if (code >= 30)
                throw std::runtime_error("Invalid compressed data");
            const unsigned distance = distance_base[code] + s.bits(distance_extra[code]);
            if (distance > out.size())
                throw std::runtime_error("Invalid compressed data");
            while (length--)
                out.push_back(out[out.size() - distance]);
        }
    }
}
} // namespace

Bytes inflate(const Bytes& in, std::size_t at, std::size_t size) {
    BitReader s{in, at, at + size};
    Bytes out;
    for (bool last = false; !last;) {
        last = s.bits(1);
        const unsigned type = s.bits(2);
        if (type == 0) {
            s.buffer = s.count = 0;
            if (s.at + 4 > s.end)
                throw std::runtime_error("Truncated compressed data");
            const unsigned length = get(in, s.at, 2);
            s.at += 4;
            if (s.at + length > s.end)
                throw std::runtime_error("Truncated compressed data");
            out.insert(out.end(), in.begin() + s.at, in.begin() + s.at + length);
            s.at += length;
        } else if (type == 1) {
            short lengths[288 + 30];
            for (int i = 0; i < 288; ++i)
                lengths[i] = short(i < 144 ? 8 : i < 256 ? 9 : i < 280 ? 7 : 8);
            std::fill(lengths + 288, lengths + 318, short(5));
            inflate_block(s, out, Huffman(lengths, 288), Huffman(lengths + 288, 30));
        } else if (type == 2) {
            static const short order[19] = {16, 17, 18, 0, 8,  7, 9,  6, 10, 5,
                                            11, 4,  12, 3, 13, 2, 14, 1, 15};
            const int nlen = int(s.bits(5)) + 257, ndist = int(s.bits(5)) + 1,
                      ncode = int(s.bits(4)) + 4;
            short lengths[320] = {};
            for (int i = 0; i < ncode; ++i)
                lengths[order[i]] = short(s.bits(3));
            const Huffman codes(lengths, 19);
            for (int i = 0; i < nlen + ndist;) {
                int symbol = codes.decode(s), repeat = 1, value = symbol;
                if (symbol == 16) {
                    if (!i)
                        throw std::runtime_error("Invalid compressed data");
                    value = lengths[i - 1];
                    repeat = 3 + int(s.bits(2));
                } else if (symbol == 17) {
                    value = 0;
                    repeat = 3 + int(s.bits(3));
                } else if (symbol == 18) {
                    value = 0;
                    repeat = 11 + int(s.bits(7));
                }
                if (i + repeat > nlen + ndist)
                    throw std::runtime_error("Invalid compressed data");
                while (repeat--)
                    lengths[i++] = short(value);
            }
            inflate_block(s, out, Huffman(lengths, nlen), Huffman(lengths + nlen, ndist));
        } else
            throw std::runtime_error("Invalid compressed data");
    }
    return out;
}

namespace {
// Deflate with the fixed Huffman codes and a greedy three-byte hash match.
struct BitWriter {
    Bytes out;
    unsigned buffer = 0, count = 0;
    void bits(unsigned value, unsigned n) {
        buffer |= value << count;
        count += n;
        while (count >= 8) {
            out.push_back(uint8_t(buffer));
            buffer >>= 8;
            count -= 8;
        }
    }
    void code(unsigned value, unsigned n) { // Huffman codes go most significant bit first
        unsigned reversed = 0;
        for (unsigned i = 0; i < n; ++i)
            reversed |= ((value >> i) & 1) << (n - 1 - i);
        bits(reversed, n);
    }
    void literal(unsigned symbol) {
        if (symbol < 144)
            code(0x30 + symbol, 8);
        else if (symbol < 256)
            code(0x190 + symbol - 144, 9);
        else if (symbol < 280)
            code(symbol - 256, 7);
        else
            code(0xc0 + symbol - 280, 8);
    }
};
Bytes deflate(const Bytes& in) {
    BitWriter w;
    w.bits(1, 1);
    w.bits(1, 2);
    std::vector<int> head(1 << 15, -1);
    const std::size_t n = in.size();
    for (std::size_t at = 0; at < n;) {
        unsigned best = 0, distance = 0;
        if (at + 3 <= n) {
            const unsigned hash = ((in[at] << 10) ^ (in[at + 1] << 5) ^ in[at + 2]) & 0x7fff;
            const int candidate = head[hash];
            head[hash] = int(at);
            if (candidate >= 0 && at - candidate <= 32768) {
                while (best < 258 && at + best < n && in[candidate + best] == in[at + best])
                    ++best;
                distance = unsigned(at - candidate);
            }
        }
        if (best < 3) {
            w.literal(in[at++]);
            continue;
        }
        int symbol = 28;
        while (length_base[symbol] > int(best))
            --symbol;
        w.literal(257 + symbol);
        w.bits(best - length_base[symbol], length_extra[symbol]);
        int code = 29;
        while (distance_base[code] > int(distance))
            --code;
        w.code(code, 5);
        w.bits(distance - distance_base[code], distance_extra[code]);
        at += best;
    }
    w.literal(256);
    w.bits(0, 7);
    return w.out;
}

// Descrambled layout: "SAGE", file count, then per file: name length (2),
// name, size (4), compressed size (4), compressed bytes.
void scramble(Bytes& data) {
    std::uint32_t state = 0x53413932;
    for (auto& byte : data) {
        state = state * 1103515245u + 12345u;
        byte ^= uint8_t(state >> 16);
    }
}
} // namespace

Package::Package(const std::string& path) {
    std::ifstream probe(path, std::ios::binary);
    probe.get(); // a directory may open, but nothing can be read from it
    if (!probe) { // not a file: a directory of loose asset files
        directory = path + "/";
        return;
    }
    probe.close();
    Bytes data = read_file(path);
    scramble(data);
    if (data.size() < 8 || std::string(data.begin(), data.begin() + 4) != "SAGE")
        throw std::runtime_error(path + " is not a Stone Age asset package");
    std::size_t at = 8;
    for (unsigned n = get(data, 4, 4); n--;) {
        const unsigned name_size = get(data, at, 2);
        const std::string name(data.begin() + at + 2, data.begin() + at + 2 + name_size);
        at += 2 + name_size;
        const unsigned size = get(data, at, 4), packed = get(data, at + 4, 4);
        at += 8;
        if (at + packed > data.size())
            throw std::runtime_error(path + " is damaged");
        files[name] = inflate(data, at, packed);
        if (files[name].size() != size)
            throw std::runtime_error(path + " is damaged");
        at += packed;
    }
}
bool Package::read(const std::string& name, Bytes& data) const {
    if (!directory.empty()) {
        std::ifstream file(directory + name, std::ios::binary);
        if (!file)
            return false;
        data.assign(std::istreambuf_iterator<char>(file), {});
        return true;
    }
    const auto found = files.find(name);
    if (found == files.end())
        return false;
    data = found->second;
    return true;
}
Bytes Package::file(const std::string& name) const {
    Bytes data;
    if (!read(name, data))
        throw std::runtime_error("Missing asset file " + name);
    return data;
}
void write_package(const std::string& path, const std::map<std::string, Bytes>& files) {
    Bytes data = {'S', 'A', 'G', 'E'};
    put(data, unsigned(files.size()), 4);
    for (const auto& entry : files) {
        const Bytes packed = deflate(entry.second);
        put(data, unsigned(entry.first.size()), 2);
        data.insert(data.end(), entry.first.begin(), entry.first.end());
        put(data, unsigned(entry.second.size()), 4);
        put(data, unsigned(packed.size()), 4);
        data.insert(data.end(), packed.begin(), packed.end());
    }
    scramble(data);
    write_file(path, data);
}
} // namespace stone
