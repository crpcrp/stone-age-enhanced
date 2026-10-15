# Building

## Requirements

- CMake 3.25 or newer and a C++14 compiler. On Windows, Visual Studio 2022
  (or its Build Tools) with the C++ workload provides both, and opens the
  folder directly as a CMake project.
- An internet connection for the first build. CMake downloads SDL3 3.4.16
  into the build tree and checks it against pinned SHA-256 hashes; nothing
  else needs installing. On Windows it uses
  SDL's prebuilt development package; elsewhere it uses an installed SDL3
  when there is one and otherwise builds SDL from source. To use local
  copy, set `FETCHCONTENT_SOURCE_DIR_SDL3_MINGW` (Windows) or
  `FETCHCONTENT_SOURCE_DIR_SDL3`.

## Build

With CMake on `PATH`, from the repository root:

```powershell
cmake --preset default
cmake --build --preset default
```

Other presets: `ninja` (Ninja with GCC or Clang), `core` (the portable
core and the asset packer only, without SDL and the player) and `wasm` (the
web version, below). Plain `cmake -S . -B build/cmake` works as well.

On Windows, `build.ps1` does the same and finds the CMake that Visual Studio
installs when none is on `PATH`:

```powershell
.\build.ps1
.\build.ps1 -Compiler C:\path\to\g++.exe   # Ninja with GCC or Clang
.\build.ps1 -CoreOnly
```

Outputs are written to `build/`: `stone_play.exe`, `SDL3.dll`,
`stoneage.dat` (the packed `assets/`), `stone_assets.exe` and the legal
notices. The CMake build trees are in `build/cmake*`; later builds are
incremental.

## Web version

The browser build needs Emscripten (emsdk 4.0.10, as pinned in
`.github/workflows/web.yml`) and Ninja, and the package the `core` preset
packs, since `stone_assets` cannot run inside the browser build:

```sh
cmake --preset core && cmake --build --preset core
emcmake cmake --preset wasm
cmake --build --preset wasm
```

The page is `build/web/index.html` with `index.js`, `index.wasm` and
`index.data`; serve the folder over HTTP (`python3 -m http.server -d
build/web`) rather than opening the file. `web/shell.html` is the page
around the canvas, `web/saves.js` keeps the saves in IndexedDB and
`web/original.js` adds the original game from a zip.

`.github/workflows/web.yml` builds the page on every push and pull request
and publishes it on GitHub Pages from `main`, with `LICENSE`, `LEGAL.md`
and the notices. Pages must be set once to deploy from Actions: Settings →
Pages → Build and deployment → Source: GitHub Actions.

## Run

```powershell
.\build\stone_play.exe
```

`stone_play.exe`, `SDL3.dll` and `stoneage.dat` in one folder are all the
game needs. New scores are written beside the player: `STONEAGE.HI` for the
original maps, `FREEMAPS.HI` for the free ones.
Audio-device errors are reported in the console; the game goes on silently.

The player loads the asset packages in its own folder: `stoneage.dat`, the
free set, and `original.dat`, which it makes itself when it finds the original
game's files in a folder `original_data/` beside it or one folder above it
(the repository root). Options F2 then chooses images, maps and music from
either one; without the original there is only the free set.
`stone_play.exe --free` leaves `original.dat` out for one run. Delete
`original.dat` after changing the original files.

The original game is `SA_V1_00.EXE` (452,788 bytes, SHA-256
`a979bc922b329297bbda2eb8b7f8375c466721bd439c9fd1b47a8db26444f927`) with its
eight `.CMF` songs beside it.

## Assets

`assets/` holds the free pictures, songs, levels, texts and colours; all of
it is yours to edit, and every build packs it into `build/stoneage.dat`.
[ASSETS.md](ASSETS.md) describes each file and what the game does with it.
By hand:

```powershell
.\build\stone_assets.exe pack assets build\stoneage.dat
.\build\stone_assets.exe export original_data\SA_V1_00.EXE some\folder
.\build\stone_assets.exe verify original_data\SA_V1_00.EXE some\folder
```

`export` unpacks the original into the same editable files, to look at or to
work from; keep that folder out of the repository. `verify` reports whether a
folder or package still equals the original.

`stoneage.dat` is the game's own format: a file table and deflate-compressed
contents with a byte scramble. It keeps players from casually opening or
replacing files; it is not encryption.

## Source layout

- `src/machine.*`: a small real-mode machine (memory, planar VGA, DOS and
  BIOS services) whose memory holds the game's variables.
- `src/game.*`, `screens.cpp`, `video.cpp`: the game itself (a level; title,
  menu and their screens; pages, sprites, colours and text).
- `src/music.*`, `music_lift.inc`, `music_tables.inc`: the song format and
  the music driver; its output is OPL register writes.
- `src/audio.*`: the music synthesized with ymfm's YM3812 emulator
  (`third_party/ymfm/`, unmodified), one register write a sample.
- `src/assets.*`, `content.*`, `package.*`, `assets_main.cpp`: the editable
  assets, the package format and `stone_assets`.
- `src/play.cpp`, `host_*.hpp`: the SDL host (window, keys, audio, score
  file). The rest of `src/` does not use SDL.
- `src/zip.*`: reading and writing zip files, for the web page's saves.
- `web/`: the web page around the browser build (`shell.html`) and its
  saves in IndexedDB (`saves.js`); see `WEB_PLAN.md`.
- `src/stone.*`: byte buffers, files, BMP output, CRC-32.

`src/game_data.inc`, `game_names.hpp`, `keyboard.hpp`, `music_lift.inc` and
`music_tables.inc` were generated from the original by tools that are not
part of this repository; in a fork, edit them like any other source.

## Tests

`stone_tests` needs nothing of the original game and is built with every
preset; run it with `ctest --test-dir build/cmake -C Release` (or the build tree
of the preset used). It tests the saved settings and the zip files (`tests/`).

The comparison tests replay key scripts on the game and compare every 70 Hz
tick with a run of the original executable under the same keys. They, their
recorded fixtures and the scripts that make them are derived from the
original game and are not part of this repository. With them in `research/` and the
original files in `original_data/`, `.\build.ps1 -Test` (or the `test`
presets) builds and runs them.

In Claude Code cloud sessions, `.claude/hooks/session-start.sh` builds the
player without a display (SDL for the console only, in
`build/cmake-headless`) and sets SDL's off-screen video and dummy audio
drivers, so scripted runs (`--keys`, `--ticks`, `--screenshot`) and
`stone_tests` work there. It builds in the background while the session
starts; the build is done when `build/cmake-headless/ready` exists.

## Limits

Native builds other than Windows have not been verified; the headless Linux
build of the cloud sessions and the web build are the exceptions.

In the browser:

- The game needs a keyboard; there are no touch controls.
- The browser keeps some keys (Ctrl+W, Ctrl+T, Ctrl+N), which the game does
  not use. In fullscreen Esc leaves fullscreen first, except in Chromium,
  where the fullscreen button locks it for the game.
- Saves belong to one browser and one site, and the browser may delete
  them; **Export saves** keeps them in a zip.
- 70 ticks a second on a 60 Hz screen: one frame in six shows two ticks, as
  on the desktop.

## Distribution

This project publishes its source and the web version built from it by the
workflow; there are no desktop downloads. If you build the player and pass
it on, keep `LICENSE`, `LEGAL.md` and the `licenses` folder
(copied into `build/`) with it. See [LEGAL.md](LEGAL.md).
