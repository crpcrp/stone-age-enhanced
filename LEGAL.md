# Legal notes

This is an unofficial reconstruction of Stone Age. It is not affiliated with,
endorsed by or authorized by the game's creators or rights holders. See
[LICENSE](LICENSE).

## What is and is not included

- **Not included:** anything of the original game's content: its
  executable, pictures, songs, levels, passwords, texts and colours. To use
  them, you need your own copy of the game; the player reads them from it on
  your computer (`original_data/`, packed into `original.dat` beside the
  player). Do not distribute these files or `original.dat`.
- **Derived from the original:** the program. `src/music_lift.inc` is a
  translation of the original's music driver; the readable game
  (`src/game.cpp`, `src/screens.cpp`, `src/video.cpp`) was written from a
  translation of the original's code, routine by routine; `src/game_data.inc`
  and `src/music_tables.inc` hold the start values, pointers and tables the
  program needs whatever its assets (no levels, texts or pictures). This is
  not a clean-room implementation, and the rights holders' rights in these
  files are not affected by this repository.
- **New work:** everything in `assets/`: pictures, songs, the hundred levels
  with their passwords, the texts and the colours. They were
  made for this project and contain nothing taken from the original.

## The web version

The page on GitHub Pages is built from this repository alone and holds the
free set only. The original game's files that a player adds there are
unpacked in their browser and kept in its storage; they are never sent to
the site or anywhere else.

## Rights holders

If you hold rights in Stone Age and want this project changed or taken
down, open an issue on the repository or contact its owner through GitHub,
and it will be dealt with promptly.

## History

The repository's history starts with the free asset set; earlier work that
held the original's exported content is not published.

## Third-party components

| Component | License | Notice |
|---|---|---|
| SDL3 | zlib | [SDL3.txt](third_party/licenses/SDL3.txt) |
| ymfm (YM3812 emulator) | BSD 3-Clause | [ymfm.txt](third_party/licenses/ymfm.txt) |

Both licences allow the player to be passed on with their notices, which the
build copies beside it and the web page publishes with it.
