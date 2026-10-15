# Stone Age reconstruction

A native C++ reconstruction of the 1992 DOS puzzle game **Stone Age**
(Eclipse), written from its executable. It plays like the original: the same
screens, controls, timing in 70 Hz ticks, AdLib music and high-score file.
The source is meant to be read, and a fork can build small enhancements on it.
It runs on Windows.

This is an unofficial project, not affiliated with or endorsed by the
original authors or rights holders.

## What is included

- The game's program, reconstructed as readable C++ (`src/`).
- A free set of pictures, songs, 100 levels with their passwords, and texts,
  made for this project, so the game is complete without the original's
  files (`assets/`). [ASSETS.md](ASSETS.md) describes every file; all of them
  can be edited.
- **Not included:** anything of the original game's content: its graphics,
  music, levels, passwords and texts.

## How it was checked

The reconstruction was compared with the original executable, run in an
emulator under the same key presses: every displayed frame and palette, music
call and score-file write, tick by tick, through the menus, the screens and
all 100 original levels. The music driver's AdLib register writes were
compared with the original driver's separately. Those tests need the original
game and are not part of this repository.

## Build

There are no prebuilt downloads: build the game from source. You need CMake
3.25+ and a C++14 compiler; on Windows, Visual Studio 2022 or its Build Tools
with the C++ workload has both. The first build downloads SDL3 and DOSBox's
OPL emulator.

```powershell
cmake --preset default
cmake --build --preset default
```

On Windows, `.\build.ps1` does the same without CMake on `PATH`. To play,
keep `stone_play.exe`, `SDL3.dll` and `stoneage.dat` from `build\` together
in one folder. [BUILDING.md](BUILDING.md) has more detail.

## Run

```powershell
.\build\stone_play.exe
```

- **In a level:** the arrow keys move; Space or Enter selects or works a
  platform; P pauses; Q retries; Esc leaves.
- **In the menus:** the function keys shown on screen.

## Playing with the original game's content

You need your own copy of the original game: `SA_V1_00.EXE` and its eight
songs (the `.CMF` files beside it). Put them in a folder named
`original_data`, either in this repository or beside `stone_play.exe`. The
player then uses the original's pictures, songs, levels, passwords and texts;
`stone_play.exe --free` plays the free set instead. Do not commit or
distribute these files, or the `original.dat` the player makes from them.

## Rights

Stone Age and its code, artwork, music, levels and texts belong to their
rights holders. The program in `src/` is derived from the original's code;
the assets in `assets/` are new work. No license is granted. See
[LICENSE](LICENSE) and [LEGAL.md](LEGAL.md).
