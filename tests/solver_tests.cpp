// Tests of the solver (solver.hpp; ENHANCEMENTS.md, E8): a small board for
// each rule, with its verdict and, where the moves are plain, its par. Each
// solution is also proved in the real game when the asset package is given.
#include "package.hpp"
#include "settings.hpp"
#include "solver.hpp"
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace {
unsigned failures = 0;

void check(bool ok, const char* what, int line) {
    if (!ok) {
        std::cerr << "solver_tests.cpp:" << line << ": failed: " << what << "\n";
        ++failures;
    }
}
#define CHECK(condition) check(condition, #condition, __LINE__)

std::unique_ptr<stone::Package> package; // for the proofs; none without its path

// A pack of one level: these rows in the top left corner, outside around
// them, and `time` in the record (seconds: time + time / 4).
stone::LevelSet board(const std::vector<std::string>& rows, unsigned time = 50) {
    char header[80];
    std::snprintf(header, sizeof header,
                  "level 1 time %03u theme 0 palette 0 background 0 flag 0\n", time);
    std::string text = header;
    for (std::size_t row = 0; row < 11; ++row)
        text += (row < rows.size() ? rows[row] : "") +
                std::string(20 - (row < rows.size() ? rows[row].size() : 0), 'a') + "\n";
    return stone::read_levels(stone::Bytes(text.begin(), text.end()), "board.txt", true);
}

// The verdict, and for a solution its par, which the real game must match.
stone::Solution solved(const std::vector<std::string>& rows, stone::Solution::Verdict verdict,
                       unsigned par = 0, unsigned time = 50, int line = 0) {
    const stone::LevelSet set = board(rows, time);
    const stone::Solution solution = stone::solve(set, 0);
    check(solution.verdict == verdict, stone::verdict_name(solution.verdict), line);
    if (par)
        check(solution.ticks == par, ("par " + std::to_string(solution.ticks)).c_str(), line);
    if (package && solution.verdict == stone::Solution::solvable) {
        const stone::Proof proof = stone::prove(*package, set, 0, solution);
        check(proof.cleared, ("not proved: " + proof.problem).c_str(), line);
        check(proof.ticks == solution.ticks, ("proved in " + std::to_string(proof.ticks)).c_str(),
              line);
    }
    return solution;
}
#define SOLVED(...) solved(__VA_ARGS__, __LINE__)
const auto solvable = stone::Solution::solvable, too_slow = stone::Solution::too_slow,
           unsolvable = stone::Solution::unsolvable;

void walking() {
    // Three steps of 16 ticks from the first tick.
    SOLVED({"LLLLLL", "LRBBSL", "LLLLLL"}, solvable, 49, 50);
    SOLVED({"LLLLLL", "LRBLSL", "LLLLLL"}, unsolvable, 0, 50);
    SOLVED({"LLLLLL", "LRASBL", "LLLLLL"}, unsolvable, 0, 50); // stone is no floor
}

void platforms() {
    // Onto E at 17, across three cells of stone at 8 ticks each, two steps.
    SOLVED({"LLLLLLLLL", "LREAAABSL", "LLLLLLLLL"}, solvable, 73, 50);
    // C goes only up, into the wall.
    SOLVED({"LLLLLLL", "LRCABSL", "LLLLLLL"}, unsolvable, 0, 50);
    // H goes both ways: there and back again is no use, so it is a bridge.
    SOLVED({"LLLLLLLL", "LRBHAASL", "LLLLLLLL"}, solvable, 0, 50);
}

void selector() {
    // D, out of reach above the gap, is sent down into it from afar.
    SOLVED({"LLLLLL", "LLDLLL", "LRABSL", "LLLLLL"}, solvable, 0, 50);
    // G with stone above and below waits for a direction key.
    SOLVED({"LLLLLL", "LLALLL", "LLGLLL", "LRABSL", "LLLLLL"}, solvable, 0, 50);
    // Called back from a one-way platform that cannot go.
    SOLVED({"LLLLLL", "LLCLLL", "LRABSL", "LLLLLL"}, unsolvable, 0, 50);
}

void keys_and_gates() {
    SOLVED({"LLLLLLL", "LROBMSL", "LLLLLLL"}, solvable, 65, 50);
    SOLVED({"LLLLLLL", "LRPBNSL", "LLLLLLL"}, solvable, 65, 50);
    SOLVED({"LLLLLLL", "LRBBMSL", "LLLLLLL"}, unsolvable, 0, 50);
    SOLVED({"LLLLLLL", "LROBNSL", "LLLLLLL"}, unsolvable, 0, 50); // a key of the other colour
    SOLVED({"LLLLLLL", "LROMMSL", "LLLLLLL"}, unsolvable, 0, 50); // a gate takes its key
}

void cracked_floors() {
    // On the exit at 33, but it opens only when the floor behind has fallen.
    SOLVED({"LLLLL", "LRKSL", "LLLLL"}, solvable, 57, 50);
    // A floor crossed falls away: there is no way back.
    SOLVED({"LLLLLL", "LLSLLL", "LRKBLL", "LLLLLL"}, solvable, 0, 50);
    SOLVED({"LLLLLL", "LLLSLL", "LBKRLL", "LLLLLL"}, solvable, 0, 50);
    SOLVED({"LLLLLLL", "LSMRKOL", "LLLLLLL"}, unsolvable, 0, 50); // the key is beyond it
}

void time_and_hourglasses() {
    // Time 1: one second, 70 ticks; five steps take 81.
    SOLVED({"LLLLLLLL", "LRBBBBSL", "LLLLLLLL"}, too_slow, 81, 1);
    SOLVED({"LLLLLLLL", "LRJBBBSL", "LLLLLLLL"}, solvable, 81, 1);
}

void teleports() {
    // Into the first at 17, out of the other 86 ticks later, one step.
    SOLVED({"LLLLLLL", "LRQLQSL", "LLLLLLL"}, solvable, 119, 50);
    // With three, Space held while the third is offered: 43 ticks more.
    SOLVED({"LLLLLLLLL", "LRQLQLQSL", "LLLLLLLLL"}, solvable, 162, 50);
}

// The free set comes with the par of each of its levels (assets/pars.txt),
// for the browser, which cannot search. A changed level needs them found again:
//   stone_solve --pars assets/pars.txt assets/levels.txt
void free_set_pars() {
    if (!package)
        return;
    const stone::LevelSet set =
        stone::read_levels(package->file("levels.txt"), "levels.txt", false);
    stone::Bytes file;
    CHECK(package->read("pars.txt", file));
    const stone::Settings pars(file);
    unsigned found = 0;
    for (unsigned level = 1; level <= set.count; ++level)
        found += pars.par(set.crc(), level).kind == stone::Settings::Par::solvable;
    CHECK(found == 100);
}

void limits() {
    const stone::Solution solution =
        stone::solve(board({"LLLLLLLLL", "LREAAABSL", "LLLLLLLLL"}), 0, 2);
    CHECK(solution.verdict == stone::Solution::unknown);
}
} // namespace

unsigned solver_tests(const std::string& package_path) {
    if (!package_path.empty())
        package.reset(new stone::Package(package_path));
    walking();
    platforms();
    selector();
    keys_and_gates();
    cracked_floors();
    time_and_hourglasses();
    teleports();
    free_set_pars();
    limits();
    package.reset();
    if (!failures)
        std::cout << "solver_tests passed" << (package_path.empty() ? " (without proofs)" : "")
                  << "\n";
    return failures;
}
