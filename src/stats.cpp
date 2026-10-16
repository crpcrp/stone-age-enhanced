// The statistics (ENHANCEMENTS.md, E4). Not original: enhanced mode only, F8
// in the menu. What is counted while a level is played is kept in Machine and
// added to STONEAGE.ENH once the level ends: a line per map of each level set
// and one line of totals. The screen shows them over the score screen's
// picture, in its font and red, 14 pixels a line.
#include "game.hpp"
#include <algorithm>
#include <string>
#include <vector>

namespace stone {
namespace {
using namespace scan;
const unsigned per_page = 10, columns = 31;
const unsigned scores_picture = 0x328d0; // as in screens.cpp

// Ticks of 70 a second as hours, minutes and seconds: 1:02:03, or 2:03.
std::string clock_time(uint64_t ticks) {
    const uint64_t seconds = ticks / 70;
    const auto two = [](uint64_t value) {
        return std::string(value < 10 ? "0" : "") + std::to_string(value);
    };
    if (seconds < 3600)
        return std::to_string(seconds / 60) + ':' + two(seconds % 60);
    return std::to_string(seconds / 3600) + ':' + two(seconds / 60 % 60) + ':' + two(seconds % 60);
}

// `left` and `right` on one line of `columns` characters.
std::string spread(const std::string& left, const std::string& right) {
    const std::size_t used = left.size() + right.size();
    return left + std::string(used < columns ? columns - used : 1, ' ') + right;
}

// `text` right-aligned in `width` characters.
std::string pad(const std::string& text, std::size_t width) {
    return std::string(text.size() < width ? width - text.size() : 0, ' ') + text;
}

class Statistics {
public:
    explicit Statistics(const Game& game) : g(game), screen(game.screen) {}
    void run();

private:
    struct Map {
        unsigned number; // 1-100
        Settings::MapStats stats;
        Settings::Best best;
        Settings::Par par; // E8
    };
    void gather();
    void sort();
    unsigned pages() const {
        return 1 + std::max(1u, unsigned(played.size() + per_page - 1) / per_page);
    }
    std::vector<std::string> lines(unsigned page) const;
    void draw(unsigned page) const;

    const Game& g;
    const Screen& screen;
    Settings::Totals totals;
    std::vector<Map> played; // the maps of this level set with an attempt
    unsigned cleared = 0;    // maps of this level set with a best (E3)
    bool by_attempts = false;
};

void Statistics::run() {
    gather();
    g.load_picture(scores_picture);
    screen.set_target(g.red(), 15, 1);
    screen.show_picture();
    screen.save_page();
    g.unpack(g.font_resource(), g.work_buffer());
    unsigned page = 0;
    draw(page);
    screen.fade_in();
    g.read_scan();
    for (;;) {
        screen.wait(1);
        g.music.tick();
        const uint8_t key = g.read_scan();
        if (!g.scan_change() || key >= 0x80)
            continue;
        int to = int(page);
        switch (key) {
        case arrow_left:
        case arrow_up:
        case page_up: to -= 1; break;
        case arrow_right:
        case arrow_down:
        case page_down: to += 1; break;
        case home: to = 0; break;
        case end_key: to = int(pages()) - 1; break;
        case tab:
        case key_s:
            by_attempts = !by_attempts;
            sort();
            to = 1; // the list, from its first page
            break;
        case esc: screen.fade_out(); return;
        default: continue;
        }
        to = std::min(std::max(to, 0), int(pages()) - 1);
        page = unsigned(to);
        draw(page);
    }
}

void Statistics::gather() {
    const Settings settings = g.load_settings();
    totals = settings.totals();
    const uint32_t set = g.level_set();
    for (unsigned number = 1; number <= g.m.levels; ++number) { // E7: a pack may have fewer
        const Map map = {number, settings.map_stats(set, number), settings.best(set, number),
                         settings.par(set, number)};
        if (map.best.cleared())
            ++cleared;
        if (map.stats.attempts)
            played.push_back(map);
    }
    sort();
}

// By number, or by attempts, the most first (the hardest maps), then by number.
void Statistics::sort() {
    std::sort(played.begin(), played.end(), [&](const Map& a, const Map& b) {
        if (by_attempts && a.stats.attempts != b.stats.attempts)
            return a.stats.attempts > b.stats.attempts;
        return a.number < b.number;
    });
}

std::vector<std::string> Statistics::lines(unsigned page) const {
    const std::string number = std::to_string(page + 1) + '/' + std::to_string(pages());
    std::vector<std::string> text;
    if (page == 0) {
        text.push_back(spread("STATISTICS", number));
        const std::pair<const char*, std::string> rows[] = {
            {"TIME PLAYED", clock_time(totals.ticks)},
            {"GAMES STARTED", std::to_string(totals.games)},
            {"LEVELS CLEARED", std::to_string(totals.cleared)},
            {"STEPS WALKED", std::to_string(totals.steps)},
            {"HOURGLASSES TAKEN", std::to_string(totals.hourglasses)},
            {"KEYS COLLECTED", std::to_string(totals.keys)},
            {"GATES OPENED", std::to_string(totals.gates)},
            {"TELEPORTS USED", std::to_string(totals.teleports)},
            {"PLATFORMS MOVED", std::to_string(totals.platforms)},
        };
        for (const auto& row : rows)
            text.push_back(spread(row.first, row.second));
        text.push_back(spread("THESE MAPS", std::to_string(cleared) + " OF " +
                                                std::to_string(g.m.levels) + " CLEARED"));
        return text;
    }
    text.push_back(spread(by_attempts ? "MAPS BY ATTEMPTS" : "MAPS BY NUMBER", number));
    text.push_back("MAP TRIES WON LOST QUIT  BEST   PAR");
    if (played.empty())
        text.push_back("NONE PLAYED YET");
    const std::size_t first = (page - 1) * per_page;
    for (std::size_t n = first; n < std::min(played.size(), first + per_page); ++n) {
        const Map& map = played[n];
        text.push_back(pad(std::to_string(map.number), 3) +
                       pad(std::to_string(map.stats.attempts), 6) + pad(std::to_string(map.stats.clears), 4) +
                       pad(std::to_string(map.stats.lives_lost), 5) +
                       pad(std::to_string(map.stats.give_ups), 5) +
                       pad(map.best.cleared() ? tick_seconds(map.best.ticks) : "-", 6) +
                       pad(map.par.kind == Settings::Par::solvable ? tick_seconds(map.par.ticks)
                           : map.par.kind == Settings::Par::unsolvable ? "NONE"
                                                                       : "-", 6));
    }
    return text;
}

void Statistics::draw(unsigned page) const {
    screen.restore_page();
    const uint8_t colour = g.text_colour();
    g.text_colour() = 15; // the score table's red
    const std::vector<std::string> text = lines(page);
    for (std::size_t n = 0; n < text.size(); ++n) {
        screen.move_to(32, unsigned(7 + 14 * n));
        for (const char character : text[n])
            screen.write(uint8_t(character));
    }
    g.text_colour() = colour;
    screen.flip();
}
} // namespace

void Game::statistics() const { Statistics(*this).run(); }

void Game::record_stats(Outcome outcome, bool first) const {
    if (!m.recording)
        return;
    const uint32_t set = level_set();
    const unsigned map = level() + 1u;
    Settings settings = load_settings();
    Settings::MapStats stats = settings.map_stats(set, map);
    ++stats.attempts;
    if (outcome == completed)
        ++stats.clears;
    else if (outcome == gave_up)
        ++stats.give_ups;
    else
        ++stats.lives_lost; // the time ran out, or Q
    stats.ticks += m.level_ticks;
    settings.set_map_stats(set, map, stats);
    Settings::Totals totals = settings.totals();
    totals.ticks += m.level_ticks;
    totals.games += first;
    totals.cleared += outcome == completed;
    totals.steps += m.tally.steps;
    totals.hourglasses += m.tally.hourglasses;
    totals.keys += m.tally.keys;
    totals.gates += m.tally.gates;
    totals.teleports += m.tally.teleports;
    totals.platforms += m.tally.platforms;
    settings.set_totals(totals);
    save_settings(settings);
}
} // namespace stone
