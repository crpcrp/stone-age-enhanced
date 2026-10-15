#include "solver.hpp"
#include "assets.hpp"
#include "game.hpp"
#include "levels.hpp"
#include "machine.hpp"
#include "package.hpp"
#include <algorithm>
#include <climits>
#include <cstring>
#include <queue>
#include <stdexcept>

namespace stone {
namespace {
// The model (ENHANCEMENTS.md, E8). Each rule names the routine of game.cpp
// it mirrors. Times are level ticks, as Machine::level_ticks counts them:
// the first tick of play is 1.
const int columns = 20, rows = 11, cells = columns * rows;
const unsigned walk_ticks = 16;      // a step at one pixel a tick (aim, move_toward_target)
const unsigned ride_ticks = 8;       // a cell at two pixels a tick, platform or selector
const unsigned teleport_ticks = 86;  // from arriving on a Q to obeying keys at the other
const unsigned offer_ticks = 43;     // each further teleport offered, with more than two
const unsigned crumble_stone = 18;   // a crumbling floor is A after this long (crumble_tiles)
const unsigned crumble_done = 24;    // and stops keeping the exit shut (exit_open)
const unsigned hourglass_ticks = 50 * 70;
const uint8_t space = 0x39;
// Ways in the order up, down, left, right: Direction::bit is 1 << way.
const uint8_t arrow[4] = {0x48, 0x50, 0x4b, 0x4d};
const int way_columns[4] = {0, 0, -1, 1}, way_rows[4] = {-1, 1, 0, 0};

int beside(int cell, int way) { // -1 off the board, where Board::at gives wall
    const int column = cell % columns + way_columns[way], row = cell / columns + way_rows[way];
    return column < 0 || column >= columns || row < 0 || row >= rows ? -1 : row * columns + column;
}
unsigned distance(int a, int b) {
    return unsigned(std::abs(a % columns - b % columns) + std::abs(a / columns - b / columns));
}
// A and a: platforms cross it and stop before anything else (ride_platform).
bool stone(uint8_t tile) { return (tile & 0xdf) == 'A'; }
bool platform(uint8_t tile) { return tile >= 'C' && tile <= 'I'; }
bool one_way(uint8_t tile) { return tile >= 'C' && tile <= 'F'; }
// Game::platform_goes.
bool goes(uint8_t tile, int way) {
    const uint8_t only = way == 0 ? 'C' : way == 1 ? 'D' : way == 2 ? 'F' : 'E';
    return tile == only || tile == (way < 2 ? 'G' : 'H') || tile == 'I';
}

// The tiles a state keeps, four bits a cell: all others never change. A
// crumbling floor (k, l, m) is `k` until it is A.
const char codes[] = "ABCDEFGHIJKkMNOP";

struct Level {
    uint8_t tiles[cells];
    std::vector<uint8_t> changing; // cells whose tile can change
    std::vector<uint8_t> teleports; // row by row (Game::next_teleport)
    unsigned time_limit = 0;        // the tick the countdown runs out at
    uint8_t start = 0;
};

Level read_level(const uint8_t* record) {
    Level level;
    // Game::load_level and start_level: a quarter more seconds than the
    // record says; 0 counts round to 5,536.
    const uint8_t hundreds = uint8_t(record[0] - '0'), tens = uint8_t(record[1] - '0'),
                  ones = uint8_t(record[2] - '0');
    const uint16_t time = uint16_t(hundreds * 100 + tens * 10 + (tens * 10 & 0xff00) + ones);
    const uint16_t seconds = uint16_t(time + time / 4);
    level.time_limit = 70u * (seconds ? seconds % 10000 : 5536);
    for (int cell = 0; cell < cells; ++cell) {
        uint8_t tile = record[7 + cell];
        if (tile == 'a' || tile == 'q') // the same to everything that moves
            tile = tile == 'a' ? 'A' : 'B';
        level.tiles[cell] = tile;
        if (std::strchr(codes, tile) && tile != 'B')
            level.changing.push_back(uint8_t(cell));
        if (tile == 'R')
            level.start = uint8_t(cell);
        if (tile == 'Q')
            level.teleports.push_back(uint8_t(cell));
    }
    return level;
}

struct Crumble {
    uint8_t cell;
    unsigned start; // the tick it started (Crumbling::start)
};

// Where everything is when the game obeys keys.
struct State {
    uint8_t tiles[cells];
    uint8_t player = 0, selector = 0;
    // 0: the player moves; 1: the selector is out; 2: the selector is out on
    // a platform that waits for a direction key (platform_waits).
    uint8_t mode = 0;
    // The mover has just landed on a platform: Game::walk reads the tile
    // under the player, and drop_selector the tile beside, from before the
    // platform was put down, so a launch and a Space wait a tick.
    bool landed = false;
    bool space_down = false; // Space goes up before it can go down again

    bool armed = true; // teleport_armed
    uint8_t keys_a = 0, keys_b = 0, hourglasses = 0;
    std::vector<Crumble> crumbles;
};

enum Kind { press, bring_out, wait, drop, withdraw, launch, show };
struct Move {
    Kind kind;
    int way, cell, teleport; // teleport: which of more than two the player comes out of
};
uint32_t encode(const Move& move) {
    return uint32_t(move.kind) | uint32_t(move.way) << 4 | uint32_t(move.teleport) << 8 |
           uint32_t(move.cell) << 16;
}
Move decode(uint32_t code) {
    return {Kind(code & 15), int(code >> 4 & 15), int(code >> 16), int(code >> 8 & 255)};
}

// One move from a state, as the game would play it.
class Play {
public:
    Play(const Level& level, const State& state, unsigned tick, bool timed,
         std::vector<SolverStep>* steps)
        : level(level), s(state), t(tick), timed(timed), steps(steps) {}
    const Level& level;
    State s;
    unsigned t; // the tick the game obeys keys again, or the map is cleared
    bool timed;
    std::vector<SolverStep>* steps;
    enum Result { impossible, playing, cleared };

    Result apply(const Move& move) {
        const bool landed = s.landed, down = s.space_down, late = landed || down;
        s.landed = s.space_down = false;
        Result result = impossible;
        switch (move.kind) {
        case press:
            result = s.mode == 0 ? press_way(move.way, move.teleport, landed) : impossible;
            break;
        case bring_out:
            if (s.mode != 0)
                break;
            note(false, s.player, space, t + down);
            s.mode = 1;
            s.selector = s.player;
            s.space_down = true;
            result = advance(t + down + 1);
            break;
        case wait: // only while a floor crumbles: nothing else changes with time
            result = s.crumbles.empty() ? impossible : advance(t + 1);
            break;
        case drop: result = s.mode == 1 ? drop_on(move.cell, move.way, late) : impossible; break;
        case withdraw: result = s.mode == 1 ? withdraw_to_player(late) : impossible; break;
        case launch:
            if (s.mode != 2 || !goes(s.tiles[s.selector], move.way) ||
                !stone(tile(beside(s.selector, move.way), t)))
                break;
            note(true, s.selector, arrow[move.way], t);
            result = ride(s.selector, move.way, t, true);
            break;
        case show:
            if (s.mode != 2)
                break;
            {
                const unsigned at = t + late;
                note(true, s.selector, space, at);
                s.mode = 1;
                s.space_down = true;
                result = advance(at + 1);
            }
            break;
        }
        if (result == playing)
            settle();
        return result;
    }

    // The tile at a cell at a tick, crumbling floors as they will be then.
    uint8_t tile(int cell, unsigned tick) const {
        if (cell < 0)
            return 'L';
        const uint8_t here = s.tiles[cell];
        if (here != 'k')
            return here;
        for (const Crumble& crumble : s.crumbles)
            if (crumble.cell == cell)
                return tick >= crumble.start + crumble_stone ? 'A' : 'k';
        return 'A';
    }
    unsigned deadline() const {
        return timed ? level.time_limit + hourglass_ticks * s.hourglasses : UINT_MAX;
    }

private:
    // Game::blocks.
    bool blocks(uint8_t tile) const {
        switch (tile) {
        case 'A':
        case 'L': return true;
        case 'M': return s.keys_a == 0;
        case 'N': return s.keys_b == 0;
        default: return tile >= 'a' && tile <= 'm';
        }
    }
    void note(bool selector, int cell, uint8_t scan, unsigned tick, bool offer = false) {
        if (steps) {
            SolverStep step;
            step.selector = selector;
            step.offer = offer;
            step.cell = uint8_t(cell);
            step.scan = scan;
            step.tick = tick;
            steps->push_back(step);
        }
    }
    Result advance(unsigned tick) {
        t = tick;
        return tick < deadline() ? playing : impossible;
    }
    // Crumbling floors that have finished are A for good.
    void settle() {
        auto done = [&](const Crumble& crumble) {
            if (t < crumble.start + crumble_done)
                return false;
            s.tiles[crumble.cell] = 'A';
            return true;
        };
        s.crumbles.erase(std::remove_if(s.crumbles.begin(), s.crumbles.end(), done),
                         s.crumbles.end());
    }

    // Game::walk: a step, a platform set off, or a push against something
    // that blocks, which arms the teleports (Game::probe).
    Result press_way(int way, int teleport, bool landed) {
        const int ahead = beside(s.player, way);
        const uint8_t tile_ahead = tile(ahead, t);
        if (!blocks(tile_ahead)) {
            note(false, s.player, arrow[way], t);
            return arrive(ahead, t + walk_ticks, teleport);
        }
        if (teleport > 1)
            return impossible;
        const unsigned at = t + landed;
        if (stone(tile(ahead, at)) && goes(s.tiles[s.player], way)) {
            note(false, s.player, arrow[way], at);
            return ride(s.player, way, at, false);
        }
        if (s.tiles[s.player] != 'Q' || s.armed)
            return impossible; // it changes nothing
        note(false, s.player, arrow[way], t);
        s.armed = true;
        return arrive_on_teleport(t + 1, teleport);
    }

    // The player reaches a cell on foot (Game::arrive, passes_over, step_on,
    // exit_open).
    Result arrive(int cell, unsigned tick, int teleport) {
        if (tick >= deadline())
            return impossible;
        if (s.tiles[s.player] == 'K') { // the floor left behind starts to crumble
            s.tiles[s.player] = 'k';
            if (s.crumbles.size() < 4)
                s.crumbles.push_back({s.player, tick});
        }
        s.player = uint8_t(cell);
        const uint8_t here = tile(cell, tick);
        if (here != 'Q')
            s.armed = true;
        if (teleport > 1 && !(here == 'Q' && s.armed))
            return impossible;
        switch (here) {
        case 'O':
            if (s.keys_a < 255) {
                ++s.keys_a;
                s.tiles[cell] = 'B';
            }
            break;
        case 'P':
            ++s.keys_b;
            s.tiles[cell] = 'B';
            break;
        case 'M':
            --s.keys_a;
            s.tiles[cell] = 'B';
            break;
        case 'N':
            --s.keys_b;
            s.tiles[cell] = 'B';
            break;
        case 'J':
            ++s.hourglasses;
            s.tiles[cell] = 'B';
            break;
        case 'Q':
            if (s.armed)
                return arrive_on_teleport(tick, teleport);
            break;
        case 'S': {
            unsigned open = tick;
            for (const Crumble& crumble : s.crumbles)
                open = std::max(open, crumble.start + crumble_done);
            t = open;
            return open < deadline() ? cleared : impossible;
        }
        default: break;
        }
        return advance(tick);
    }

    // Game::teleport and after_teleport_animation: with two teleports the
    // player comes out of the other; with more, of the one offered while
    // Space is held.
    Result arrive_on_teleport(unsigned tick, int teleport) {
        if (tick >= deadline())
            return impossible;
        const std::size_t count = level.teleports.size();
        const std::size_t at =
            std::find(level.teleports.begin(), level.teleports.end(), s.player) -
            level.teleports.begin();
        unsigned ticks = teleport_ticks;
        if (count > 2) {
            if (teleport < 1 || std::size_t(teleport) >= count)
                return impossible;
            ticks += offer_ticks * unsigned(teleport - 1);
        } else
            teleport = 1;
        s.player = level.teleports[(at + unsigned(teleport)) % count];
        if (count > 2)
            note(false, s.player, space, tick, true);
        s.armed = false;
        return advance(tick + ticks);
    }

    // Game::launch_platform, ride_platform and land_platform: the platform
    // under the mover at `from` sets off at `tick` and goes on while there is
    // stone ahead.
    Result ride(int from, int way, unsigned tick, bool selector) {
        const uint8_t carried = s.tiles[from];
        s.tiles[from] = 'A';
        int at = beside(from, way);
        unsigned when = tick + ride_ticks;
        for (;; when += ride_ticks) {
            if (when >= deadline())
                return impossible;
            const int next = beside(at, way);
            if (!stone(tile(next, when)))
                break;
            at = next;
        }
        // A floor still crumbling there would draw over the platform.
        for (const Crumble& crumble : s.crumbles)
            if (crumble.cell == at)
                return impossible;
        s.tiles[at] = carried;
        if (selector) {
            s.selector = uint8_t(at);
            s.mode = one_way(carried) ? 1 : 2;
        } else {
            s.player = uint8_t(at);
            s.armed = true;
        }
        s.landed = true;
        return advance(when);
    }

    // The selector flies to a cell, a row at a time, then a column. Returns
    // the tick it is there and Space can go down.
    unsigned fly(int to, bool late) {
        if (to == s.selector)
            return t + late;
        unsigned tick = t;
        for (int at = s.selector; at != to; tick += ride_ticks) {
            const int way = at / columns != to / columns ? (to / columns < at / columns ? 0 : 1)
                                                         : (to % columns < at % columns ? 2 : 3);
            note(true, at, arrow[way], tick);
            at = beside(at, way);
        }
        s.selector = uint8_t(to);
        return tick;
    }

    // Game::drop_selector: Space on a platform sets it off when stone lies
    // on exactly one of its ways, or makes it wait for a direction key.
    Result drop_on(int cell, int way, bool late) {
        if (cell == s.player || !platform(s.tiles[cell]))
            return impossible;
        const unsigned tick = fly(cell, late);
        if (tick >= deadline())
            return impossible;
        const uint8_t here = s.tiles[cell];
        unsigned ways = 0;
        for (int w = 0; w < 4; ++w)
            if (goes(here, w) && stone(tile(beside(cell, w), tick)))
                ways |= 1u << w;
        if (ways && !(ways & (ways - 1))) { // one way only
            if (ways != 1u << way)
                return impossible;
            note(true, cell, space, tick);
            return ride(cell, way, tick, true);
        }
        if (one_way(here))
            return impossible; // the selector is called back: withdraw
        if (!goes(here, way) || !stone(tile(beside(cell, way), tick + 1)) || tick + 1 >= deadline())
            return impossible;
        note(true, cell, space, tick);
        note(true, cell, arrow[way], tick + 1);
        return ride(cell, way, tick + 1, true);
    }

    // Space where drop_selector calls the selector back: the player's own
    // cell, a cell that is no platform, or a one-way platform that cannot go.
    Result withdraw_to_player(bool late) {
        int best = -1;
        unsigned best_tick = UINT_MAX;
        for (int cell = 0; cell < cells; ++cell) {
            const unsigned tick = t + ride_ticks * distance(s.selector, cell) +
                                  (late && cell == s.selector);
            if (tick >= best_tick)
                continue;
            const uint8_t here = tile(cell, tick);
            bool back = cell == s.player || !platform(here);
            if (!back && one_way(here)) {
                int w = 0;
                while (!goes(here, w))
                    ++w;
                back = !stone(tile(beside(cell, w), tick));
            }
            if (back) {
                best = cell;
                best_tick = tick;
            }
        }
        const unsigned tick = fly(best, late);
        note(true, best, space, tick);
        s.mode = 0;
        s.space_down = true;
        return advance(tick + 1);
    }
};

// States are kept as bytes: the changing tiles, then the rest. Crumbling
// floors are kept by their age, so states are equal whenever they are.
class Keys {
public:
    explicit Keys(const Level& level)
        : level(level), size((level.changing.size() + 1) / 2 + 15) {}
    const Level& level;
    const std::size_t size;
    std::vector<uint8_t> make(const State& s, unsigned tick) const {
        std::vector<uint8_t> key(size, 0);
        for (std::size_t n = 0; n < level.changing.size(); ++n) {
            const uint8_t code = uint8_t(std::strchr(codes, s.tiles[level.changing[n]]) - codes);
            key[n / 2] |= uint8_t(n & 1 ? code << 4 : code);
        }
        uint8_t* rest = &key[(level.changing.size() + 1) / 2];
        rest[0] = s.player;
        rest[1] = s.mode ? s.selector : 0;
        rest[2] = uint8_t(s.mode | s.landed << 2 | s.space_down << 3 | s.armed << 4);
        rest[3] = s.keys_a;
        rest[4] = s.keys_b;
        rest[5] = s.hourglasses;
        std::vector<Crumble> crumbles = s.crumbles;
        std::sort(crumbles.begin(), crumbles.end(),
                  [](const Crumble& a, const Crumble& b) { return a.cell < b.cell; });
        for (std::size_t n = 0; n < 4; ++n) {
            rest[6 + 2 * n] = n < crumbles.size() ? crumbles[n].cell : 255;
            rest[7 + 2 * n] = n < crumbles.size() ? uint8_t(tick - crumbles[n].start) : 0;
        }
        return key;
    }
    State read(const uint8_t* key, unsigned tick) const {
        State s;
        std::copy(level.tiles, level.tiles + cells, s.tiles);
        for (std::size_t n = 0; n < level.changing.size(); ++n)
            s.tiles[level.changing[n]] = uint8_t(codes[(key[n / 2] >> (n & 1 ? 4 : 0)) & 15]);
        const uint8_t* rest = key + (level.changing.size() + 1) / 2;
        s.player = rest[0];
        s.selector = rest[1];
        s.mode = rest[2] & 3;
        s.landed = rest[2] >> 2 & 1;
        s.space_down = rest[2] >> 3 & 1;
        s.armed = rest[2] >> 4 & 1;
        s.keys_a = rest[3];
        s.keys_b = rest[4];
        s.hourglasses = rest[5];
        for (std::size_t n = 0; n < 4 && rest[6 + 2 * n] != 255; ++n)
            s.crumbles.push_back({rest[6 + 2 * n], tick - rest[7 + 2 * n]});
        return s;
    }
};

State start_state(const Level& level) {
    State s;
    std::copy(level.tiles, level.tiles + cells, s.tiles);
    s.player = level.start;
    return s;
}

void moves(const Level& level, const State& s, std::vector<Move>& out) {
    out.clear();
    const int teleports = level.teleports.size() > 2 ? int(level.teleports.size()) - 1 : 1;
    if (!s.crumbles.empty())
        out.push_back({wait, 0, 0, 0});
    if (s.mode == 0) {
        for (int way = 0; way < 4; ++way)
            for (int k = 1; k <= teleports; ++k)
                out.push_back({press, way, 0, k});
        out.push_back({bring_out, 0, 0, 0});
    } else if (s.mode == 1) {
        for (const uint8_t cell : level.changing)
            if (platform(s.tiles[cell]) && cell != s.player)
                for (int way = 0; way < 4; ++way)
                    if (goes(s.tiles[cell], way))
                        out.push_back({drop, way, cell, 0});
        out.push_back({withdraw, 0, 0, 0});
    } else {
        for (int way = 0; way < 4; ++way)
            out.push_back({launch, way, 0, 0});
        out.push_back({show, 0, 0, 0});
    }
}

struct Search {
    bool found = false, stopped = false;
    unsigned ticks = 0;
    std::vector<Move> path;
    std::size_t states = 0;
};

// Dijkstra on ticks from the start. `timed`: states past the countdown are
// dropped.
Search search(const Level& level, bool timed, std::size_t max_states,
              const std::atomic<bool>* stop) {
    const Keys keys(level);
    struct Node {
        uint32_t parent, move, tick;
    };
    std::vector<Node> nodes;
    std::vector<uint8_t> stored;
    std::vector<uint32_t> table(1 << 16, 0);
    auto hash = [&](const uint8_t* key) {
        uint64_t h = 1469598103934665603ull;
        for (std::size_t n = 0; n < keys.size; ++n)
            h = (h ^ key[n]) * 1099511628211ull;
        return std::size_t(h ^ h >> 29);
    };
    // The node of a key, or a new one.
    auto find = [&](const std::vector<uint8_t>& key, bool& added) -> uint32_t {
        if (nodes.size() * 2 >= table.size()) {
            std::vector<uint32_t> bigger(table.size() * 2, 0);
            for (uint32_t index = 0; index < nodes.size(); ++index) {
                std::size_t slot = hash(&stored[index * keys.size]) & (bigger.size() - 1);
                while (bigger[slot])
                    slot = (slot + 1) & (bigger.size() - 1);
                bigger[slot] = index + 1;
            }
            table.swap(bigger);
        }
        std::size_t slot = hash(key.data()) & (table.size() - 1);
        for (; table[slot]; slot = (slot + 1) & (table.size() - 1))
            if (std::equal(key.begin(), key.end(),
                           stored.begin() + (table[slot] - 1) * keys.size)) {
                added = false;
                return table[slot] - 1;
            }
        added = true;
        table[slot] = uint32_t(nodes.size() + 1);
        stored.insert(stored.end(), key.begin(), key.end());
        nodes.push_back({UINT32_MAX, 0, UINT32_MAX});
        return uint32_t(nodes.size() - 1);
    };

    Search result;
    typedef std::pair<uint32_t, uint32_t> Entry; // tick, node
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> queue;
    bool added;
    const uint32_t first = find(keys.make(start_state(level), 1), added);
    nodes[first].tick = 1;
    queue.push({1, first});
    uint32_t best = UINT32_MAX, best_parent = 0, best_move = 0;
    std::vector<Move> options;
    while (!queue.empty()) {
        const Entry entry = queue.top();
        queue.pop();
        if (entry.first != nodes[entry.second].tick)
            continue;
        if (entry.first >= best)
            break;
        const State state = keys.read(&stored[entry.second * keys.size], entry.first);
        moves(level, state, options);
        for (const Move& move : options) {
            Play play(level, state, entry.first, timed, nullptr);
            const Play::Result outcome = play.apply(move);
            if (outcome == Play::impossible)
                continue;
            if (outcome == Play::cleared) {
                if (play.t < best) {
                    best = play.t;
                    best_parent = entry.second;
                    best_move = encode(move);
                }
                continue;
            }
            const uint32_t node = find(keys.make(play.s, play.t), added);
            if (play.t < nodes[node].tick) {
                nodes[node] = {entry.second, encode(move), play.t};
                queue.push({play.t, node});
            }
        }
        if (nodes.size() > max_states || (stop && stop->load(std::memory_order_relaxed))) {
            result.stopped = true;
            break;
        }
    }
    result.states = nodes.size();
    if (best == UINT32_MAX || result.stopped)
        return result;
    result.found = true;
    result.ticks = best;
    result.path.push_back(decode(best_move));
    for (uint32_t node = best_parent; node != first; node = nodes[node].parent)
        result.path.push_back(decode(nodes[node].move));
    std::reverse(result.path.begin(), result.path.end());
    return result;
}
} // namespace

const char* verdict_name(Solution::Verdict verdict) {
    switch (verdict) {
    case Solution::solvable: return "solvable";
    case Solution::too_slow: return "too slow";
    case Solution::unsolvable: return "unsolvable";
    default: return "unknown";
    }
}

Solution solve(const uint8_t* record, std::size_t max_states, const std::atomic<bool>* stop) {
    const Level level = read_level(record);
    Solution solution;
    solution.time_limit = level.time_limit - 1;
    // In time, with the hourglasses taken on the way; failing that, at all.
    Search found = search(level, true, max_states, stop);
    solution.states = found.states;
    solution.verdict = Solution::solvable;
    if (!found.found && !found.stopped) {
        found = search(level, false, max_states, stop);
        solution.states = std::max(solution.states, found.states);
        solution.verdict = Solution::too_slow;
    }
    if (!found.found) {
        solution.verdict = found.stopped ? Solution::unknown : Solution::unsolvable;
        return solution;
    }
    // The moves again, this time with their keys.
    State state = start_state(level);
    unsigned tick = 1;
    for (const Move& move : found.path) {
        Play play(level, state, tick, false, &solution.steps);
        if (play.apply(move) == Play::impossible)
            throw std::logic_error("The solver's path does not play again");
        state = play.s;
        tick = play.t;
    }
    solution.ticks = tick;
    return solution;
}
// --- The proof ---

namespace {
// What the game does on its next tick, as far as the solution's keys go
// (Game::play_tick): whether it obeys keys, with the mover on which cell.
// `rest`: not the tick a platform lands, where drop_selector would read the
// tile beside.
struct Next {
    bool obeys = false, selector = false, rest = false;
    int cell = 0;
};
Next next_tick(Machine& m) {
    const Game g(m);
    Next next;
    if (g.teleport_animates())
        return next;
    next.selector = g.selector_out();
    const unsigned x = next.selector ? g.selector_x() : g.player_x();
    const unsigned y = next.selector ? g.selector_y() : g.player_y();
    const unsigned speed = g.speed();
    const auto approach = [&](unsigned at, unsigned target) {
        return at == target ? at : at < target ? at + speed : at - speed;
    };
    if (approach(x, g.target_x()) != g.target_x() || approach(y, g.target_y()) != g.target_y())
        return next;
    const int column = int8_t(g.player_column() + g.step_column());
    const int row = int8_t(g.player_row() + g.step_row());
    next.cell = row * columns + column;
    // The tiles after this tick's Crumbling::animate.
    const auto tile = [&](int c, int r) -> uint8_t {
        if (c < 0 || c >= columns || r < 0 || r >= rows)
            return 'L';
        uint8_t here = GameNames(m).board()[r * columns + c];
        for (int slot = 0; slot < 4; ++slot)
            if (g.crumble_active()[slot] && g.crumble_column()[slot] == c &&
                g.crumble_row()[slot] == r) {
                const uint8_t frame = uint8_t(g.crumble_frame()[slot] - 1);
                if (frame < 0x80)
                    here = g.crumble_tiles()[frame];
            }
        return here;
    };
    const uint8_t going = g.platform_direction();
    if (going) {
        const int way = going == 1 ? 0 : going == 2 ? 1 : going == 4 ? 2 : 3;
        if (stone(tile(column + way_columns[way], row + way_rows[way])))
            return next; // it rides on
    } else if (!next.selector) {
        if (g.teleporting() >= 0x80)
            return next;
        const uint8_t here = tile(column, row);
        if (here == 'S' || (here == 'Q' && g.teleport_armed()))
            return next;
    }
    next.rest = !going;
    next.obeys = true;
    return next;
}

// A run of the game from the start, kept as a --keys file of stone_play:
// lines `tick scan` and `tick text WORD`, applied before that tick as the
// player applies them.
struct Script {
    Machine m;
    std::string keys;
    uint8_t scan = 0;
    unsigned calls = 0;
    void press(uint8_t code) {
        if (code == scan)
            return;
        static const char hex[] = "0123456789abcdef";
        keys += std::to_string(calls) + " " + hex[code >> 4] + hex[code & 15] + "\n";
        scan = code;
        if (m.reads_characters() && code < 0x80)
            m.type_key(code);
    }
    void type(const std::string& word) {
        keys += std::to_string(calls) + " text " + word + "\n";
        for (const char c : word)
            m.type(uint8_t(c));
    }
    void tick() {
        m.tick(scan);
        ++calls;
    }
};

// From the start of the program to the first tick of a level: any key past
// the title pictures, F2 in the menu, the password and Enter, any key past
// the level's page. Keys that come too soon are pressed again.
class Navigation {
public:
    explicit Navigation(const std::string& password) : password(password) {}
    void next(Script& s) {
        const unsigned at = s.calls;
        const uint8_t shift = at / 16 % 2 ? 0xaa : 0x2a; // a key that does nothing else
        switch (phase) {
        case 0:
            if (s.m.showing != Machine::menu) {
                s.press(shift);
                return;
            }
            phase = 1;
            since = at;
            // fall through
        case 1:
            if (!s.m.reads_characters()) {
                const unsigned cycle = (at - since) % 40;
                if (cycle == 30 || cycle == 32)
                    s.press(cycle == 30 ? 0x3c : 0xbc); // F2
                return;
            }
            phase = 2;
            since = at;
            s.type(password);
            return;
        case 2:
            if (at == since + 1)
                s.press(0x1c); // Enter
            else if (at == since + 3)
                s.press(0x9c);
            else if (at > since + 3 && !s.m.reads_characters())
                phase = 3;
            return;
        default: s.press(shift);
        }
    }

private:
    std::string password;
    int phase = 0;
    unsigned since = 0;
};
} // namespace

Proof prove(const Package& graphics, const LevelSet& set, unsigned level,
            const Solution& solution) {
    Proof proof;
    if (solution.verdict != Solution::solvable && solution.verdict != Solution::too_slow) {
        proof.problem = "there is no solution to prove";
        return proof;
    }
    const Assets assets(graphics, set, graphics);
    const std::string password = password_letters(set, level);
    const unsigned most = 2000000; // ticks of the whole run
    unsigned first_play = 0;       // the run's tick that is the level's first
    // A first run finds that tick; the second plays the level from it.
    for (int run = 0; run < 2; ++run) {
        Script script;
        Machine& m = script.m;
        assets.install(m);
        m.levels = set.count;
        m.load_file = [](const std::string&, Bytes&) { return false; };
        m.save_file = [](const std::string&, const Bytes&) {};
        m.start();
        Navigation navigation(password);
        while (run == 0 || script.calls < first_play) {
            if (m.finished() || script.calls > most) {
                proof.problem = "the level did not start";
                return proof;
            }
            navigation.next(script);
            script.tick();
            if (run == 0 && m.level_ticks >= 1) {
                first_play = script.calls - 1;
                break;
            }
        }
        if (run == 0)
            continue;
        const Game g(m);
        if (g.level() != level) {
            proof.problem = "the password started level " + std::to_string(g.level() + 1);
            return proof;
        }
        const uint8_t lives = g.lives();
        std::size_t done = 0;
        unsigned skip = 0;
        bool holding = false;
        for (;;) {
            uint8_t key = uint8_t(script.scan | 0x80); // let go
            if (holding && g.teleporting() < 0x80 && !g.teleport_animates())
                holding = false; // out of the teleport
            if (skip)
                --skip;
            else if (holding)
                key = space;
            else if (done < solution.steps.size()) {
                const SolverStep& step = solution.steps[done];
                if (step.offer) {
                    const unsigned offered = step.cell % columns | (step.cell / columns) << 8;
                    if (g.teleport_animates() && g.teleport_continue() == Game::offer &&
                        g.selector_cell() == offered) {
                        key = space;
                        holding = true;
                        ++done;
                    }
                } else {
                    const Next next = next_tick(m);
                    if (next.obeys && next.selector == step.selector && next.cell == step.cell &&
                        m.level_ticks + 1 >= step.tick &&
                        (step.scan != space || !step.selector || next.rest)) {
                        if (script.scan == step.scan)
                            key = uint8_t(step.scan | 0x80); // down already: up first
                        else {
                            key = step.scan;
                            ++done;
                            if (step.scan == space && !step.selector)
                                skip = 24; // the selector comes out (bring_out_selector)
                        }
                    }
                }
            }
            script.press(key);
            script.tick();
            if (g.level() != level) {
                proof.cleared = true;
                break;
            }
            if (g.lives() != lives || m.finished() || script.calls > most) {
                proof.problem = "lost at level tick " + std::to_string(m.level_ticks) + ", after " +
                                std::to_string(done) + " of " +
                                std::to_string(solution.steps.size()) + " keys";
                if (done < solution.steps.size())
                    proof.problem += "; the next was due at tick " +
                                     std::to_string(solution.steps[done].tick);
                break;
            }
        }
        proof.ticks = m.level_ticks;
        proof.run_ticks = script.calls;
        proof.keys = script.keys;
    }
    return proof;
}
} // namespace stone
