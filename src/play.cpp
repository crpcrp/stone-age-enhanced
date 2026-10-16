// SDL host for the game. All game behavior is the readable program of game.hpp
// on stone::Machine; this file only presents frames, feeds keys, plays the songs
// the game asks its driver for, and stores the score file.
//   stone_play [package] [--free] [--original] [--fresh] [--ticks N]
//              [--keys FILE] [--screenshot FILE.bmp]
// The asset package (made by stone_assets) defaults to stoneage.dat beside the
// player: the free asset set. If the original game's files are found in a
// folder original_data, Options can choose the original's graphics and music
// in its place, each on its own (STONEAGE.ENH), and the map selector's list
// of level sets its maps; --free leaves them out for this run. That list also
// offers the level packs in a folder levels/ beside the player (ENHANCEMENTS.md,
// E7). The original maps keep their scores in STONEAGE.HI, the free ones in
// FREEMAPS.HI, a pack in scores/<crc>.HI.
// Enhanced mode follows the setting saved in STONEAGE.ENH (on without one);
// --original turns it off for this run without changing the saved setting.
// --keys gives a scripted run (--ticks) its keys: lines `tick scan`, the scan
// byte in hex (80h added for a release) latched from that tick on and typed
// while the game reads characters, `tick tap x y`, a tap at that pixel of the
// game's picture, as a finger's, or `tick text WORD`, typed. stone_solve
// writes such files (ENHANCEMENTS.md, E8). A scripted run looks for no pars.
// The level editor (E9) saves packs into levels/; a scripted run, or one
// with --fresh, keeps them in memory instead.
// --fresh plays as if nothing were saved yet: no STONEAGE.ENH, no scores.
// A gamepad and taps on the screen's lines send keys too (ENHANCEMENTS.md,
// E6); in the browser the page's buttons send them (stone_key, stone_type).
// In the browser the scores, settings and original.dat are kept in /save,
// which web/saves.js keeps in IndexedDB; the page exports and imports them
// as a zip (stone_export_saves, stone_import_saves). There original.dat is
// packed from a zip of the original game the player adds on the page
// (stone_add_original, web/original.js).
#define SDL_MAIN_HANDLED
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include "assets.hpp"
#include "content.hpp"
#include "levels.hpp"
#include "package.hpp"
#include "zip.hpp"
#include "settings.hpp"
#include "audio.hpp"
#include "gamepad.hpp"
#include "host_clock.hpp"
#include "host_sdl.hpp"
#include "keyboard.hpp"
#include "machine.hpp"
#include "solver.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <sys/stat.h>
#include <unistd.h>
#else
#include <mutex>
#include <thread>
#endif

namespace {
// Unicode -> code page 437, 80h..A0h: the characters name entry accepts.
const unsigned extended[] = {0xc7, 0xfc, 0xe9, 0xe2, 0xe4, 0xe0, 0xe5, 0xe7, 0xea,   0xeb,  0xe8,
                             0xef, 0xee, 0xec, 0xc4, 0xc5, 0xc9, 0xe6, 0xc6, 0xf4,   0xf6,  0xf2,
                             0xfb, 0xf9, 0xff, 0xd6, 0xdc, 0xa2, 0xa3, 0xa5, 0x20a7, 0x192, 0xe1};
bool read(const std::string& name, stone::Bytes& data) {
    std::ifstream file(name, std::ios::binary);
    if (!file)
        return false;
    data.assign(std::istreambuf_iterator<char>(file), {});
    return true;
}
// What DOS character input (INT 21h/07h) returns for a key press. Printable
// characters arrive separately as text input.
void type_key(stone::Machine& machine, const SDL_KeyboardEvent& key) {
    if (key.key == SDLK_BACKSPACE || key.key == SDLK_ESCAPE || key.key == SDLK_TAB)
        machine.type(uint8_t(key.key));
    else if (key.key == SDLK_RETURN || key.key == SDLK_KP_ENTER)
        machine.type(13);
    else if ((key.scancode >= SDL_SCANCODE_F1 && key.scancode <= SDL_SCANCODE_F10) ||
             (key.scancode >= SDL_SCANCODE_INSERT && key.scancode <= SDL_SCANCODE_UP)) {
        machine.type(0); // extended key: zero, then the scan code
        machine.type(stone::dos_scan(key.scancode));
    }
}
void type_text(stone::Machine& machine, const char* utf8) {
    const auto* text = reinterpret_cast<const unsigned char*>(utf8);
    for (int n = 0; text[n]; ++n) {
        unsigned unicode = text[n];
        if (unicode >= 0xc0 && unicode < 0xe0 && text[n + 1])
            unicode = ((unicode & 31) << 6) | (text[++n] & 63);
        else if (unicode >= 0xe0 && unicode < 0xf0 && text[n + 1] && text[n + 2]) {
            unicode = ((unicode & 15) << 12) | ((text[n + 1] & 63) << 6) | (text[n + 2] & 63);
            n += 2;
        }
        if (unicode >= 32 && unicode < 127)
            machine.type(uint8_t(unicode));
        else
            for (unsigned k = 0; k < 33; ++k)
                if (unicode == extended[k])
                    machine.type(uint8_t(128 + k));
    }
}
#ifdef __EMSCRIPTEN__
const std::string save_directory = "/save/";
// What Export saves and Import saves carry: everything the player stores,
// and the files in these folders with these endings, the level packs and
// their score tables (E7).
const char* const save_files[] = {"STONEAGE.ENH", "STONEAGE.HI", "FREEMAPS.HI", "original.dat"};
const struct {
    const char *folder, *ending;
} save_folders[] = {{"levels", ".txt"}, {"scores", ".HI"}};
#endif

bool same_name(const std::string& a, const std::string& b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                      [](char x, char y) { return std::toupper(x) == std::toupper(y); });
}

// The files in `folder` whose names end in `ending` (in any case), sorted.
std::vector<std::string> files_in(const std::string& folder, const std::string& ending) {
    std::vector<std::string> found;
    int count = 0;
    char** names = SDL_GlobDirectory(folder.c_str(), ("*" + ending).c_str(),
                                     SDL_GLOB_CASEINSENSITIVE, &count);
    if (!names)
        return found;
    for (int n = 0; n < count; ++n)
        found.push_back(names[n]);
    SDL_free(names);
    std::sort(found.begin(), found.end());
    return found;
}

// The original game's own graphics, levels and music, for those who have its
// files: SA_V1_00.EXE and the eight CMF songs in a folder original_data
// beside the player, or beside the folder the player is in. They are packed
// once into original.dat beside the player. Empty when they are not there.
std::string find_original_package(const std::string& base) {
#ifdef __EMSCRIPTEN__
    // In the browser there is no folder to look in: the page packs
    // original.dat from the zip the player adds (add_original).
    stone::Bytes unused;
    return read(base + "original.dat", unused) ? base + "original.dat" : "";
#else
    for (const std::string folder : {base + "original_data/", base + "../original_data/"}) {
        stone::Bytes unused;
        if (!read(folder + "SA_V1_00.EXE", unused))
            continue;
        const std::string package = base + "original.dat";
        // One packed before the texts were assets is packed again.
        if (read(package, unused) && stone::Package(package).read("texts.txt", unused))
            return package;
        try {
            std::cout << "Preparing the original game files from " << folder << " ...\n";
            stone::pack_original(folder + "SA_V1_00.EXE", package);
            return package;
        } catch (const std::exception& e) {
            std::cerr << "The original game files could not be used: " << e.what() << "\n";
        }
    }
    return "";
#endif
}

#ifndef __EMSCRIPTEN__
// Not original (ENHANCEMENTS.md, E8): maps of the level set played that have
// no par in STONEAGE.ENH yet, solved one after another on a thread of their
// own while the game goes on. The browser has no threads: there only the free
// set's maps have a par (assets/pars.txt), and those an imported STONEAGE.ENH
// brings.
class ParFinder {
public:
    struct Found {
        unsigned level;
        stone::Settings::Par par;
    };
    ParFinder(const stone::LevelSet& set, const std::vector<unsigned>& levels)
        : thread([this, set, levels] { run(set, levels); }) {}
    ~ParFinder() {
        stop = true;
        thread.join();
    }
    ParFinder(const ParFinder&) = delete;
    ParFinder& operator=(const ParFinder&) = delete;
    // What has been found since the last call.
    std::vector<Found> take() {
        std::lock_guard<std::mutex> lock(mutex);
        std::vector<Found> taken;
        taken.swap(found);
        return taken;
    }

private:
    void run(const stone::LevelSet& set, const std::vector<unsigned>& levels) {
        for (const unsigned level : levels) {
            const stone::Solution solution = stone::solve(set, level - 1, 2000000, &stop);
            if (stop)
                return;
            Found result = {level, stone::Settings::Par()};
            switch (solution.verdict) {
            case stone::Solution::solvable:
                result.par.kind = stone::Settings::Par::solvable;
                result.par.ticks = solution.ticks;
                break;
            case stone::Solution::unknown: result.par.kind = stone::Settings::Par::unknown; break;
            default: result.par.kind = stone::Settings::Par::unsolvable; // also too slow
            }
            std::lock_guard<std::mutex> lock(mutex);
            found.push_back(result);
        }
    }
    std::atomic<bool> stop{false};
    std::mutex mutex;
    std::vector<Found> found;
    std::thread thread; // last: it starts once the rest is there
};
#endif

// Everything the loop uses. The desktop calls step() in a loop; in the browser
// it runs once per frame.
struct Player {
    std::string screenshot;
    long scripted_ticks = -1;
    struct Key {
        long tick;
        uint8_t scan; // or 0 for a tap at x, y, or for `text` typed
        float x, y;
        std::string text;
    };
    std::vector<Key> keys;
    bool scripted = false, fresh = false;
    std::string score_directory;
    std::unique_ptr<stone::Package> free_package, original_package;
    stone::Bytes settings_file;
    // Each part from the set STONEAGE.ENH chooses for it.
    const stone::Package* parts[stone::Settings::parts] = {};
    // The level set played (E7), Machine::pack, and the CRC of its records.
    uint32_t maps_crc = 0;
    std::unique_ptr<stone::LevelSet> played;
    std::unique_ptr<stone::Assets> assets;
    // The pars of the level set played (E8): whether to look for the missing
    // ones, and on the desktop the search for them.
    bool pars_wanted = false;
#ifndef __EMSCRIPTEN__
    std::unique_ptr<ParFinder> pars;
#endif
    void find_pars();
    // STONEAGE.ENH as the host changed it, written where the game writes it.
    void store_settings();
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::unique_ptr<stone::GameView> view;
    std::unique_ptr<stone::MusicSynth> music;
    SDL_AudioStream* stream = nullptr;
    std::string song_name;
    stone::Machine machine;
    stone::HostTickClock clock;
    uint64_t previous = 0;
    uint8_t scan = 0;
    // Keys sent by a gamepad, a button of the page or a tap, latched one a
    // tick, so that a press and its release both reach the game.
    std::deque<uint8_t> queued;
    std::unique_ptr<stone::Gamepad> gamepad;
    bool quit = false, focused = true, typing = false;
    // In the browser the game stands still while the canvas does not have the
    // keyboard, as before the first click (stone_focus).
    bool paused = false;
    long ticks = 0;
    std::size_t next_key = 0;
    stone::Image image;

    Player(int argc, char** argv);
    ~Player();
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;
    void choose_assets();
    // Level packs (E7) live here, as text files.
    std::string pack_folder() const { return score_directory + "levels/"; }
    // Packs the level editor saved (E9) in a scripted run or with --fresh,
    // which keep them here instead (rule 5); they are read before levels/.
    std::map<std::string, stone::Bytes> kept_packs;
    std::string save_pack(const std::string& file, const stone::Bytes& data);
    // A pack read, or one of the built-in sets; throws when it does not read.
    stone::LevelSet level_set(const std::string& pack) const;
    // What the map selector's list offers (Machine::level_sets).
    std::vector<stone::Machine::LevelSetEntry> level_sets() const;
    // STONEAGE.HI is the original's file, so only its maps' scores go
    // there: the free maps' go to FREEMAPS.HI, a pack's to scores/<crc>.HI.
    std::string score_file(const std::string& name) const {
        if (name != "STONEAGE.HI" || machine.pack == stone::Settings::original_maps)
            return score_directory + name;
        if (machine.pack == stone::Settings::free_maps)
            return score_directory + "FREEMAPS.HI";
        char crc[16];
        std::snprintf(crc, sizeof crc, "%08x", unsigned(maps_crc));
        return score_directory + "scores/" + crc + ".HI";
    }
    void play_song();
    // A key pressed or released other than on the keyboard; typed too while
    // the game reads characters.
    void key(uint8_t code, bool down);
    // A tap at a pixel of the game's picture: the nearest of the screen's
    // lines (Machine::lines) within a few pixels acts.
    void tap(float x, float y);
    // The keyboard and every other key let go, as the window loses focus.
    void release_keys();
    // One pass of the loop: events, ticks, the picture and sound. False when
    // the run is over.
    bool step();
    void finish();
};

Player::Player(int argc, char** argv) {
    std::string path;
    bool free_set = false, original = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--ticks" && i + 1 < argc)
            scripted_ticks = std::stol(argv[++i]);
        else if (arg == "--keys" && i + 1 < argc) {
            std::ifstream file(argv[++i]);
            if (!file)
                throw std::runtime_error(std::string("Cannot read ") + argv[i]);
            long tick;
            std::string scan;
            while (file >> tick >> scan) {
                Key key = {tick, 0, 0, 0, ""};
                if (scan == "tap")
                    file >> key.x >> key.y;
                else if (scan == "text")
                    file >> key.text;
                else
                    key.scan = uint8_t(std::stoul(scan, nullptr, 16));
                keys.push_back(key);
            }
            std::stable_sort(keys.begin(), keys.end(),
                             [](const Key& a, const Key& b) { return a.tick < b.tick; });
        } else if (arg == "--screenshot" && i + 1 < argc)
            screenshot = argv[++i];
        else if (arg == "--free")
            free_set = true;
        else if (arg == "--original")
            original = true;
        else if (arg == "--fresh")
            fresh = true;
        else if (!arg.empty() && arg[0] != '-')
            path = arg;
        else
            throw std::runtime_error(
                "Usage: stone_play [package] [--free] [--original] [--fresh] [--ticks N] "
                "[--keys FILE] [--screenshot FILE.bmp]");
    }
    scripted = scripted_ticks >= 0;
    // Game files come from the asset packages; new scores and settings
    // are stored beside the player, in the browser in /save.
    const char* base = SDL_GetBasePath();
#ifdef __EMSCRIPTEN__
    score_directory = save_directory;
#else
    score_directory = base ? base : "";
#endif
    free_package.reset(new stone::Package(
        path.empty() ? std::string(base ? base : "") + "stoneage.dat" : path));
    if (!free_package->packed())
        throw std::runtime_error("Asset package not found. It is built by stone_assets.");
    if (path.empty() && !free_set) {
        const std::string original = find_original_package(score_directory);
        try {
            if (!original.empty())
                original_package.reset(new stone::Package(original));
        } catch (const std::exception& e) { // the free set still plays
            std::cerr << "The original game files could not be used: " << e.what() << "\n";
        }
    }
    // STONEAGE.ENH, empty when there is none: every default. A scripted
    // run keeps what it saves in memory, so it never changes the
    // player's progress (ENHANCEMENTS.md, rule 5).
    if (!fresh)
        read(score_directory + stone::Settings::file_name, settings_file);
    choose_assets();

    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | (scripted ? SDL_WINDOW_HIDDEN : 0);
#ifdef __EMSCRIPTEN__
    // Keys reach the game only while the canvas has them, so the rest of the
    // page keeps its own. The page sizes the canvas in device pixels
    // (web/shell.html), so whole multiples stay sharp on any screen. The
    // window is made for WebGL at once: recreated for the renderer, it would
    // drop the page's size on a high-density screen.
    SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");
    flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_OPENGL;
    paused = true; // until the page's Click to play
#endif
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO))
        throw std::runtime_error(SDL_GetError());
    // A scripted run takes no keys but its own. Without gamepads the game
    // still plays.
    if (!scripted) {
        if (SDL_InitSubSystem(SDL_INIT_GAMEPAD))
            gamepad.reset(new stone::Gamepad(machine, [this](uint8_t code) { queued.push_back(code); }));
        else
            std::cerr << "Gamepads unavailable: " << SDL_GetError() << "\n";
    }
    if (!SDL_CreateWindowAndRenderer("Stone Age", 960, 600, flags, &window, &renderer))
        throw std::runtime_error(SDL_GetError());
    view.reset(new stone::GameView(renderer));

    assets->install(machine);
    machine.original_assets = original_package != nullptr;
    machine.change_assets = [this](stone::Machine& cpu) {
        choose_assets();
        assets->install(cpu);
    };
    machine.level_sets = [this] { return level_sets(); };
    machine.save_pack = [this](const std::string& file, const stone::Bytes& data) {
        return save_pack(file, data);
    };
    const auto now = std::time(nullptr);
    const auto local = std::localtime(&now);
    machine.seconds = scripted || !local ? 0 : unsigned(local->tm_sec);
    // Music: the game asks the FX driver (INT 64h) to load, play and stop songs.
    machine.driver = [this](stone::Machine& cpu) {
        if (cpu.ax == 0) // load the file named at BX:CX
            song_name = cpu.text(cpu.bx, cpu.cx);
        else if (cpu.ax == 1) // play
            play_song();
        else if (!music)
            return;
        else if (cpu.ax == 0x1b) // the music fades out, this fast
            music->fade_step(cpu.bx);
        else if (cpu.ax == 0x19)
            music->fade_out();
        else if (cpu.ax == 5) // silence
            music->reset();
    };
    machine.load_file = [this](const std::string& name, stone::Bytes& data) {
        if (name == stone::Settings::file_name) {
            data = settings_file;
            return !data.empty();
        }
        // Without a saved table the game keeps the one built into it.
        return !fresh && read(score_file(name), data);
    };
    machine.save_file = [this](const std::string& name, const stone::Bytes& data) {
        if (name == stone::Settings::file_name) {
            settings_file = data;
            if (scripted)
                return;
        }
        if (fresh)
            return;
        const std::string path = score_file(name);
        if (path.compare(score_directory.size(), 7, "scores/") == 0)
            SDL_CreateDirectory((score_directory + "scores").c_str());
        stone::write_file(path, data);
#ifdef __EMSCRIPTEN__
        // Into IndexedDB at once, so it is kept even if the tab closes next.
        EM_ASM({ Module.storeSaves(); });
#endif
    };
    machine.enhanced = !original && stone::Settings(settings_file).enhanced();
    machine.display = stone::Settings(settings_file).display();
    machine.start(); // the game ends through INT 21h/4Ch
    previous = SDL_GetTicksNS();
}

Player::~Player() {
    gamepad.reset();
    if (stream)
        SDL_DestroyAudioStream(stream);
    view.reset();
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
}

void Player::choose_assets() {
    const stone::Settings settings(settings_file);
    for (unsigned n = 0; n < stone::Settings::parts; ++n)
        parts[n] = original_package && settings.original(stone::Settings::Part(n))
                       ? original_package.get()
                       : free_package.get();
    // The level set: a pack that no longer reads gives way to the maps of
    // the original, or without its files to the free ones.
    std::string pack = settings.pack();
    if (pack == stone::Settings::original_maps && !original_package)
        pack = stone::Settings::free_maps; // the default without the original's files
    std::unique_ptr<stone::LevelSet> maps;
    try {
        maps.reset(new stone::LevelSet(level_set(pack)));
    } catch (const std::exception& e) {
        std::cerr << "The level set " << pack << " is not played: " << e.what() << "\n";
        pack = original_package ? stone::Settings::original_maps : stone::Settings::free_maps;
        maps.reset(new stone::LevelSet(level_set(pack)));
    }
    assets.reset(new stone::Assets(*parts[stone::Settings::images], *maps,
                                   *parts[stone::Settings::music]));
    const bool built_in =
        pack == stone::Settings::free_maps || pack == stone::Settings::original_maps;
    machine.levels = maps->count;
    machine.pack = pack;
    machine.pack_name = built_in ? "" : maps->name;
    if (maps->crc() != maps_crc) { // another level set: its own pars
#ifndef __EMSCRIPTEN__
        pars.reset();
#endif
        pars_wanted = !scripted; // a scripted run stays the same every time
    }
    maps_crc = maps->crc();
    played = std::move(maps);
}

// Between ticks, where the game holds no settings of its own, once enhanced
// mode is on: the pars the free set comes with (assets/pars.txt) go into
// STONEAGE.ENH, and on the desktop those of every other map are searched for,
// and go there as they are found.
void Player::find_pars() {
    if (pars_wanted && machine.enhanced) {
        pars_wanted = false;
        stone::Settings settings(settings_file);
        stone::Bytes file;
        bool added = false;
        if (machine.pack == stone::Settings::free_maps && free_package->read("pars.txt", file)) {
            const stone::Settings known(file);
            for (unsigned level = 1; level <= played->count; ++level) {
                const stone::Settings::Par par = known.par(maps_crc, level);
                if (par.kind != stone::Settings::Par::none &&
                    settings.par(maps_crc, level).kind == stone::Settings::Par::none) {
                    settings.set_par(maps_crc, level, par);
                    added = true;
                }
            }
        }
        if (added) {
            settings_file = settings.bytes();
            store_settings();
        }
#ifndef __EMSCRIPTEN__
        std::vector<unsigned> levels;
        for (unsigned level = 1; level <= played->count; ++level)
            if (settings.par(maps_crc, level).kind == stone::Settings::Par::none)
                levels.push_back(level);
        if (!levels.empty())
            pars.reset(new ParFinder(*played, levels));
#endif
    }
#ifndef __EMSCRIPTEN__
    if (!pars)
        return;
    const std::vector<ParFinder::Found> found = pars->take();
    if (found.empty())
        return;
    stone::Settings settings(settings_file);
    for (const ParFinder::Found& result : found)
        settings.set_par(maps_crc, result.level, result.par);
    settings_file = settings.bytes();
    store_settings();
#endif
}

void Player::store_settings() {
    if (fresh)
        return;
    stone::write_file(score_directory + stone::Settings::file_name, settings_file);
#ifdef __EMSCRIPTEN__
    EM_ASM({ Module.storeSaves(); });
#endif
}

stone::LevelSet Player::level_set(const std::string& pack) const {
    if (pack == stone::Settings::free_maps ||
        (pack == stone::Settings::original_maps && original_package)) {
        const stone::Package& package =
            pack == stone::Settings::free_maps ? *free_package : *original_package;
        return stone::read_levels(package.file("levels.txt"), "levels.txt", false);
    }
    if (pack == stone::Settings::original_maps)
        throw std::runtime_error("the original game's files are not there");
    stone::Bytes file;
    const auto kept = kept_packs.find(pack);
    if (kept != kept_packs.end())
        file = kept->second;
    else if (pack.find_first_of("/\\") != std::string::npos || !read(pack_folder() + pack, file))
        throw std::runtime_error("levels/" + pack + " cannot be read");
    stone::LevelSet set = stone::read_levels(file, pack, true);
    if (set.name.empty()) // the file's name without .txt
        set.name = pack.size() > 4 && same_name(pack.substr(pack.size() - 4), ".txt")
                       ? pack.substr(0, pack.size() - 4)
                       : pack;
    return set;
}

std::vector<stone::Machine::LevelSetEntry> Player::level_sets() const {
    std::vector<std::string> packs = {stone::Settings::free_maps};
    if (original_package)
        packs.push_back(stone::Settings::original_maps);
    std::vector<std::string> files = files_in(pack_folder(), ".txt");
    for (const auto& kept : kept_packs)
        if (std::none_of(files.begin(), files.end(),
                         [&](const std::string& file) { return same_name(file, kept.first); }))
            files.push_back(kept.first);
    std::sort(files.begin(), files.end());
    for (const std::string& file : files)
        packs.push_back(file);
    std::vector<stone::Machine::LevelSetEntry> sets;
    for (const std::string& pack : packs) {
        stone::Machine::LevelSetEntry entry = {pack, pack, "", 0, 0};
        try {
            const stone::LevelSet set = level_set(pack);
            entry.name = pack == stone::Settings::free_maps       ? "Free maps"
                         : pack == stone::Settings::original_maps ? "Original maps"
                                                                  : set.name;
            entry.levels = set.count;
            entry.crc = set.crc();
        } catch (const stone::LevelError& e) { // its line, without the file's name
            entry.problem = e.what();
            entry.problem = entry.problem.substr(entry.problem.find(", line ") + 2);
        } catch (const std::exception& e) {
            entry.problem = e.what();
        }
        sets.push_back(entry);
    }
    return sets;
}

std::string Player::save_pack(const std::string& file, const stone::Bytes& data) {
    if (file.empty() || file.find_first_of("/\\:") != std::string::npos)
        return "a pack's file name may not name a folder";
    if (scripted || fresh) {
        kept_packs[file] = data;
        return "";
    }
    SDL_CreateDirectory(pack_folder().c_str());
    try {
        stone::write_file(pack_folder() + file, data);
    } catch (const std::exception& e) {
        return e.what();
    }
#ifdef __EMSCRIPTEN__
    EM_ASM({ Module.storeSaves(); });
#endif
    return "";
}

void Player::play_song() {
    stone::Bytes file;
    if (!parts[stone::Settings::music]->read(song_name, file))
        return;
    auto song = stone::parse_song(file);
    if (music)
        music->replace_song(song);
    else {
        music.reset(new stone::MusicSynth(song));
        SDL_AudioSpec spec = {SDL_AUDIO_S16, 1, music->sample_rate()};
        stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr,
                                           nullptr);
        if (stream)
            SDL_ResumeAudioStreamDevice(stream);
        else
            std::cerr << "Audio unavailable: " << SDL_GetError() << "\n";
    }
    if (stream)
        SDL_ClearAudioStream(stream);
}

bool Player::step() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT)
            quit = true;
        else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
            // Not original: without focus the game stands still, and a held
            // key is released because its key-up would never arrive.
            focused = false;
            release_keys();
        } else if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
            focused = true;
            clock.reset();
        } else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
            const uint8_t code = stone::dos_scan(event.key.scancode);
            if (code && !event.key.repeat)
                scan = event.type == SDL_EVENT_KEY_DOWN ? code : uint8_t(code | 128);
            if (event.type == SDL_EVENT_KEY_DOWN && machine.reads_characters())
                type_key(machine, event.key);
        } else if (event.type == SDL_EVENT_TEXT_INPUT && machine.reads_characters())
            type_text(machine, event.text.text);
        else if (event.type == SDL_EVENT_FINGER_DOWN && !scripted) {
            // A touch on the picture (E6); in the browser, the canvas.
            int width = 0, height = 0;
            float x, y;
            SDL_GetWindowSize(window, &width, &height);
            if (view->to_game(event.tfinger.x * width, event.tfinger.y * height, x, y))
                tap(x, y);
        } else if (gamepad)
            gamepad->handle(event);
    }
    if (gamepad)
        gamepad->update(SDL_GetTicksNS());
    // Before its first tick the game has not taken the keyboard yet: that
    // is not typing.
    if (ticks && machine.reads_characters() != typing) {
        typing = !typing;
        if (typing)
            SDL_StartTextInput(window);
        else
            SDL_StopTextInput(window);
#ifdef __EMSCRIPTEN__
        // A phone shows its keyboard only for a text field (web/shell.html).
        EM_ASM({ if (Module.setTyping) Module.setTyping($0, $1); }, typing,
               machine.showing == stone::Machine::password);
#endif
    }
    find_pars();
    const auto current = SDL_GetTicksNS();
    if (focused && !paused)
        clock.add(current - previous, scripted);
    previous = current;
    bool advanced = image.rgb.empty();
    while (clock.next() && !machine.finished()) {
        for (; next_key < keys.size() && keys[next_key].tick <= ticks; ++next_key) {
            const Key& key = keys[next_key];
            if (key.scan) {
                scan = key.scan;
                if (machine.reads_characters() && scan < 0x80) // as a key of the page
                    machine.type_key(scan);
            } else if (!key.text.empty())
                for (const char c : key.text)
                    machine.type(uint8_t(c));
            else
                tap(key.x, key.y);
        }
        if (!queued.empty()) {
            scan = queued.front();
            queued.pop_front();
        }
        machine.tick(scan);
        ++ticks;
        advanced = true;
    }
    if (advanced) {
        image = machine.frame();
        view->update(image);
    }
    view->present(machine.enhanced, machine.display);
    if (stream && !scripted)
        while (SDL_GetAudioStreamQueued(stream) < music->sample_rate() / 5) {
            auto samples = music->generate(music->sample_rate() / 20);
            if (!SDL_PutAudioStreamData(stream, samples.data(), int(samples.size() * 2)))
                throw std::runtime_error(SDL_GetError());
        }
    return !quit && !machine.finished() && !(scripted && ticks >= scripted_ticks);
}

void Player::key(uint8_t code, bool down) {
    queued.push_back(down ? code : uint8_t(code | 128));
    if (down && machine.reads_characters())
        machine.type_key(code);
}

void Player::tap(float x, float y) {
    const stone::Machine::Line* nearest = nullptr;
    float best = 6; // pixels from a line, at most
    for (const stone::Machine::Line& line : machine.lines) {
        const float dx = std::max({float(line.x) - x, 0.0f, x - float(line.x + line.width)});
        const float dy = std::max({float(line.y) - y, 0.0f, y - float(line.y + line.height)});
        const float distance = std::max(dx, dy);
        if (distance <= best) {
            best = distance;
            nearest = &line;
        }
    }
    if (!nearest)
        return;
    if (nearest->key) {
        key(nearest->key, true);
        key(nearest->key, false);
    } else
        machine.tapped = int(nearest - machine.lines.data());
}

void Player::release_keys() {
    queued.clear();
    if (scan)
        scan |= 128;
}

void Player::finish() {
    if (!screenshot.empty())
        stone::write_bmp(screenshot, image);
    if (scripted)
        std::cout << "SDL host ran " << ticks << " ticks.\n";
}

#ifdef __EMSCRIPTEN__

// Add the original game: /tmp/original.zip holds SA_V1_00.EXE and its eight
// songs, at its top or in one folder, with names in any case. They are
// written to /tmp/original_data/ under the names the executable gives them
// and packed into /save/original.dat, as find_original_package does on the
// desktop; only original.dat is kept.
void add_original() {
    stone::Bytes zip;
    if (!read("/tmp/original.zip", zip))
        throw std::runtime_error("The zip could not be read.");
    std::remove("/tmp/original.zip");
    const std::map<std::string, stone::Bytes> entries = stone::read_zip(zip);
    zip.clear();
    auto folder_of = [](const std::string& entry) {
        return entry.substr(0, entry.find_last_of("/\\") + 1);
    };
    const stone::Bytes* exe = nullptr;
    std::string folder;
    for (const auto& entry : entries) {
        const std::string in = folder_of(entry.first);
        if (same_name(entry.first.substr(in.size()), "SA_V1_00.EXE") &&
            std::count_if(in.begin(), in.end(), [](char c) { return c == '/' || c == '\\'; }) <= 1) {
            exe = &entry.second;
            folder = in;
            break;
        }
    }
    if (!exe)
        throw std::runtime_error("The zip holds no SA_V1_00.EXE, at its top or in one folder.");
    if (exe->size() != 452788 || stone::crc32(exe->data(), exe->size()) != 0x0e5149ac)
        throw std::runtime_error("The zip's SA_V1_00.EXE is not the original Stone Age, version "
                                 "1.00 (452,788 bytes).");
    std::map<std::string, const stone::Bytes*> files = {{"SA_V1_00.EXE", exe}};
    for (const std::string& song : stone::song_files(*exe)) {
        for (const auto& entry : entries)
            if (same_name(folder_of(entry.first), folder) &&
                same_name(entry.first.substr(folder_of(entry.first).size()), song))
                files[song] = &entry.second;
        if (!files.count(song))
            throw std::runtime_error("The zip lacks the song " + song +
                                     "; the eight .CMF songs go beside SA_V1_00.EXE.");
    }
    const std::string loose = "/tmp/original_data/", packed = "/tmp/original.dat";
    auto clean_up = [&] {
        for (const auto& file : files)
            std::remove((loose + file.first).c_str());
        rmdir(loose.c_str());
    };
    stone::Bytes package;
    mkdir(loose.c_str(), 0777);
    try {
        for (const auto& file : files)
            stone::write_file(loose + file.first, *file.second);
        stone::pack_original(loose + "SA_V1_00.EXE", packed);
        stone::Package check(packed); // it opens, or the game would not start
        package = stone::read_file(packed);
    } catch (const std::exception& e) {
        clean_up();
        std::remove(packed.c_str());
        throw std::runtime_error(std::string("The original game files could not be used: ") +
                                 e.what());
    }
    clean_up();
    std::remove(packed.c_str());
    stone::write_file(save_directory + "original.dat", package);
}

// The page asked to add the original game; the next frame does it, as packing
// runs the game's code on a fiber, which the browser's frame can swap to.
bool original_asked = false;
// The answer of the last add: empty, or what went wrong.
std::string original_problem;
bool game_ended = false;
Player* web_player = nullptr;

// The browser calls this once a frame. When the game ends (INT 21h/4Ch) the
// page offers to play again in place of closing; the frames go on, so the
// original game can still be added.
void step_frame(void* argument) {
    Player* const player = static_cast<Player*>(argument);
    if (original_asked) {
        original_asked = false;
        try {
            add_original();
            original_problem.clear();
        } catch (const std::exception& e) {
            original_problem = e.what();
        }
        player->previous = SDL_GetTicksNS(); // the game does not catch up on the wait
        EM_ASM({ Module.originalPacked(); });
    }
    if (game_ended)
        return;
    try {
        if (player->step())
            return;
        player->finish();
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
    game_ended = true;
    EM_ASM({
        if (Module.gameEnded)
            Module.gameEnded();
    });
}

// The answer of the last export or import: empty, or what went wrong.
std::string saves_problem;
#endif
} // namespace

#ifdef __EMSCRIPTEN__
// Export saves: a zip of the files in /save, written to /tmp/saves.zip.
extern "C" EMSCRIPTEN_KEEPALIVE const char* stone_export_saves() {
    try {
        std::map<std::string, stone::Bytes> files;
        for (const char* name : save_files) {
            stone::Bytes data;
            if (read(save_directory + name, data))
                files[name] = data;
        }
        for (const auto& folder : save_folders)
            for (const std::string& name : files_in(save_directory + folder.folder, folder.ending)) {
                stone::Bytes data;
                const std::string path = std::string(folder.folder) + "/" + name;
                if (read(save_directory + path, data))
                    files[path] = data;
            }
        if (files.empty())
            throw std::runtime_error("There are no saves yet.");
        stone::write_file("/tmp/saves.zip", stone::write_zip(files));
        saves_problem.clear();
    } catch (const std::exception& e) {
        saves_problem = e.what();
    }
    return saves_problem.c_str();
}

// Import saves: the save files in /tmp/saves.zip replace those in /save.
// Names are matched in any case, in a folder or not; other files are left
// out, and saves the zip does not hold stay as they are.
extern "C" EMSCRIPTEN_KEEPALIVE const char* stone_import_saves() {
    try {
        stone::Bytes zip;
        if (!read("/tmp/saves.zip", zip))
            throw std::runtime_error("The zip could not be read.");
        std::map<std::string, stone::Bytes> saves;
        for (auto& entry : stone::read_zip(zip)) {
            const std::size_t slash = entry.first.find_last_of("/\\");
            const std::string name = entry.first.substr(slash + 1);
            for (const char* save : save_files)
                if (same_name(name, save))
                    saves[save] = std::move(entry.second);
            // A pack or a score table in its folder (E7), in a folder or not.
            const std::string in = slash == std::string::npos ? "" : entry.first.substr(0, slash);
            const std::string folder = in.substr(in.find_last_of("/\\") + 1);
            for (const auto& kept : save_folders) {
                const std::size_t ending = std::strlen(kept.ending);
                if (same_name(folder, kept.folder) && name.size() > ending &&
                    same_name(name.substr(name.size() - ending), kept.ending))
                    saves[std::string(kept.folder) + "/" + name] = std::move(entry.second);
            }
        }
        if (saves.empty())
            throw std::runtime_error("The zip holds no saves (STONEAGE.ENH, STONEAGE.HI, "
                                     "FREEMAPS.HI, original.dat, levels/ or scores/).");
        const auto original = saves.find("original.dat");
        if (original != saves.end()) { // it must open, or the game would not start
            stone::write_file("/tmp/original.dat", original->second);
            try {
                stone::Package("/tmp/original.dat");
            } catch (const std::exception&) {
                std::remove("/tmp/original.dat");
                throw std::runtime_error("The zip's original.dat is damaged.");
            }
            std::remove("/tmp/original.dat");
        }
        for (const auto& folder : save_folders)
            SDL_CreateDirectory((save_directory + folder.folder).c_str());
        for (const auto& save : saves)
            stone::write_file(save_directory + save.first, save.second);
        saves_problem.clear();
    } catch (const std::exception& e) {
        saves_problem = e.what();
    }
    return saves_problem.c_str();
}

// Add the original game from /tmp/original.zip: asked here, done at the next
// frame, which then calls Module.originalPacked (web/original.js).
extern "C" EMSCRIPTEN_KEEPALIVE void stone_add_original() { original_asked = true; }
extern "C" EMSCRIPTEN_KEEPALIVE const char* stone_original_problem() {
    return original_problem.c_str();
}

// The canvas got or lost the keyboard (web/shell.html). Without it the game
// stands still, as the desktop player does without focus, and a held key is
// released because its key-up goes to the page.
extern "C" EMSCRIPTEN_KEEPALIVE void stone_focus(int focus) {
    Player* const player = web_player;
    if (!player)
        return;
    player->paused = !focus;
    if (!focus)
        player->release_keys();
    if (focus)
        player->clock.reset();
}

// A button of the page pressed or released (web/shell.html; ENHANCEMENTS.md,
// E6): a scan byte, as a key of the keyboard sends it.
extern "C" EMSCRIPTEN_KEEPALIVE void stone_key(int code, int down) {
    if (web_player && code > 0 && code < 128)
        web_player->key(uint8_t(code), down != 0);
}

// Text typed in the page's text field, while the game reads characters.
extern "C" EMSCRIPTEN_KEEPALIVE void stone_type(const char* utf8) {
    if (web_player && web_player->machine.reads_characters())
        type_text(web_player->machine, utf8);
}
#endif

int main(int argc, char** argv) {
    try {
#ifdef __EMSCRIPTEN__
        // The player lives as long as the page; main returns at once.
        web_player = new Player(argc, argv);
        emscripten_set_main_loop_arg(step_frame, web_player, 0, false);
#else
        Player player(argc, argv);
        while (player.step())
            if (!player.scripted)
                SDL_Delay(1);
        player.finish();
#endif
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        SDL_Quit();
        return 1;
    }
}
