# Stone Age Enhanced

An enhanced version of the 1992 DOS puzzle game **Stone Age** (Eclipse),
built on a C++ reconstruction of its executable. With enhanced mode off the
game matches the original tick for tick. With it on (the default) it adds a
map selector, best times, statistics, display presets, gamepad support and
touch controls. It runs on Windows and in a web browser.

This is an unofficial project, not affiliated with or endorsed by the
original authors or rights holders.

**Play in the browser: <https://crpcrp.github.io/stone-age-enhanced/>**

## What is in this repository

- `src/`: the game, reconstructed as readable C++, and the SDL3 host.
- `assets/`: a free set of pictures, songs, 100 levels and texts made for
  this project, so the game is complete without the original's files.
- `web/`: the page around the browser build.
- Nothing of the original game's content. If you own the original, the
  player can use its files (see [BUILDING.md](BUILDING.md#the-original-game)).

## Build

There are no prebuilt desktop downloads. You need CMake 3.25+ and a C++14
compiler; on Windows, Visual Studio 2022 or its Build Tools with the C++
workload has both. The first build downloads SDL3.

```sh
cmake --preset default
cmake --build --preset default
```

The player, `stoneage.dat` and (on Windows) `SDL3.dll` are written to
`build/`.

## How it was checked

With enhanced mode off, the reconstruction was run beside the original
executable in an emulator under the same key presses and compared tick by
tick: every frame, palette, music call and score-file write, through the
menus and all 100 original levels. The music driver's AdLib register writes
were compared separately. These tests need the original game and are not in
this repository.

## Documentation

- [BUILDING.md](BUILDING.md): building, the web version, running, tests and
  the source layout.
- [ENHANCEMENTS.md](ENHANCEMENTS.md): what enhanced mode adds and what is
  planned.
- [ASSETS.md](ASSETS.md): every asset file and how the game uses it.
- [LEGAL.md](LEGAL.md): what is derived from the original, what is new, and
  third-party licences.

## Rights

Stone Age and its code, artwork, music, levels and texts belong to their
rights holders. The program in `src/` is derived from the original's code;
the assets in `assets/` are new work. No licence is granted. See
[LICENSE](LICENSE) and [LEGAL.md](LEGAL.md).
