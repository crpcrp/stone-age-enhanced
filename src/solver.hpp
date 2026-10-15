#pragma once
#include "levels.hpp"
#include <atomic>
#include <string>
#include <vector>

namespace stone {
class Package;
// The solver (ENHANCEMENTS.md, E8): a search over a model of one level, in
// plain C++ on its record, and the proof of what it finds in the real game.

// A key the solution presses for one tick, at a moment the game obeys keys:
// when the player (or the selector, `selector`) has reached `cell`
// (column + 20 * row), at level tick `tick` or later. `offer` is a key
// held instead while a teleport offers `cell`, with more than two
// teleports on the board.
struct SolverStep {
    bool selector = false, offer = false;
    uint8_t cell = 0, scan = 0;
    unsigned tick = 0;
};

struct Solution {
    // solvable: the exit is reached in time. too_slow: it is reached only
    // after the time runs out. unsolvable: never. unknown: the search
    // stopped at its state limit.
    enum Verdict { solvable, too_slow, unsolvable, unknown };
    Verdict verdict = unknown;
    // The level tick (Machine::level_ticks) the model clears the map at,
    // the fewest it can; and the last tick the countdown leaves without
    // hourglasses.
    unsigned ticks = 0, time_limit = 0;
    std::size_t states = 0; // states the search kept
    std::vector<SolverStep> steps;
};
const char* verdict_name(Solution::Verdict verdict);

// Solve the level whose 227-byte record (LevelSet::records) starts here.
// The search keeps at most `max_states` states, and gives up (unknown) as
// soon as `stop` is set.
Solution solve(const uint8_t* record, std::size_t max_states = 2000000,
               const std::atomic<bool>* stop = nullptr);
inline Solution solve(const LevelSet& set, unsigned level, std::size_t max_states = 2000000,
                      const std::atomic<bool>* stop = nullptr) {
    return solve(set.records.data() + level * LevelSet::record_size, max_states, stop);
}

// A solution played through Game on a Machine without a window, from the
// start of the program: past the title, menu F2 (enhanced mode off: the
// password screen), the level's password and its first page, then the
// steps. `keys` is the run as a --keys file of stone_play, which replays it
// with --fresh --free --original for the built-in free set.
struct Proof {
    bool cleared = false;
    unsigned ticks = 0;      // level ticks the game took
    unsigned run_ticks = 0;  // ticks of the whole run, for --ticks
    std::string keys, problem;
};
Proof prove(const Package& graphics, const LevelSet& set, unsigned level,
            const Solution& solution);
} // namespace stone

