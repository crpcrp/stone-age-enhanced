#pragma once
#include "levels.hpp"
#include <algorithm>
#include <array>
#include <string>
#include <vector>

namespace stone {
// What the level editor (ENHANCEMENTS.md, E9) does to a level, apart from
// the screen: plain C++ on its 227-byte record (LevelSet::records), so the
// tests can try it without a Machine.
struct LevelRecord {
    static const unsigned columns = 20, rows = 11;
    std::array<uint8_t, LevelSet::record_size> bytes{};

    LevelRecord() = default;
    explicit LevelRecord(const uint8_t* record) {
        std::copy_n(record, LevelSet::record_size, bytes.begin());
    }
    // An empty slot: walls round the edge, floor inside, a start on the left
    // and an exit on the right; 60 in the record (75 seconds), theme,
    // palette and background 0.
    static LevelRecord empty();

    uint8_t at(unsigned column, unsigned row) const { return bytes[7 + row * columns + column]; }
    void set(unsigned column, unsigned row, uint8_t tile) { bytes[7 + row * columns + column] = tile; }
    // The record's time, 0-999 (the level gets time + time/4 seconds).
    unsigned time() const;
    void set_time(unsigned time);
    unsigned theme() const { return unsigned(bytes[3] - '0'); }
    void set_theme(unsigned theme) { bytes[3] = uint8_t('0' + theme); }
    unsigned palette() const { return bytes[4]; }
    void set_palette(unsigned palette) { bytes[4] = uint8_t(palette); }
    unsigned background() const { return bytes[5]; }
    void set_background(unsigned background) { bytes[5] = uint8_t(background); }
    bool operator==(const LevelRecord& other) const { return bytes == other.bytes; }
    bool operator!=(const LevelRecord& other) const { return bytes != other.bytes; }

    // The tiles the editor places, in the order Tab goes through them: A-S,
    // the outside `a` and the 3x3 rock, which `rock` stands for. The rest
    // (`k`-`m` and the signs) the game draws itself.
    static const uint8_t rock = 'e';
    static const std::vector<uint8_t>& tiles();
    // `tile` at a cell, as the editor places it. A start moves there from
    // where it was. The rock covers three by three cells from this one,
    // right and down, and fits only inside the board. A cell of a rock
    // takes the whole rock with it, as does a rock placed over part of one.
    // False when nothing changes.
    bool place(unsigned column, unsigned row, uint8_t tile);
    // The cell, and with it a whole rock, back to empty space (`A`).
    bool clear(unsigned column, unsigned row) { return place(column, row, 'A'); }

    // What the editor says of the level before it is saved: `errors` keep it
    // from being saved (not exactly one start, no exit), `warnings` do not
    // (keys without their gate, gates without their key, pieces of a rock).
    struct Check {
        std::vector<std::string> errors, warnings;
    };
    Check check() const;

private:
    // The top-left cell of the whole rock this cell is part of; false when
    // the cell is not part of a whole rock.
    bool rock_origin(unsigned column, unsigned row, unsigned& left, unsigned& top) const;
    void remove_rock(unsigned column, unsigned row);
};

// The file a pack of this name is saved as in levels/: the name with each
// character other than a letter, a digit, `-` or `_` as `_`, and `.txt`.
std::string pack_file_name(const std::string& name);
} // namespace stone
