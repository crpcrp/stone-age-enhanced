#include "assets.hpp"
#include "content.hpp"
#include "levels.hpp"
#include "machine.hpp"
#include <algorithm>
#include "package.hpp"
#include <cstdio>
#include <sstream>
#include <stdexcept>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

// Asset files and the resource of the original executable each one replaces.
// All pictures are uncompressed 8-bit BMP files. Pixel values are the game's
// palette indices; index 0 is transparent in sprites.
//
//   levels.txt               157e0  100 levels as text (format in the file itself and in
//                                   levels.cpp), with their passwords (DS:1f24)
//   texts.txt, colours.txt   texts and colours of the program image (content.cpp)
//   pars.txt                 optional: the levels' pars, lines of STONEAGE.ENH (E8)
//   tiles_1..5.bmp           17630, 18200, 18e40, 19a90, 1a710  40 tiles of 16x16, 8 per row
//   background_a1..a5.bmp    1b340, 1d7f0, 1f7e0, 217b0, 23830  320x176
//   background_b1..b5.bmp    41950, 46230, 4ac10, 4f690, 55510  320x176
//   hud.bmp                  25530  320x22
//   pause.bmp                27c20  320x176 mask: 2 shows the board, other values are drawn
//   screen_*.bmp             320x200 with their own palette (the BMP palette is used)
//   sprites/000..184.bmp     25f70 pixels and 27a40 directory
//   password/00..26.bmp      2e800 pixels and 2e7a0 directory
//   options/00..11.bmp       30d90 pixels, directory at 30d60
//   jukebox/000..099.bmp     31ce0 pixels and 31bd0 directory
//   birds/0..6.bmp           37380 pixels, directory at 373c0
//   font.bmp                 324c0  128 characters of 8x16 from code 32, 16 per row
//   sine.bin                 33720  bird flight table
//   *.CMF                    the eight songs, unchanged, by the names in texts.txt
//
// Except for the screens, the BMP palette only makes the picture look right
// in an editor; the game sets those colors itself. Sprites in a folder may
// change size (up to 255x255) but not number.
namespace stone {
namespace {
using Palette = std::array<uint8_t, 768>; // six-bit DAC values

struct Picture {
    int width = 0, height = 0;
    Bytes pixels; // row by row, top first
    Palette palette{};
};

void put(Bytes& b, std::size_t at, unsigned value, int count) {
    for (int i = 0; i < count; ++i)
        b[at + i] = uint8_t(value >> (8 * i));
}
unsigned get(const Bytes& b, std::size_t at, int count) {
    unsigned value = 0;
    for (int i = 0; i < count; ++i)
        value |= unsigned(b.at(at + i)) << (8 * i);
    return value;
}
Bytes picture_file(const Picture& p) {
    const int stride = (p.width + 3) & ~3;
    Bytes bmp(54 + 1024 + std::size_t(stride) * p.height, 0);
    bmp[0] = 'B';
    bmp[1] = 'M';
    put(bmp, 2, unsigned(bmp.size()), 4);
    put(bmp, 10, 54 + 1024, 4);
    put(bmp, 14, 40, 4);
    put(bmp, 18, unsigned(p.width), 4);
    put(bmp, 22, unsigned(p.height), 4);
    put(bmp, 26, 1, 2);
    put(bmp, 28, 8, 2);
    put(bmp, 34, unsigned(stride * p.height), 4);
    put(bmp, 46, 256, 4);
    for (int n = 0; n < 256; ++n)
        for (int c = 0; c < 3; ++c) { // stored blue, green, red
            const unsigned six = p.palette[n * 3 + 2 - c] & 63;
            bmp[54 + n * 4 + c] = uint8_t((six << 2) | (six >> 4));
        }
    for (int y = 0; y < p.height; ++y)
        std::copy_n(p.pixels.begin() + y * p.width, p.width,
                    bmp.begin() + 54 + 1024 + std::size_t(p.height - 1 - y) * stride);
    return bmp;
}
Picture file_picture(const Bytes& bmp, const std::string& path) {
    if (bmp.size() < 54 || bmp[0] != 'B' || bmp[1] != 'M' || get(bmp, 28, 2) != 8 ||
        get(bmp, 30, 4) != 0)
        throw std::runtime_error(path + " is not an uncompressed 8-bit BMP");
    Picture p;
    p.width = int(get(bmp, 18, 4));
    const int signed_height = int(get(bmp, 22, 4));
    p.height = signed_height < 0 ? -signed_height : signed_height;
    unsigned colors = get(bmp, 46, 4);
    if (!colors || colors > 256)
        colors = 256;
    const std::size_t table = 14 + get(bmp, 14, 4), data = get(bmp, 10, 4);
    const std::size_t stride = (std::size_t(p.width) + 3) & ~std::size_t(3);
    if (p.width <= 0 || p.height <= 0 || table + colors * 4 > bmp.size() ||
        data + stride * p.height > bmp.size())
        throw std::runtime_error(path + " is truncated");
    for (unsigned n = 0; n < colors; ++n)
        for (int c = 0; c < 3; ++c)
            p.palette[n * 3 + 2 - c] = uint8_t(bmp[table + n * 4 + c] >> 2);
    p.pixels.resize(std::size_t(p.width) * p.height);
    for (int y = 0; y < p.height; ++y) {
        const int row = signed_height < 0 ? y : p.height - 1 - y;
        std::copy_n(bmp.begin() + data + row * stride, p.width, p.pixels.begin() + y * p.width);
    }
    return p;
}
Picture expect(Picture p, int width, int height, const std::string& path) {
    if (p.width != width || p.height != height)
        throw std::runtime_error(path + " must be " + std::to_string(width) + "x" +
                                 std::to_string(height));
    return p;
}
std::string numbered(const std::string& pattern, unsigned n) {
    char name[64];
    std::snprintf(name, sizeof name, pattern.c_str(), n);
    return name;
}

// Sprites are stored column by column: the pixel at (x, y) is byte x * height + y.
Picture sprite_picture(const uint8_t* data, int width, int height, const Palette& palette) {
    Picture p;
    p.width = width;
    p.height = height;
    p.palette = palette;
    p.pixels.resize(std::size_t(width) * height);
    for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y)
            p.pixels[y * width + x] = data[x * height + y];
    return p;
}
void append_sprite(Bytes& sheet, const Picture& p) {
    for (int x = 0; x < p.width; ++x)
        for (int y = 0; y < p.height; ++y)
            sheet.push_back(p.pixels[y * p.width + x]);
}

const unsigned data_in_file = 0x33b0; // DS:0000 in SA_V1_00.EXE
const uint16_t consonants_at = 0x1f06, vowels_at = 0x1f15, passwords_at = 0x1f24;

struct Named {
    unsigned offset;
    const char* name;
};
const Named screens[] = {
    {0x28720, "screen_title"},   {0x2c330, "screen_password"}, {0x2eb40, "screen_options"},
    {0x30f10, "screen_jukebox"}, {0x328d0, "screen_scores"},   {0x373e0, "screen_bones_park"},
    {0x38c30, "screen_eclipse"}, {0x396a0, "screen_dinosaur"}, {0x3d770, "screen_credits"}};
const unsigned tiles[] = {0x17630, 0x18200, 0x18e40, 0x19a90, 0x1a710};
const unsigned backgrounds[] = {0x1b340, 0x1d7f0, 0x1f7e0, 0x217b0, 0x23830,
                                0x41950, 0x46230, 0x4ac10, 0x4f690, 0x55510};
// Sprite folders: pixels, directory, whether the directory is a packed
// resource (else plain bytes in the program image), file name pattern, and
// which screen's palette shows them best (0: a level's).
struct Sheet {
    unsigned pixels, directory, count;
    bool packed_directory;
    const char* folder;
    const char* pattern;
    unsigned palette_screen;
};
const Sheet sheets[] = {{0x25f70, 0x27a40, 185, true, "sprites", "sprites/%03u.bmp", 0},
                        {0x2e800, 0x2e7a0, 27, true, "password", "password/%02u.bmp", 0x2c330},
                        {0x30d90, 0x30d60, 12, false, "options", "options/%02u.bmp", 0x2eb40},
                        {0x31ce0, 0x31bd0, 100, true, "jukebox", "jukebox/%03u.bmp", 0x30f10},
                        {0x37380, 0x373c0, 7, false, "birds", "birds/%u.bmp", 0x328d0}};
void make_directory(const std::string& path) {
#ifdef _WIN32
    _mkdir(path.c_str());
#else
    mkdir(path.c_str(), 0777);
#endif
}
std::vector<std::string> asset_names(const std::vector<std::string>& songs) {
    std::vector<std::string> names = {"levels.txt", "sine.bin", "hud.bmp", "pause.bmp", "font.bmp"};
    names.insert(names.end(), songs.begin(), songs.end());
    for (const auto& screen : screens)
        names.push_back(std::string(screen.name) + ".bmp");
    for (unsigned n = 0; n < 5; ++n) {
        names.push_back(numbered("tiles_%u.bmp", n + 1));
        names.push_back(numbered("background_a%u.bmp", n + 1));
        names.push_back(numbered("background_b%u.bmp", n + 1));
    }
    for (const auto& sheet : sheets)
        for (unsigned n = 0; n < sheet.count; ++n)
            names.push_back(numbered(sheet.pattern, n));
    return names;
}
Picture rows_picture(const Bytes& data, int width, int height, const Palette& palette) {
    Picture p;
    p.width = width;
    p.height = height;
    p.palette = palette;
    p.pixels.assign(data.begin(), data.begin() + std::size_t(width) * height);
    return p;
}
// Font 324c0: 128 characters from code 32, each 8 columns of 16 rows. A column is
// a little-endian word with the top row in its highest bit.
Picture font_picture(const Bytes& data) {
    Picture p;
    p.width = 16 * 8;
    p.height = 8 * 16;
    p.palette[3] = p.palette[4] = p.palette[5] = 63;
    p.pixels.assign(std::size_t(p.width) * p.height, 0);
    for (int ch = 0; ch < 128; ++ch)
        for (int x = 0; x < 8; ++x)
            for (int y = 0; y < 16; ++y)
                if (data.at(ch * 16 + x * 2 + 1 - y / 8) & (0x80 >> (y & 7)))
                    p.pixels[((ch / 16) * 16 + y) * p.width + (ch % 16) * 8 + x] = 1;
    return p;
}
Bytes font_bytes(const Picture& p) {
    Bytes data(2048, 0);
    for (int ch = 0; ch < 128; ++ch)
        for (int x = 0; x < 8; ++x)
            for (int y = 0; y < 16; ++y)
                if (p.pixels[((ch / 16) * 16 + y) * p.width + (ch % 16) * 8 + x])
                    data[ch * 16 + x * 2 + 1 - y / 8] |= uint8_t(0x80 >> (y & 7));
    return data;
}

// The palette a level shows: play the original to that level and read the DAC.
class PaletteProbe {
public:
    explicit PaletteProbe(const Bytes& exe) : machine(exe) {
        machine.start();
        wait(100);
        key(0x39);
        wait(150);
        key(0x39);
        wait(200);
        text("MAYDAY");
        key(0x3c);
        wait(300);
    }
    void other_backgrounds() {
        key(0x41); // F7
        wait(20);
    }
    Palette level(unsigned number) {
        text(numbered("LVL%03u", number));
        key(0x3c); // F2: magic word
        wait(300);
        key(0x39);
        wait(230);
        const Palette palette = machine.frame().palette;
        key(0x01); // Esc, then a key for the game-over page
        wait(250);
        key(0x39);
        wait(300);
        return palette;
    }

private:
    Machine machine;
    uint8_t scan = 0;
    void wait(int ticks) {
        while (ticks-- > 0)
            machine.tick(scan);
    }
    void key(uint8_t code) {
        scan = code;
        wait(3);
        scan = uint8_t(code | 128);
        wait(3);
    }
    void text(const std::string& characters) {
        for (char c : characters)
            machine.type(uint8_t(c));
        machine.type(13);
    }
};
} // namespace

namespace {
// Every asset file of the original game, by name.
std::map<std::string, Bytes> original_files(const std::string& executable) {
    const Bytes exe = read_file(executable);
    Machine machine(exe);
    std::map<std::string, Bytes> files;
    auto path = [](const std::string& name) {
        return name;
    };
    auto write_file = [&](const std::string& name, const Bytes& data) {
        files[name] = data;
    };
    auto write_picture = [&](const std::string& name, const Picture& p) {
        files[name] = picture_file(p);
    };
    // The songs are separate files already: take them along unchanged.
    const auto slash = executable.find_last_of("/\\");
    const std::string folder = slash == std::string::npos ? "" : executable.substr(0, slash + 1);
    for (const std::string& name : song_files(exe))
        files[name] = read_file(folder + name);
    const Bytes levels = machine.unpack(0x157e0);
    {
        LevelSet set;
        set.count = LevelSet::most;
        set.records = levels;
        const uint8_t* passwords = exe.data() + data_in_file + passwords_at;
        set.passwords.assign(passwords, passwords + 3 * LevelSet::most);
        const std::string text = write_levels(set);
        write_file(path("levels.txt"), Bytes(text.begin(), text.end()));
    }
    write_file(path("sine.bin"), machine.unpack(0x33720));
    for (const auto& file : content_files(exe))
        write_file(file.first, file.second);

    // One level for each tile theme and each background, to borrow its palette.
    unsigned theme_level[5] = {}, background_level[5] = {};
    for (unsigned n = 100; n-- > 0;) {
        unsigned theme = uint8_t(levels[n * 227 + 3] - '0');
        if (theme >= 5)
            theme = 3;
        theme_level[theme] = n + 1;
        background_level[levels[n * 227 + 5] & 7] = n + 1;
    }
    PaletteProbe probe(exe);
    Palette theme_palette[5], background_palette[10];
    for (int n = 0; n < 5; ++n)
        theme_palette[n] = probe.level(theme_level[n]);
    for (int n = 0; n < 5; ++n)
        background_palette[n] = probe.level(background_level[n]);
    probe.other_backgrounds();
    for (int n = 0; n < 5; ++n)
        background_palette[5 + n] = probe.level(background_level[n]);
    const Palette& level_palette = theme_palette[0];

    std::map<unsigned, Palette> screen_palette;
    for (const auto& screen : screens) {
        const Bytes data = machine.unpack(screen.offset);
        Picture p = rows_picture(data, 320, 200, Palette());
        std::copy_n(data.begin() + 64000, 768, p.palette.begin());
        screen_palette[screen.offset] = p.palette;
        write_picture(path(std::string(screen.name) + ".bmp"), p);
    }
    for (unsigned n = 0; n < 5; ++n) {
        const Bytes data = machine.unpack(tiles[n]);
        Picture sheet;
        sheet.width = 8 * 16;
        sheet.height = 5 * 16;
        sheet.palette = theme_palette[n];
        sheet.pixels.resize(std::size_t(sheet.width) * sheet.height);
        for (int tile = 0; tile < 40; ++tile)
            for (int x = 0; x < 16; ++x)
                for (int y = 0; y < 16; ++y)
                    sheet.pixels[((tile / 8) * 16 + y) * sheet.width + (tile % 8) * 16 + x] =
                        data[tile * 256 + x * 16 + y];
        write_picture(path(numbered("tiles_%u.bmp", n + 1)), sheet);
    }
    for (unsigned n = 0; n < 10; ++n)
        write_picture(
            path(numbered(n < 5 ? "background_a%u.bmp" : "background_b%u.bmp", n % 5 + 1)),
            rows_picture(machine.unpack(backgrounds[n]), 320, 176, background_palette[n]));
    write_picture(path("hud.bmp"), rows_picture(machine.unpack(0x25530), 320, 22, level_palette));
    write_picture(path("pause.bmp"),
                  rows_picture(machine.unpack(0x27c20), 320, 176, level_palette));
    write_picture(path("font.bmp"), font_picture(machine.unpack(0x324c0)));
    for (const auto& sheet : sheets) {
        const Bytes pixels = machine.unpack(sheet.pixels);
        Bytes entries;
        if (sheet.packed_directory)
            entries = machine.unpack(sheet.directory);
        else
            entries.assign(exe.begin() + sheet.directory,
                           exe.begin() + sheet.directory + sheet.count * 4);
        const Palette& palette =
            sheet.palette_screen ? screen_palette[sheet.palette_screen] : level_palette;
        for (unsigned n = 0; n < sheet.count; ++n)
            write_picture(path(numbered(sheet.pattern, n)),
                          sprite_picture(pixels.data() + get(entries, n * 4, 2), entries[n * 4 + 2],
                                         entries[n * 4 + 3], palette));
    }
    return files;
}
} // namespace

void export_assets(const std::string& executable, const std::string& directory) {
    const std::map<std::string, Bytes> files = original_files(executable);
    make_directory(directory);
    for (const auto& sheet : sheets)
        make_directory(directory + "/" + sheet.folder);
    for (const auto& file : files)
        stone::write_file(directory + "/" + file.first, file.second);
}
void pack_original(const std::string& executable, const std::string& package) {
    write_package(package, original_files(executable));
}
void pack_assets(const std::string& directory, const std::string& package) {
    const Package loose(directory);
    const Assets check(loose); // refuses anything the game could not load
    std::map<std::string, Bytes> files;
    for (const std::string& name : asset_names(song_files(loose)))
        files[name] = loose.file(name);
    for (const char* name : {"texts.txt", "colours.txt"})
        files[name] = loose.file(name);
    Bytes pars; // the levels' pars, found ahead of time (ENHANCEMENTS.md, E8); optional
    if (loose.read("pars.txt", pars))
        files["pars.txt"] = pars;
    write_package(package, files);
}

Assets::Assets(const Package& package)
    : Assets(package, read_levels(package.file("levels.txt"), "levels.txt", false), package) {}

Assets::Assets(const Package& graphics, const LevelSet& maps, const Package& music) {
    auto path = [](const std::string& name) {
        return name;
    };
    auto read_file = [&](const std::string& name) {
        return graphics.file(name);
    };
    auto read_picture = [&](const std::string& name) {
        return file_picture(graphics.file(name), name);
    };
    resources[0x157e0] = maps.records;
    program[data_in_file + passwords_at] = maps.passwords;
    content_program(graphics, program);
    if (&music != &graphics)
        song_program(music, program);
    resources[0x33720] = read_file(path("sine.bin"));
    if (resources[0x33720].size() != 16384)
        throw std::runtime_error("sine.bin has the wrong size");
    for (const auto& screen : screens) {
        const std::string name = path(std::string(screen.name) + ".bmp");
        const Picture p = expect(read_picture(name), 320, 200, name);
        Bytes& data = resources[screen.offset];
        data = p.pixels;
        data.insert(data.end(), p.palette.begin(), p.palette.end());
    }
    for (unsigned n = 0; n < 5; ++n) {
        const std::string name = path(numbered("tiles_%u.bmp", n + 1));
        const Picture sheet = expect(read_picture(name), 128, 80, name);
        Bytes& data = resources[tiles[n]];
        data.resize(10240);
        for (int tile = 0; tile < 40; ++tile)
            for (int x = 0; x < 16; ++x)
                for (int y = 0; y < 16; ++y)
                    data[tile * 256 + x * 16 + y] =
                        sheet.pixels[((tile / 8) * 16 + y) * sheet.width + (tile % 8) * 16 + x];
    }
    for (unsigned n = 0; n < 10; ++n) {
        const std::string name =
            path(numbered(n < 5 ? "background_a%u.bmp" : "background_b%u.bmp", n % 5 + 1));
        resources[backgrounds[n]] = expect(read_picture(name), 320, 176, name).pixels;
    }
    resources[0x25530] = expect(read_picture(path("hud.bmp")), 320, 22, "hud.bmp").pixels;
    resources[0x27c20] = expect(read_picture(path("pause.bmp")), 320, 176, "pause.bmp").pixels;
    resources[0x324c0] = font_bytes(expect(read_picture(path("font.bmp")), 128, 128, "font.bmp"));
    for (const auto& sheet : sheets) {
        Bytes pixels, entries;
        for (unsigned n = 0; n < sheet.count; ++n) {
            const std::string name = path(numbered(sheet.pattern, n));
            const Picture p = read_picture(name);
            if (p.width > 255 || p.height > 255 || pixels.size() + p.pixels.size() > 65535)
                throw std::runtime_error(name + " is too large");
            entries.resize(entries.size() + 4);
            put(entries, entries.size() - 4, unsigned(pixels.size()), 2);
            entries[entries.size() - 2] = uint8_t(p.width);
            entries[entries.size() - 1] = uint8_t(p.height);
            append_sprite(pixels, p);
        }
        resources[sheet.pixels] = pixels;
        (sheet.packed_directory ? resources : program)[sheet.directory] = entries;
    }
}
const Bytes* Assets::find(unsigned file_offset) const {
    auto found = resources.find(file_offset);
    return found == resources.end() ? nullptr : &found->second;
}
void Assets::install(Machine& machine) const {
    machine.resource = [this](unsigned offset) {
        return find(offset);
    };
    for (const auto& bytes : program)
        machine.write_image(bytes.first, bytes.second);
}
void verify_assets(const std::string& executable, const std::string& package) {
    const Bytes exe = read_file(executable);
    Machine machine(exe);
    const Assets assets{Package(package)};
    unsigned count = 0;
    for (std::size_t at = 512; at + 12 < exe.size(); at += 16)
        if (exe[at] == 'I' && exe[at + 1] == 'C' && exe[at + 2] == 'E' && exe[at + 3] == '!') {
            const Bytes* file = assets.find(unsigned(at));
            if (!file || *file != machine.unpack(unsigned(at)))
                throw std::runtime_error("Asset files differ from the original resource at " +
                                         std::to_string(at));
            ++count;
        }
    if (count != 37)
        throw std::runtime_error("Expected 37 resources");
    // Program bytes: sprite directories, passwords, texts and colours. A password
    // may be spelt with other codes than the original's.
    const uint8_t* data = exe.data() + data_in_file;
    const auto letters = [&](const uint8_t* code) {
        std::string s;
        for (int n = 0; n < 3; ++n)
            s += {char(data[consonants_at + (code[n] >> 4)]), char(data[vowels_at + (code[n] & 15)])};
        return s;
    };
    unsigned bytes = 0;
    for (const auto& entry : assets.program) {
        const uint8_t* original = exe.data() + entry.first;
        bool same = std::equal(entry.second.begin(), entry.second.end(), original);
        if (entry.first == data_in_file + passwords_at) {
            same = true;
            for (std::size_t n = 0; n < entry.second.size(); n += 3)
                same = same && letters(&entry.second[n]) == letters(original + n);
        }
        if (!same)
            throw std::runtime_error("Asset files differ from the original program bytes at " +
                                     std::to_string(entry.first));
        bytes += unsigned(entry.second.size());
    }
    if (bytes != 76 + 7933) // research/tools/data_map.py: the directories and all content
        throw std::runtime_error("Expected the sprite directories, passwords, texts and colours");
}
} // namespace stone
