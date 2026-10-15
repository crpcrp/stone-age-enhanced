#include "content.hpp"
#include "package.hpp"
#include <algorithm>
#include <cstdio>
#include <sstream>
#include <stdexcept>

namespace stone {
namespace {
const unsigned data_in_file = 0x33b0; // DS:0000 in SA_V1_00.EXE

// Areas of the data segment and how their bytes are written in texts.txt.
enum class Form { strings, pages };
struct Area {
    const char* name;
    uint16_t start, end;
    Form form;
};
const Area areas[] = {{"credits_idle", 0x006e, 0x0c52, Form::strings},
                      {"credits_victory", 0x0c52, 0x1499, Form::strings},
                      {"jukebox", 0x1499, 0x15ec, Form::strings},
                      {"song_files", 0x15ec, 0x1654, Form::strings},
                      {"scores", 0x1654, 0x17e1, Form::strings},
                      {"name_entry", 0x17ed, 0x18e3, Form::strings},
                      {"retry_page", 0x18e3, 0x196a, Form::pages},
                      {"completed_page", 0x196a, 0x19cf, Form::pages},
                      {"quotes", 0x19d7, 0x1b4e, Form::pages},
                      {"game_over_and_level_page", 0x1b4e, 0x1c0b, Form::pages}};
const uint16_t quote_pointers = 0x19cf; // offsets of the four quotes (CS:0b49)

// Palettes of sixteen colours, three six-bit values each.
struct Colours {
    const char* pattern;
    uint16_t start;
    unsigned count, first;
};
const Colours colours[] = {{"tiles_%u", 0x1cac, 5, 1},   // DS:1cac by theme
                           {"palette_%u", 0x1d9c, 6, 0}, // DS:1d9c by the level's palette
                           {"letters", 0x1ec5, 1, 0}};   // DS:1ec5

// The letters of the pages between levels: sprite 134 + code (CS:0bcc).
const char page_letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#$?!%.:,][ ";
const unsigned page_letter_count = sizeof page_letters - 1;
const uint8_t record_end = 0x7f, page_end = 0xff;

const char* const texts_help =
    "# Stone Age texts. Each [section] fills a fixed area of the program's data in\n"
    "# the original's own encoding. A shorter text is padded with zero bytes; a\n"
    "# longer one is refused.\n"
    "#\n"
    "# Text sections hold the bytes as written, except \\r (carriage return: the\n"
    "# next line down), \\n (line feed: back to the left edge), \\0 (end of a\n"
    "# string), \\\\ (a backslash) and \\xNN (any other byte; [ is always \\x5b).\n"
    "# Line breaks in this file are ignored: one follows each \\0 and each line end.\n"
    "# Characters are code page 437; in the font of the score and credits screens\n"
    "# \\xe0 starts bold letters and \\xe2 ends them.\n"
    "#   credits_idle     The credits (C on the menu): lines of 20 characters ended\n"
    "#                    by \\r\\n\\0. The scroll ends when its top line reaches byte\n"
    "#                    2722; the lines after that are the last screen. Before it\n"
    "#                    starts, the 20 bytes from 2177 and from 880 go to 694 and\n"
    "#                    717, in an order left to chance.\n"
    "#   credits_victory  The credits after level 100; the scroll ends at byte 1748.\n"
    "#   jukebox          Song titles F1-F8 (one line each, ended by \\r), the prompt\n"
    "#                    and the key help, each ended by \\0. Letters: A-Z . , \" -\n"
    "#                    space ! ? ( ' : ; 0-9.\n"
    "#   song_files       Eight song file names of 13 bytes each, padded with \\0.\n"
    "#   scores           The heading of the score table; then the table itself,\n"
    "#                    shown when there is no STONEAGE.HI: nine rows of 32 bytes\n"
    "#                    (name from byte 2, score from byte 24, then \\r\\n) and \\0.\n"
    "#   name_entry       The text above the name being entered.\n"
    "#\n"
    "# Page sections are drawn with the letters of sprites 134-180: one line a\n"
    "# record, x y |letters|, and an empty line ends a page. The letters are A-Z,\n"
    "# 0-9, # $ ? ! % . : , ] [ and space; # $ % are ornaments, ] [ and space are\n"
    "# blank. \\xNN is any other sprite code. The game writes into fixed bytes:\n"
    "#   retry_page                After a lost life.\n"
    "#   completed_page            After a level: score glyphs at byte 16, the next\n"
    "#                             level at 40. The 15 bytes from 45 are copied from\n"
    "#                             60 (lives) or 75 (one life), the digit to 49. The\n"
    "#                             password page is at 89, its letters at 93.\n"
    "#   quotes                    Four pages; one is drawn on the completed page.\n"
    "#   game_over_and_level_page  The game over page, then the page before a level\n"
    "#                             with the level number at byte 122.\n";

const char* const colours_help =
    "# Stone Age colours: sixteen lines of red green blue a section, six-bit DAC\n"
    "# values 0-63.\n"
    "#   tiles_1..5   Slots 0-15 in a level with tiles_1..5.bmp. Slot 14 is also the\n"
    "#                base of the background shades in slots 32-39.\n"
    "#   palette_0..5 Slots 16-31 in a level with that palette (hud.bmp).\n"
    "#   letters      Slots 48-63 on the pages between levels.\n";

std::string numbered(const char* pattern, unsigned n) {
    char name[32];
    std::snprintf(name, sizeof name, pattern, n);
    return name;
}
std::string hex(uint8_t c) {
    char text[8];
    std::snprintf(text, sizeof text, "\\x%02x", c);
    return text;
}
bool line_end(uint8_t c) {
    return c == 0 || c == '\r' || c == '\n';
}

std::string strings_text(const uint8_t* b, std::size_t size) {
    std::string s;
    for (std::size_t i = 0; i < size; ++i) {
        const uint8_t c = b[i];
        if (c == 0)
            s += "\\0";
        else if (c == '\r')
            s += "\\r";
        else if (c == '\n')
            s += "\\n";
        else if (c == '\\')
            s += "\\\\";
        else if (c >= 0x20 && c < 0x7f && c != '[')
            s += char(c);
        else
            s += hex(c);
        if (c == 0 || (line_end(c) && !(i + 1 < size && line_end(b[i + 1]))))
            s += '\n';
    }
    if (s.empty() || s.back() != '\n')
        s += '\n';
    return s;
}
uint8_t hex_value(const std::string& text, std::size_t at, const std::string& where) {
    unsigned value;
    if (at + 2 > text.size() || std::sscanf(text.substr(at, 2).c_str(), "%2x", &value) != 1)
        throw std::runtime_error(where + ": bad \\x escape");
    return uint8_t(value);
}
Bytes strings_bytes(const std::vector<std::string>& lines, const std::string& where) {
    std::string text;
    for (const std::string& line : lines)
        text += line;
    Bytes b;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] != '\\') {
            b.push_back(uint8_t(text[i]));
            continue;
        }
        const char e = i + 1 < text.size() ? text[++i] : ' ';
        if (e == '0')
            b.push_back(0);
        else if (e == 'r')
            b.push_back('\r');
        else if (e == 'n')
            b.push_back('\n');
        else if (e == '\\')
            b.push_back('\\');
        else if (e == 'x') {
            b.push_back(hex_value(text, i + 1, where));
            i += 2;
        } else
            throw std::runtime_error(where + ": unknown escape \\" + std::string(1, e));
    }
    return b;
}

// Records of x (word), y (byte) and letter codes, ended by 7Fh, or by a byte of
// 80h or more that ends the page (CS:0bcc).
std::string pages_text(const uint8_t* b, std::size_t size, const std::string& where) {
    std::ostringstream s;
    std::size_t at = 0;
    while (at < size) {
        bool rest_zero = true;
        for (std::size_t i = at; i < size; ++i)
            rest_zero = rest_zero && !b[i];
        if (rest_zero)
            break;
        for (bool page = true; page;) {
            if (at + 3 > size)
                throw std::runtime_error(where + " ends inside a record");
            s << (b[at] | b[at + 1] << 8) << ' ' << int(b[at + 2]) << " |";
            for (at += 3;; ++at) {
                if (at >= size)
                    throw std::runtime_error(where + " ends inside a record");
                const uint8_t c = b[at];
                if (c == record_end || c >= 0x80) {
                    if (c >= 0x80 && c != page_end)
                        throw std::runtime_error(where + ": unexpected page end");
                    page = c == record_end;
                    ++at;
                    break;
                }
                if (c < page_letter_count)
                    s << page_letters[c];
                else
                    s << hex(c);
            }
            s << "|\n";
        }
        s << '\n';
    }
    return s.str();
}
// The bytes, and where each page starts.
Bytes pages_bytes(const std::vector<std::string>& lines, const std::string& where,
                  std::vector<std::size_t>& starts) {
    Bytes b;
    bool open = false;
    for (const std::string& line : lines) {
        if (line.find_first_not_of(" \t") == std::string::npos) {
            if (open)
                b.back() = page_end;
            open = false;
            continue;
        }
        unsigned x, y;
        const auto first = line.find('|'), last = line.rfind('|');
        if (std::sscanf(line.c_str(), "%u %u", &x, &y) != 2 || first == std::string::npos ||
            last == first || x > 0xffff || y > 0xff)
            throw std::runtime_error(where + ": a record is x y |letters|: " + line);
        if (!open)
            starts.push_back(b.size());
        open = true;
        b.push_back(uint8_t(x));
        b.push_back(uint8_t(x >> 8));
        b.push_back(uint8_t(y));
        for (std::size_t i = first + 1; i < last; ++i) {
            if (line.compare(i, 2, "\\x") == 0) {
                const uint8_t c = hex_value(line, i + 2, where);
                if (c == record_end || c >= 0x80)
                    throw std::runtime_error(where + ": letter code out of range");
                b.push_back(c);
                i += 3;
                continue;
            }
            const char* found =
                std::char_traits<char>::find(page_letters, page_letter_count, line[i]);
            if (!found)
                throw std::runtime_error(where + ": no letter for '" + line.substr(i, 1) + "'");
            b.push_back(uint8_t(found - page_letters));
        }
        b.push_back(record_end);
    }
    if (open)
        b.back() = page_end;
    return b;
}

// Sections of a file: [name] lines, then the lines up to the next one. Lines
// before the first section are its description.
std::map<std::string, std::vector<std::string>> sections(const Bytes& file,
                                                         const std::string& path) {
    std::map<std::string, std::vector<std::string>> found;
    std::istringstream text(std::string(file.begin(), file.end()));
    std::vector<std::string>* current = nullptr;
    std::string line;
    while (std::getline(text, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.size() > 2 && line.front() == '[' && line.back() == ']') {
            const std::string name = line.substr(1, line.size() - 2);
            if (found.count(name))
                throw std::runtime_error(path + ": [" + name + "] appears twice");
            current = &found[name];
        } else if (current)
            current->push_back(line);
    }
    return found;
}
const std::vector<std::string>& section(const std::map<std::string, std::vector<std::string>>& all,
                                        const std::string& name, const std::string& path) {
    const auto found = all.find(name);
    if (found == all.end())
        throw std::runtime_error(path + " has no [" + name + "]");
    return found->second;
}
void put_data(std::map<unsigned, Bytes>& program, uint16_t offset, Bytes bytes) {
    program[data_in_file + offset] = std::move(bytes);
}

const uint16_t song_files_at = 0x15ec; // eight names of 13 bytes (CS:1f76)
std::vector<std::string> song_names(const uint8_t* names) {
    std::vector<std::string> found;
    for (unsigned n = 0; n < 8; ++n) {
        const char* name = reinterpret_cast<const char*>(names + n * 13);
        found.emplace_back(name, std::find(name, name + 12, 0));
        if (found.back().empty() || found.back().find_first_of("/\\") != std::string::npos)
            throw std::runtime_error("The song file names are not eight file names");
    }
    return found;
}
} // namespace

std::map<std::string, Bytes> content_files(const Bytes& exe) {
    const uint8_t* data = exe.data() + data_in_file;
    std::string texts = texts_help;
    for (const Area& area : areas) {
        texts += std::string("\n[") + area.name + "]\n";
        texts += area.form == Form::strings
                     ? strings_text(data + area.start, area.end - area.start)
                     : pages_text(data + area.start, area.end - area.start, area.name);
    }
    std::ostringstream palettes;
    palettes << colours_help;
    for (const Colours& set : colours)
        for (unsigned n = 0; n < set.count; ++n) {
            palettes << "\n[" << numbered(set.pattern, set.first + n) << "]\n";
            for (unsigned colour = 0; colour < 16; ++colour) {
                const uint8_t* c = data + set.start + (n * 16 + colour) * 3;
                palettes << int(c[0]) << ' ' << int(c[1]) << ' ' << int(c[2]) << '\n';
            }
        }
    const std::string colour_text = palettes.str();
    return {{"texts.txt", Bytes(texts.begin(), texts.end())},
            {"colours.txt", Bytes(colour_text.begin(), colour_text.end())}};
}

void content_program(const Package& package, std::map<unsigned, Bytes>& program) {
    {
        const auto all = sections(package.file("texts.txt"), "texts.txt");
        for (const Area& area : areas) {
            const std::string where = std::string("texts.txt [") + area.name + "]";
            const auto& lines = section(all, area.name, "texts.txt");
            std::vector<std::size_t> starts;
            Bytes b = area.form == Form::strings ? strings_bytes(lines, where)
                                                 : pages_bytes(lines, where, starts);
            const std::size_t size = area.end - area.start;
            if (b.size() > size)
                throw std::runtime_error(where + " is " + std::to_string(b.size() - size) +
                                         " bytes too long");
            b.resize(size, 0);
            put_data(program, area.start, b);
            if (area.start == 0x19d7) { // the quotes: point at each
                if (starts.size() != 4)
                    throw std::runtime_error(where + " must hold four pages");
                Bytes pointers;
                for (std::size_t start : starts) {
                    pointers.push_back(uint8_t((area.start + start) & 0xff));
                    pointers.push_back(uint8_t((area.start + start) >> 8));
                }
                put_data(program, quote_pointers, pointers);
            }
        }
    }
    {
        const auto all = sections(package.file("colours.txt"), "colours.txt");
        for (const Colours& set : colours) {
            Bytes b;
            for (unsigned n = 0; n < set.count; ++n) {
                const std::string name = numbered(set.pattern, set.first + n);
                const std::string where = "colours.txt [" + name + "]";
                unsigned count = 0;
                for (const std::string& line : section(all, name, "colours.txt")) {
                    if (line.find_first_not_of(" \t") == std::string::npos || line[0] == '#')
                        continue;
                    int r, g, bl;
                    char extra;
                    if (std::sscanf(line.c_str(), "%d %d %d %c", &r, &g, &bl, &extra) != 3 ||
                        r < 0 || r > 63 || g < 0 || g > 63 || bl < 0 || bl > 63)
                        throw std::runtime_error(where + ": a colour is red green blue, 0-63");
                    b.insert(b.end(), {uint8_t(r), uint8_t(g), uint8_t(bl)});
                    ++count;
                }
                if (count != 16)
                    throw std::runtime_error(where + " must hold sixteen colours");
            }
            put_data(program, set.start, b);
        }
    }
}

std::vector<std::string> song_files(const Bytes& exe) {
    return song_names(exe.data() + data_in_file + song_files_at);
}
std::vector<std::string> song_files(const Package& package) {
    std::map<unsigned, Bytes> program;
    content_program(package, program);
    return song_names(program.at(data_in_file + song_files_at).data());
}
} // namespace stone
