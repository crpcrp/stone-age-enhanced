# Stone Age Enhanced

An enhanced version of the 1992 DOS puzzle game **Stone Age** (Eclipse),
built on a native C++ reconstruction written from its executable. The levels
play exactly as in the original: the same rules, movement, timing in 70 Hz
ticks, graphics, AdLib music and high-score file. Around them it adds
improvements, such as choosing any map already reached instead of typing its
password. An enhanced mode, on by default and switched in Options, turns
them all off and gives the original game back. It runs on Windows and in a
web browser.

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

## Play in the browser

**<https://crpcrp.github.io/stone-age-enhanced/>**: the free set, nothing to
install. It needs a keyboard. Scores and settings stay in that browser;
**Export saves** and **Import saves** keep them in a zip. If you own the
original game, **Add the original game (zip)** takes a zip of `SA_V1_00.EXE`
and its eight `.CMF` songs; it is unpacked in your browser and never
uploaded. `?original` after the address plays one session without the
enhancements, `?free` one without the original's files. The page is built
from `main` by GitHub Actions on every push.

## How it was checked

The reconstruction, with the enhancements off, was compared with the
original executable, run in an emulator under the same key presses: every
displayed frame and palette, music call and score-file write, tick by tick,
through the menus, the screens and all 100 original levels. The music
driver's AdLib register writes were compared with the original driver's
separately. Those tests need the original game and are not part of this
repository.

## Build

For the desktop there are no prebuilt downloads: build the game from source.
You need CMake
3.25+ and a C++14 compiler; on Windows, Visual Studio 2022 or its Build Tools
with the C++ workload has both. The first build downloads SDL3.

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
- **Maps (F2):** every map reached so far, 20 to a page. The arrow keys,
  PgUp/PgDn, Home and End choose one; Enter or Space plays it from there
  with a new score and five lives. Maps not reached yet show as grey;
  cleared ones have a yellow corner, and the top line shows the chosen
  map's best time and best points. Tab
  or P opens the password screen of the original (passwords still work
  there), Esc goes back.
- **Statistics (F8):** the totals of every game played (time, games,
  levels cleared, steps, hourglasses, keys, gates, teleports and
  platforms), then each map played of the current set: attempts, clears,
  lives lost, give-ups and best time, ten to a page. The arrow keys and
  PgUp/PgDn turn the pages, Tab or S sorts the maps by number or by
  attempts, Esc goes back.
- **Options (F5):** F1 enhanced mode, F2 setup, F3 full background, F4
  flat stone, F5 sound. With enhanced mode off the game is the original's:
  F2 in the menu is the password screen, F8 does nothing and no progress
  is recorded.
  `stone_play.exe --original` plays one session that way without changing
  the setting.
- **Setup (F2 in Options):** F1 images, F2 maps, F3 music, each FREE or
  ORIG (the original game's). They can be changed only when the original
  game's files are there (see below); F10 goes back to Options, which then
  shows the images chosen. Images are all pictures, colours and texts; maps
  are the levels with their passwords; music is the songs with their titles
  in the jukebox. F4 (VIEW) chooses how the picture fills the window in
  enhanced mode: SHARP, square pixels at a whole multiple; CRT, 4:3 as on a
  DOS monitor, with scanlines; SMOOTH, 4:3 with the edges of the pixel art
  rounded off.

The furthest map reached and the best result on each map cleared (time
from arrival to exit, pauses left out, and points including the bonus),
for each set of levels, the statistics, the enhanced-mode setting,
the asset choices and the display preset are kept in `STONEAGE.ENH` beside the player. Games
started after MAYDAY record no bests and no statistics. The high
scores of the original maps stay in the original's `STONEAGE.HI`; those of
the free maps are in `FREEMAPS.HI`.

## Playing with the original game's content

You need your own copy of the original game: `SA_V1_00.EXE` and its eight
songs (the `.CMF` files beside it). Put them in a folder named
`original_data`, either in this repository or beside `stone_play.exe`. The
player then uses the original's pictures, songs, levels, passwords and texts,
and Options F2 can switch each of images, maps and music back to the free
set. `stone_play.exe --free` plays the free set only, for one run. Do not
commit or distribute these files, or the `original.dat` the player makes from
them.

## Rights

Stone Age and its code, artwork, music, levels and texts belong to their
rights holders. The program in `src/` is derived from the original's code;
the assets in `assets/` are new work. No license is granted. See
[LICENSE](LICENSE) and [LEGAL.md](LEGAL.md).
