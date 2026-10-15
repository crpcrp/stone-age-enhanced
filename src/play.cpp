// SDL host for the game. All game behavior is the readable program of game.hpp
// on stone::Machine; this file only presents frames, feeds keys, plays the songs
// the game asks its driver for, and stores the score file.
//   stone_play [package] [--free] [--original] [--ticks N] [--keys FILE]
//              [--screenshot FILE.bmp]
// The asset package (made by stone_assets) defaults to stoneage.dat beside the
// player: the free asset set. If the original game's files are found in a
// folder original_data, Options can choose the original's graphics, maps and
// music in its place, each on its own (STONEAGE.ENH); --free leaves them out
// for this run. The original maps keep their scores in STONEAGE.HI, the free
// ones in FREEMAPS.HI.
// Enhanced mode follows the setting saved in STONEAGE.ENH (on without one);
// --original turns it off for this run without changing the saved setting.
// --keys gives a scripted run (--ticks) its keys: lines `tick scan`, the scan
// byte in hex (80h added for a release) latched from that tick on.
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
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include "assets.hpp"
#include "content.hpp"
#include "package.hpp"
#include "zip.hpp"
#include "settings.hpp"
#include "audio.hpp"
#include "host_clock.hpp"
#include "host_sdl.hpp"
#include "keyboard.hpp"
#include "machine.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <sys/stat.h>
#include <unistd.h>
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
// What Export saves and Import saves carry: everything the player stores.
const char* const save_files[] = {"STONEAGE.ENH", "STONEAGE.HI", "FREEMAPS.HI", "original.dat"};
#endif

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

// Everything the loop uses. The desktop calls step() in a loop; in the browser
// it runs once per frame.
struct Player {
    std::string screenshot;
    long scripted_ticks = -1;
    std::vector<std::pair<long, uint8_t>> keys; // tick, scan byte
    bool scripted = false;
    std::string score_directory;
    std::unique_ptr<stone::Package> free_package, original_package;
    stone::Bytes settings_file;
    // Each part from the set STONEAGE.ENH chooses for it.
    const stone::Package* parts[stone::Settings::parts] = {};
    std::unique_ptr<stone::Assets> assets;
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
    // STONEAGE.HI is the original's file, so only its maps' scores go there.
    std::string score_file(const std::string& name) const {
        const bool free_maps = parts[stone::Settings::maps] == free_package.get();
        return score_directory + (free_maps && name == "STONEAGE.HI" ? "FREEMAPS.HI" : name);
    }
    void play_song();
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
            while (file >> tick >> scan)
                keys.emplace_back(tick, uint8_t(std::stoul(scan, nullptr, 16)));
            std::stable_sort(keys.begin(), keys.end(),
                             [](const std::pair<long, uint8_t>& a,
                                const std::pair<long, uint8_t>& b) { return a.first < b.first; });
        } else if (arg == "--screenshot" && i + 1 < argc)
            screenshot = argv[++i];
        else if (arg == "--free")
            free_set = true;
        else if (arg == "--original")
            original = true;
        else if (!arg.empty() && arg[0] != '-')
            path = arg;
        else
            throw std::runtime_error(
                "Usage: stone_play [package] [--free] [--original] [--ticks N] "
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
    if (!SDL_CreateWindowAndRenderer("Stone Age", 960, 600, flags, &window, &renderer))
        throw std::runtime_error(SDL_GetError());
    view.reset(new stone::GameView(renderer));

    assets->install(machine);
    machine.original_assets = original_package != nullptr;
    machine.change_assets = [this](stone::Machine& cpu) {
        choose_assets();
        assets->install(cpu);
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
        return read(score_file(name), data);
    };
    machine.save_file = [this](const std::string& name, const stone::Bytes& data) {
        if (name == stone::Settings::file_name) {
            settings_file = data;
            if (scripted)
                return;
        }
        stone::write_file(score_file(name), data);
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
    assets.reset(new stone::Assets(*parts[stone::Settings::images], *parts[stone::Settings::maps],
                                   *parts[stone::Settings::music]));
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
            if (scan && !(scan & 128))
                scan |= 128;
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
    }
    if (machine.reads_characters() != typing) {
        typing = !typing;
        if (typing)
            SDL_StartTextInput(window);
        else
            SDL_StopTextInput(window);
    }
    const auto current = SDL_GetTicksNS();
    if (focused && !paused)
        clock.add(current - previous, scripted);
    previous = current;
    bool advanced = image.rgb.empty();
    while (clock.next() && !machine.finished()) {
        for (; next_key < keys.size() && keys[next_key].first <= ticks; ++next_key)
            scan = keys[next_key].second;
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

void Player::finish() {
    if (!screenshot.empty())
        stone::write_bmp(screenshot, image);
    if (scripted)
        std::cout << "SDL host ran " << ticks << " ticks.\n";
}

#ifdef __EMSCRIPTEN__
bool same_name(const std::string& a, const std::string& b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                      [](char x, char y) { return std::toupper(x) == std::toupper(y); });
}

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
    if (exe->size() != 452788 ||
        stone::sha256(*exe) != "a979bc922b329297bbda2eb8b7f8375c466721bd439c9fd1b47a8db26444f927")
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
            std::string name = entry.first.substr(entry.first.find_last_of("/\\") + 1);
            for (const char* save : save_files)
                if (same_name(name, save))
                    saves[save] = std::move(entry.second);
        }
        if (saves.empty())
            throw std::runtime_error("The zip holds no saves (STONEAGE.ENH, STONEAGE.HI, "
                                     "FREEMAPS.HI or original.dat).");
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
        for (const auto& save : saves)
            stone::write_file(save_directory + save.first, save.second);
        saves_problem.clear();
    } catch (const std::exception& e) {
        saves_problem = e.what();
    }
    return saves_problem.c_str();
}
#endif

#ifdef __EMSCRIPTEN__
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
    if (!focus && player->scan && !(player->scan & 128))
        player->scan |= 128;
    if (focus)
        player->clock.reset();
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
