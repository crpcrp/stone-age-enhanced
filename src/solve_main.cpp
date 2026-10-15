// stone_solve [options] LEVELS.txt [LEVEL...]   solve levels and prove the solutions
// (ENHANCEMENTS.md, E8). Without LEVEL, every level of the file.
//   --package FILE  the asset package the proof plays with (stoneage.dat beside this program)
//   --keys DIR      write each proved solution as DIR/levelNNN.keys, a --keys file of stone_play
//   --states N      the search's state limit (2000000)
//   --no-proof      solve only
#include "levels.hpp"
#include "package.hpp"
#include "solver.hpp"
#include <chrono>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
std::string seconds(unsigned ticks) {
    char text[32];
    std::snprintf(text, sizeof text, "%.1f s", ticks / 70.0);
    return text;
}
} // namespace

int main(int argc, char** argv) {
    try {
        std::string package_path, keys_directory, levels_path;
        std::size_t states = 2000000;
        bool proof = true;
        std::vector<unsigned> chosen;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--package" && i + 1 < argc)
                package_path = argv[++i];
            else if (arg == "--keys" && i + 1 < argc)
                keys_directory = argv[++i];
            else if (arg == "--states" && i + 1 < argc)
                states = std::stoul(argv[++i]);
            else if (arg == "--no-proof")
                proof = false;
            else if (!arg.empty() && arg[0] != '-' && levels_path.empty())
                levels_path = arg;
            else if (!arg.empty() && arg[0] != '-')
                chosen.push_back(unsigned(std::stoul(arg)));
            else
                throw std::runtime_error(
                    "Usage: stone_solve [--package FILE] [--keys DIR] [--states N] [--no-proof] "
                    "LEVELS.txt [LEVEL...]");
        }
        if (levels_path.empty())
            throw std::runtime_error("Which levels? Usage: stone_solve LEVELS.txt [LEVEL...]");
        if (package_path.empty()) {
            const std::string program = argv[0];
            const std::size_t slash = program.find_last_of("/\\");
            package_path = (slash == std::string::npos ? "" : program.substr(0, slash + 1)) +
                           "stoneage.dat";
        }
        const stone::LevelSet set =
            stone::read_levels(stone::read_file(levels_path), levels_path, true);
        if (chosen.empty())
            for (unsigned level = 1; level <= set.count; ++level)
                chosen.push_back(level);
        std::unique_ptr<stone::Package> package;
        if (proof)
            package.reset(new stone::Package(package_path));
        unsigned failures = 0;
        for (const unsigned level : chosen) {
            if (level < 1 || level > set.count)
                throw std::runtime_error("There is no level " + std::to_string(level));
            const auto begun = std::chrono::steady_clock::now();
            const stone::Solution solution = stone::solve(set, level - 1, states);
            const double took =
                std::chrono::duration<double>(std::chrono::steady_clock::now() - begun).count();
            std::cout << "level " << level << ": " << stone::verdict_name(solution.verdict);
            if (solution.verdict == stone::Solution::solvable ||
                solution.verdict == stone::Solution::too_slow)
                std::cout << ", par " << solution.ticks << " ticks (" << seconds(solution.ticks)
                          << ") of " << solution.time_limit << ", " << solution.steps.size()
                          << " keys";
            std::printf(", %zu states, %.2f s\n", solution.states, took);
            if (solution.verdict != stone::Solution::solvable)
                ++failures;
            if (!proof || solution.steps.empty())
                continue;
            const stone::Proof played = stone::prove(*package, set, level - 1, solution);
            if (played.cleared) {
                std::cout << "  proved: cleared in " << played.ticks << " ticks";
                if (played.ticks != solution.ticks)
                    std::cout << " (the model said " << solution.ticks << ")";
                std::cout << "\n";
            } else {
                std::cout << "  NOT PROVED, a solver bug: " << played.problem << "\n";
                ++failures;
            }
            if (!keys_directory.empty() && played.cleared) {
                char name[32];
                std::snprintf(name, sizeof name, "/level%03u.keys", level);
                const std::string path = keys_directory + name;
                stone::write_file(path, stone::Bytes(played.keys.begin(), played.keys.end()));
                std::cout << "  " << path << ": stone_play --fresh --free --original --ticks "
                          << played.run_ticks + 70 << " --keys " << path << "\n";
            }
        }
        return failures ? 2 : 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
