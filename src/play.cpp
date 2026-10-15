// SDL host for the game. All game behavior is the readable program of game.hpp
// on stone::Machine; this file only presents frames, feeds keys, plays the songs
// the game asks its driver for, and stores the score file.
//   stone_play [package] [--free] [--ticks N] [--screenshot FILE.bmp]
// The asset package (made by stone_assets) defaults to stoneage.dat beside the
// player: the free asset set. If the original game's files are found in a
// folder original_data, they are used instead, unless --free is given.
#define SDL_MAIN_HANDLED
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include "assets.hpp"
#include "package.hpp"
#include "audio.hpp"
#include "host_clock.hpp"
#include "host_sdl.hpp"
#include "keyboard.hpp"
#include "machine.hpp"
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>

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
// The original game's own graphics, levels and music, for those who have its
// files: SA_V1_00.EXE and the eight CMF songs in a folder original_data
// beside the player, or beside the folder the player is in. They are packed
// once into original.dat beside the player. Empty when they are not there.
std::string original_package(const std::string& base) {
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
}
} // namespace

int main(int argc, char** argv) {
    try {
        std::string path, screenshot;
        long scripted_ticks = -1;
        bool free_set = false;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--ticks" && i + 1 < argc)
                scripted_ticks = std::stol(argv[++i]);
            else if (arg == "--screenshot" && i + 1 < argc)
                screenshot = argv[++i];
            else if (arg == "--free")
                free_set = true;
            else if (!arg.empty() && arg[0] != '-')
                path = arg;
            else
                throw std::runtime_error(
                    "Usage: stone_play [package] [--free] [--ticks N] [--screenshot FILE.bmp]");
        }
        const bool scripted = scripted_ticks >= 0;
        // Game files come from the asset package; new scores are stored beside
        // the player.
        const char* base = SDL_GetBasePath();
        const std::string score_directory = base ? base : "";
        if (path.empty() && !free_set)
            path = original_package(score_directory);
        const stone::Package package(path.empty() ? score_directory + "stoneage.dat" : path);
        if (!package.packed())
            throw std::runtime_error("Asset package not found. It is built by stone_assets.");
        const stone::Assets assets(package);

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO))
            throw std::runtime_error(SDL_GetError());
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        if (!SDL_CreateWindowAndRenderer("Stone Age", 960, 600,
                                         SDL_WINDOW_RESIZABLE | (scripted ? SDL_WINDOW_HIDDEN : 0),
                                         &window, &renderer))
            throw std::runtime_error(SDL_GetError());
        auto texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
                                         SDL_TEXTUREACCESS_STREAMING, 320, 200);
        if (!texture)
            throw std::runtime_error(SDL_GetError());
        stone::configure_game_output(renderer, texture);

        // Music: the game asks the FX driver (INT 64h) to load, play and stop songs.
        std::unique_ptr<stone::MusicSynth> music;
        SDL_AudioStream* stream = nullptr;
        std::string song_name;
        auto play_song = [&]() {
            stone::Bytes file;
            if (!package.read(song_name, file))
                return;
            auto song = stone::parse_song(file);
            if (music)
                music->replace_song(song);
            else {
                music.reset(new stone::MusicSynth(song));
                SDL_AudioSpec spec = {SDL_AUDIO_S16, 1, music->sample_rate()};
                stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec,
                                                   nullptr, nullptr);
                if (stream)
                    SDL_ResumeAudioStreamDevice(stream);
                else
                    std::cerr << "Audio unavailable: " << SDL_GetError() << "\n";
            }
            if (stream)
                SDL_ClearAudioStream(stream);
        };

        stone::Machine machine;
        assets.install(machine);
        const auto now = std::time(nullptr);
        const auto local = std::localtime(&now);
        machine.seconds = scripted || !local ? 0 : unsigned(local->tm_sec);
        machine.driver = [&](stone::Machine& cpu) {
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
        machine.load_file = [&](const std::string& name, stone::Bytes& data) {
            // Without a saved table the game keeps the one built into it.
            return read(score_directory + name, data);
        };
        machine.save_file = [&](const std::string& name, const stone::Bytes& data) {
            stone::write_file(score_directory + name, data);
        };
        machine.start(); // the game ends through INT 21h/4Ch

        stone::HostTickClock clock;
        uint64_t previous = SDL_GetTicksNS();
        uint8_t scan = 0;
        bool quit = false, focused = true, typing = false;
        long ticks = 0;
        stone::Image image;
        while (!quit && !machine.finished()) {
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
            if (focused)
                clock.add(current - previous, scripted);
            previous = current;
            bool advanced = image.rgb.empty();
            while (clock.next() && !machine.finished()) {
                machine.tick(scan);
                ++ticks;
                advanced = true;
            }
            if (advanced) {
                image = machine.frame();
                if (!SDL_UpdateTexture(texture, nullptr, image.rgb.data(), 320 * 3))
                    throw std::runtime_error(SDL_GetError());
            }
            SDL_SetRenderDrawColor(renderer, 16, 16, 16, 255);
            SDL_RenderClear(renderer);
            SDL_RenderTexture(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
            if (stream && !scripted)
                while (SDL_GetAudioStreamQueued(stream) < music->sample_rate() / 5) {
                    auto samples = music->generate(music->sample_rate() / 20);
                    if (!SDL_PutAudioStreamData(stream, samples.data(), int(samples.size() * 2)))
                        throw std::runtime_error(SDL_GetError());
                }
            if (scripted && ticks >= scripted_ticks)
                break;
            if (!scripted)
                SDL_Delay(1);
        }
        if (!screenshot.empty())
            stone::write_bmp(screenshot, image);
        if (scripted)
            std::cout << "SDL host ran " << ticks << " ticks.\n";
        if (stream)
            SDL_DestroyAudioStream(stream);
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        SDL_Quit();
        return 1;
    }
}
