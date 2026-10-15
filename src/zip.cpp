#include "zip.hpp"
#include "package.hpp"
#include <stdexcept>

// The central directory at the end of the zip lists every entry with its
// sizes and where its local header is; the local header comes before the
// entry's data. Sizes are taken from the central directory, which has them
// even when the local header leaves them to a data descriptor.
namespace stone {
namespace {
unsigned get(const Bytes& b, std::size_t at, int count) {
    if (at + count > b.size())
        throw std::runtime_error("The zip is damaged.");
    unsigned value = 0;
    for (int i = 0; i < count; ++i)
        value |= unsigned(b[at + i]) << (8 * i);
    return value;
}
void put(Bytes& b, unsigned value, int count) {
    for (int i = 0; i < count; ++i)
        b.push_back(uint8_t(value >> (8 * i)));
}
const unsigned local_header = 0x04034b50, central_header = 0x02014b50, end_record = 0x06054b50;
} // namespace

std::map<std::string, Bytes> read_zip(const Bytes& zip) {
    // The end record is the last 22 bytes, unless a comment of up to 64 KB follows it.
    if (zip.size() < 22)
        throw std::runtime_error("This is not a zip file.");
    std::size_t end = zip.size() - 22;
    while (get(zip, end, 4) != end_record) {
        if (end == 0 || zip.size() - end >= 22 + 65535)
            throw std::runtime_error("This is not a zip file.");
        --end;
    }
    const unsigned count = get(zip, end + 10, 2);
    std::size_t at = get(zip, end + 16, 4);
    std::map<std::string, Bytes> files;
    for (unsigned n = 0; n < count; ++n) {
        if (get(zip, at, 4) != central_header)
            throw std::runtime_error("The zip is damaged.");
        const unsigned flags = get(zip, at + 8, 2), method = get(zip, at + 10, 2);
        const unsigned crc = get(zip, at + 16, 4), packed = get(zip, at + 20, 4),
                       size = get(zip, at + 24, 4);
        const unsigned name_size = get(zip, at + 28, 2);
        const std::size_t next = at + 46 + name_size + get(zip, at + 30, 2) + get(zip, at + 32, 2);
        const std::size_t local = get(zip, at + 42, 4);
        if (at + 46 + name_size > zip.size())
            throw std::runtime_error("The zip is damaged.");
        const std::string name(zip.begin() + at + 46, zip.begin() + at + 46 + name_size);
        at = next;
        if (!name.empty() && (name.back() == '/' || name.back() == '\\'))
            continue; // a folder
        if (flags & 1)
            throw std::runtime_error("The zip is encrypted; a zip without a password is needed.");
        if (packed == 0xffffffff || size == 0xffffffff || local == 0xffffffff)
            throw std::runtime_error("The zip is in the Zip64 format, which is not supported.");
        if (method != 0 && method != 8)
            throw std::runtime_error("The zip's compression is not supported; "
                                     "only stored and deflated files are.");
        if (get(zip, local, 4) != local_header)
            throw std::runtime_error("The zip is damaged.");
        const std::size_t data = local + 30 + get(zip, local + 26, 2) + get(zip, local + 28, 2);
        if (data + packed > zip.size())
            throw std::runtime_error("The zip is damaged.");
        Bytes file = method == 0 ? Bytes(zip.begin() + data, zip.begin() + data + packed)
                                 : inflate(zip, data, packed);
        if (file.size() != size || crc32(file.data(), file.size()) != crc)
            throw std::runtime_error("The zip is damaged: " + name + " does not unpack correctly.");
        files[name] = std::move(file);
    }
    return files;
}

Bytes write_zip(const std::map<std::string, Bytes>& files) {
    Bytes zip, directory;
    for (const auto& file : files) {
        const unsigned crc = crc32(file.second.data(), file.second.size());
        const unsigned size = unsigned(file.second.size()), offset = unsigned(zip.size());
        const unsigned name_size = unsigned(file.first.size());
        // Local header: version 1.0, no flags, stored, time and date 0.
        put(zip, local_header, 4);
        put(zip, 10, 2);
        put(zip, 0, 2);
        put(zip, 0, 2);
        put(zip, 0, 4);
        put(zip, crc, 4);
        put(zip, size, 4);
        put(zip, size, 4);
        put(zip, name_size, 2);
        put(zip, 0, 2);
        zip.insert(zip.end(), file.first.begin(), file.first.end());
        zip.insert(zip.end(), file.second.begin(), file.second.end());
        // Its central directory entry.
        put(directory, central_header, 4);
        put(directory, 10, 2);
        put(directory, 10, 2);
        put(directory, 0, 2);
        put(directory, 0, 2);
        put(directory, 0, 4);
        put(directory, crc, 4);
        put(directory, size, 4);
        put(directory, size, 4);
        put(directory, name_size, 2);
        put(directory, 0, 2); // extra field
        put(directory, 0, 2); // comment
        put(directory, 0, 2); // disk
        put(directory, 0, 2); // internal attributes
        put(directory, 0, 4); // external attributes
        put(directory, offset, 4);
        directory.insert(directory.end(), file.first.begin(), file.first.end());
    }
    const unsigned start = unsigned(zip.size());
    zip.insert(zip.end(), directory.begin(), directory.end());
    put(zip, end_record, 4);
    put(zip, 0, 2);
    put(zip, 0, 2);
    put(zip, unsigned(files.size()), 2);
    put(zip, unsigned(files.size()), 2);
    put(zip, unsigned(directory.size()), 4);
    put(zip, start, 4);
    put(zip, 0, 2);
    return zip;
}
} // namespace stone
