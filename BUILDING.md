# Building

## Requirements

- CMake 3.25+ and a C++14 compiler. On Windows, Visual Studio 2022 or its
  Build Tools with the C++ workload provides both and opens the folder as a
  CMake project.
- An internet connection for the first build. CMake downloads SDL3 3.4.16
  into the build tree and checks it against a pinned SHA-256. On Windows it
  uses SDL's prebuilt development package; elsewhere an installed SDL3 3.4+
  if there is one, otherwise SDL built from source. To use a local copy, set
  `FETCHCONTENT_SOURCE_DIR_SDL3_MINGW` (Windows) or
  `FETCHCONTENT_SOURCE_DIR_SDL3`.

## Build

```sh
cmake --preset default
cmake --build --preset default
```

| Preset | Builds | Build tree |
|---|---|---|
| `default` | Player, asset packer, solver, `stone_tests` | `build/cmake` |
| `ninja` | The same with Ninja (GCC or Clang) | `build/cmake-ninja` |
| `core` | Core library, asset packer, solver and `stone_tests`; no SDL | `build/cmake-core` |
| `wasm` | The web version (below) | `build/cmake-wasm` |
| `test` | `default` plus the comparison tests (see Tests) | `build/cmake` |

On Windows, `build.ps1` runs the same presets and finds the CMake that
Visual Studio installs when none is on `PATH`:

```powershell
.\build.ps1                          # default
.\build.ps1 -Compiler C:\path\to\g++.exe   # ninja, with that compiler
.\build.ps1 -CoreOnly                # core
.\build.ps1 -Test                    # test, then runs ctest
```

Every build writes to `build/`: `stone_play` (`.exe` on Windows),
`stone_assets`, `stone_solve`, `stoneage.dat` (the packed `assets/`), `LICENSE`,
`LEGAL.md`, the `licenses/` folder and, on Windows, `SDL3.dll`.

## Run

`stone_play`, `stoneage.dat` and (on Windows) `SDL3.dll` in one folder are
all the game needs.

```
stone_play [package] [--free] [--original] [--fresh] [--ticks N]
           [--keys FILE] [--screenshot FILE.bmp]
```

| Option | Effect |
|---|---|
| `package` | Asset package to load instead of `stoneage.dat` beside the player. |
| `--original` | Enhanced mode off for this run, without changing the setting. |
| `--free` | Ignore the original game's files for this run. |
| `--fresh` | Play as if nothing were saved: no `STONEAGE.ENH`, no scores. |
| `--ticks N` | Scripted run: stop after N ticks (70 a second). Saves nothing. |
| `--keys FILE` | Keys for a scripted run: lines `tick scan` (scan byte in hex, 80h added for a release; a press is also typed while the game reads characters), `tick tap x y` (a tap at that pixel of the 320x200 picture) or `tick text WORD` (typed, as on the password screen). |
| `--screenshot FILE.bmp` | Write the last frame when the run ends. |

Files written beside the player:

- `STONEAGE.ENH`: everything enhanced mode keeps (settings, progress, bests,
  statistics); see [ENHANCEMENTS.md](ENHANCEMENTS.md).
- `STONEAGE.HI`: high scores of the original's levels, in the original's
  format.
- `FREEMAPS.HI`: high scores of the free levels, same format.
- `scores/<crc>.HI`: high scores of each level pack, same format.
- `original.dat`: the original game's files, packed (below).

Level packs go in a folder `levels/` beside the player: text files in the
format of `assets/levels.txt` with 1 to 100 levels (ENHANCEMENTS.md, E7).
L in the map selector lists them.

Audio-device errors are reported on the console; the game goes on silently.

### The solver

`stone_solve` finds the fastest way through levels and proves each solution
by playing it in the game without a window (ENHANCEMENTS.md, E8):

```
stone_solve [--keys DIR] [--states N] [--no-proof] [--package FILE]
            [--pars FILE] [--compare FILE] LEVELS.txt [LEVEL...]
```

```sh
build/stone_solve assets/levels.txt 1 34 63           # three free levels
build/stone_solve assets/levels.txt                   # all 100, about 1.5 minutes
build/stone_solve --keys solutions levels/mine.txt    # a pack, writing the solutions
```

Without LEVEL it solves every level of the file. `--package` is the asset
package the proof plays with (`stoneage.dat` beside `stone_solve`).
`--keys DIR` writes each proved solution as `DIR/levelNNN.keys`, which
`stone_play --fresh --free --original --ticks N --keys FILE` plays again for
the free set (it prints N). `--pars FILE` writes the pars found as lines of
`STONEAGE.ENH`; `--compare FILE` reports where they differ from those in a
file. The exit code is 2 when a level is not solvable in time, a proof
fails or a par differs.

`assets/pars.txt` holds the free levels' pars, which the player copies into
`STONEAGE.ENH` (the browser cannot search for them). After changing
`assets/levels.txt` or the solver, write it again:

```sh
build/stone_solve --pars assets/pars.txt assets/levels.txt
```

### The original game

To play with the original's pictures, songs, levels and texts you need your
own copy: `SA_V1_00.EXE` version 1.00 (452,788 bytes, SHA-256
`a979bc922b329297bbda2eb8b7f8375c466721bd439c9fd1b47a8db26444f927`) and its
eight `.CMF` songs. Put them in a folder `original_data/` beside the player
or one folder above it (from `build/`, that is the repository root). The
player packs them once into `original.dat` beside itself; delete that file
after changing the original files. Options F2 (setup) then chooses images
and music from either set, and the map selector's list of level sets (L)
offers the original's maps.

Do not commit or distribute these files or `original.dat`.

## Web version

Needs Emscripten (emsdk 4.0.10, as pinned in `.github/workflows/web.yml`),
Ninja, and the package from a native `core` build, since `stone_assets`
cannot run inside the wasm build:

```sh
cmake --preset core && cmake --build --preset core
emcmake cmake --preset wasm
cmake --build --preset wasm
```

The output is `build/web/` (`index.html`, `.js`, `.wasm`, `.data`, the
manifest and icons). Serve it over HTTP, for example
`python3 -m http.server -d build/web`.

Differences from the desktop:

- **No threads.** `Machine::Turns` (`machine.cpp`) switches between the host
  and the game with Emscripten fibers (`-sASYNCIFY`), so the page needs no
  `SharedArrayBuffer` or cross-origin isolation headers, which GitHub Pages
  cannot send.
- **One step a frame.** `play.cpp`'s loop body is a step function the
  browser calls once a frame (`emscripten_set_main_loop_arg`). The game
  waits while the canvas does not have focus.
- **Saves** live in `/save`, kept in IndexedDB by `web/saves.js`.
  **Export saves** and **Import saves** zip `STONEAGE.ENH`, `STONEAGE.HI`,
  `FREEMAPS.HI`, `original.dat` and the folders `levels/` and `scores/`
  (`src/zip.cpp`); importing a zip with a `levels/` folder is how level
  packs get into the browser.
- **The original game.** **Add the original game (zip)** takes a zip of
  `SA_V1_00.EXE` and its songs. `web/original.js` unpacks it in the browser,
  the executable's size and hash are checked, and it is packed into
  `/save/original.dat`. Nothing is uploaded.
- **Options.** `?original` and `?free` in the address stand for
  `--original` and `--free`.
- **Touch.** On a touch screen (`pointer: coarse`) the page adds buttons
  beside the picture, a text field for typing, and a button that leaves
  fullscreen; see ENHANCEMENTS.md, E6. `web/manifest.webmanifest` lets a
  phone add the page to its home screen.

`.github/workflows/web.yml` builds the page on every push and pull request
and, from `main`, publishes it to GitHub Pages with `LICENSE`, `LEGAL.md`
and the licence notices. Pages must be set once to deploy from Actions
(Settings → Pages → Source: GitHub Actions).

## Assets

`assets/` holds the free set; every native build packs it into
`build/stoneage.dat`. [ASSETS.md](ASSETS.md) describes each file. By hand:

```sh
stone_assets pack assets build/stoneage.dat
stone_assets export original_data/SA_V1_00.EXE some/folder
stone_assets verify original_data/SA_V1_00.EXE some/folder
```

`export` unpacks the original into the same editable files; keep that
folder out of the repository. `verify` reports whether a folder or package
still equals the original.

`stoneage.dat` is a file table and deflate-compressed contents with a byte
scramble. The scramble deters casual editing; it is not encryption.

## Tests

- `stone_tests` (`tests/`) checks `STONEAGE.ENH` reading and writing, the
  zip code, reading and writing level packs, and the solver on small
  boards. It needs nothing of the original and is built with every native
  preset. Given the asset package as its argument, as CTest does, it also
  proves the solver's solutions in the game.
- With the player, CTest also runs two scripted taps of the free set
  (`tests/taps/`) and compares their screenshots by SHA-256
  (`tests/screenshot_test.cmake`).

```sh
ctest --test-dir build/cmake -C Release
```

The comparison tests replay key scripts on the reconstruction and on the
original executable and compare every tick. They, their fixtures and the
tools that make them derive from the original and are not in this
repository. With them in `research/` and the original in `original_data/`,
the `test` preset (or `.\build.ps1 -Test`) builds and runs them.

In Claude Code cloud sessions, `.claude/hooks/session-start.sh` builds the
player in the background without a display (`build/cmake-headless`, done
when `build/cmake-headless/ready` exists) and sets SDL's `offscreen` video
and `dummy` audio drivers, so scripted runs and `stone_tests` work there.

## Source layout

| Files | Contents |
|---|---|
| `machine.*` | A small real-mode machine (memory, planar VGA, DOS and BIOS services) whose memory holds the game's variables. |
| `game.*`, `screens.cpp`, `video.cpp` | The game: a level; the title, menu and their screens; pages, sprites, colours and text. |
| `music.*`, `music_lift.inc`, `music_tables.inc` | The song format and the music driver, which outputs OPL register writes. |
| `audio.*` | Synthesis with ymfm's YM3812 emulator (`third_party/ymfm/`, one change). |
| `assets.*`, `content.*`, `package.*`, `assets_main.cpp` | The editable assets, the package format and `stone_assets`. |
| `levels.*` | Level sets as text: `levels.txt` and level packs. |
| `solver.*`, `solve_main.cpp` | The solver and its proof in the game (E8), and `stone_solve`. |
| `editor.cpp`, `level_edit.*` | The level editor (E9): its screen, and what it does to a level. |
| `selector.cpp`, `packs.cpp`, `stats.cpp`, `lines.cpp`, `settings.*` | Enhanced mode: the map selector, its list of level sets, statistics, the line cursor and `STONEAGE.ENH`. |
| `play.cpp`, `host_*.hpp`, `gamepad.*` | The SDL host: window, keys, gamepad, touch, audio, files. Nothing else in `src/` uses SDL. |
| `zip.*` | Zip reading and writing, for the web page's saves. |
| `stone.*` | Byte buffers, files, BMP output, CRC-32, SHA-256. |

`game_data.inc`, `game_names.hpp`, `keyboard.hpp`, `music_lift.inc` and
`music_tables.inc` were generated from the original by tools that are not in
this repository; edit them like any other source.

## Limits

- Of the native builds only Windows has been tested, plus the headless
  Linux build of the cloud sessions.
- The web version has been tested in Chromium only. Not yet checked by
  hand: Firefox, touch on real Android and iOS devices, gamepads in the
  browser, and adding a real zip of the original game.
- The browser keeps some shortcuts (Ctrl+W, Ctrl+T, Ctrl+N), which the game
  does not use. In fullscreen, Esc leaves fullscreen first, except in
  Chromium, where the Keyboard Lock API gives it to the game.
- Browser saves belong to one browser and site and may be cleared; use
  **Export saves**.
- At 70 ticks a second on a 60 Hz screen, one frame in six shows two ticks,
  on the desktop as in the browser.

## Distribution

This project publishes its source and the web version; there are no
desktop downloads. If you pass on a player you built, keep `LICENSE`,
`LEGAL.md` and the `licenses` folder (copied into `build/`) with it.
