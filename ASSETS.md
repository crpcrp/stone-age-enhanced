# Assets: what each file is and what the game does with it

Everything the game shows and plays comes from the files in `assets/`, packed
by the build into `build/stoneage.dat`. This is the map for anyone making new
ones: which file, which size, which colours, which sprite number does what.
Addresses such as `CS:0946` point into the original program (see
`src/game_names.hpp` for the named variables).

## Two asset sets

- **The free set**, in `assets/`: pictures, songs, a hundred levels with
  their own passwords, and texts, all made for this project, with nothing
  taken from the original game. It is what the repository ships and what the
  build packs.
- **The original game's own**, for those who have its files. Put
  `SA_V1_00.EXE` and the eight `.CMF` songs in a folder `original_data/`
  beside `stone_play.exe` or one folder above it (the repository root). The
  player then packs them once into `original.dat` beside itself and uses that:
  the original's pictures, songs, levels, passwords and texts.
  `stone_play --free` plays the free set anyway; delete `original.dat` after
  changing the original files.

With both sets there, Options F2 takes three parts from either one: **images**
(every picture, `colours.txt`, `sine.bin` and the texts), **maps**
(`levels.txt`, with the passwords) and **music** (the `.CMF` songs, and from
`texts.txt` their `[jukebox]` titles and `[song_files]` names). A set's parts
need not match the other set's: levels name a theme, palette and background
by number, which every set has.

## Pictures in general

All pictures are uncompressed 8-bit BMP files. A pixel is a palette slot.
Only the nine `screen_*.bmp` files bring their own colours; everywhere else
the game sets the colours from `colours.txt` and the palette in the file is a
preview.

Colour slots during a level:

| Slots | Set from | Notes |
|---|---|---|
| 0 | | Transparent in sprites and tiles, black elsewhere |
| 1-15 | Tile theme (`colours.txt` `[tiles_1..5]`) | 4 red, 5-9 a green ramp and 15 white are the same in every theme; 10-14 are the theme's ramp from dark to light; 1-3 differ per theme |
| 16-31 | The level's `palette` 0-5 (`[palette_0..5]`) | Greys with a tint, dark to light: 21, 19, 23, 18, 22, 26. Fixed: 17 white, 24 and 25 grey, 27 green, 28 red, 29 dark red, 30 and 31 dark green |
| 32-39 | Derived from the theme (`CS:1938`) | Background shades, dark to light. Background set A has only 33, 36 and 39; the rest are black. The free backgrounds use only the darker ones |
| 48-63 | `[letters]` | Letters of the level pages: 49-53 browns, 54-55 purple, 57-60 greens |
| 254, 255 | | Always black and white. Use 254 for black that must not be transparent |

Fades touch slots 0-63 only, so a screen keeps its colours inside them.

## Tiles: `tiles_1.bmp` to `tiles_5.bmp`

128x80: forty tiles of 16x16, eight a row, one sheet per theme. The position
of a tile is its letter: `A`-`T` are 0-19, `a`-`t` are 20-39 (`CS:1bad`).

| Tile | Picture |
|---|---|
| `A` (0) | Empty space: leave it transparent |
| `B` (1) | Flat floor, used when Options F4 (flat stone) is on |
| `C D E F` | Platform going up, down, right, left |
| `G H I` | Platform going up/down, left/right, all four ways |
| `J K L` | Hourglass, cracked floor, wall |
| `M N`, `O P` | Red and green gate; red and green key |
| `Q` | Teleport; `n o p` (33-35) are its animation |
| `R S` | Start, exit |
| `T` (19), `t` (39) | EXIT sign shown over an exit in the lower half / under one in the upper half, during arrival |
| `a` (20) | The outside: solid black (254) |
| `b`-`j` (21-29) | The large rock, 3x3 tiles |
| `k l m` (30-32) | Cracked floor falling away, three stages |
| `q` (36) | The floor the game normally draws for `B` |
| `r s` | Unused |

## Sprites: `sprites/000.bmp` to `184.bmp`

The count is fixed; sizes may change (up to 255x255). 0 is transparent.

| Numbers | Size | Use |
|---|---|---|
| 0-39 | 16x16 | Replaced by the level's tiles; leave empty |
| 40-47, 48-55 | 16x16 | Walking right, left: eight frames each (`DS:1c0b`) |
| 56-63, 64-71 | 16x16 | Walking down, up. 56 is also the standing player |
| 72-78 | 16x16 | Falling asleep |
| 79-82 | 16x16 | Arrival, small to large, then 56 (`DS:1c3e`, nine ticks a frame) |
| 83-92 | 16x16 | Exit, after 64: spinning and shrinking (`DS:1c32`, seven ticks a frame) |
| 93-98 | 16x16 | Selector opening (`DS:1c52`); 98 is the selector frame |
| 99 | | Empty |
| 100-123 | 7x5 | The eyes of the status bar, following the player: frame n looks 15 degrees further clockwise, 100 straight up (`DS:1c7c`) |
| 124-133 | 7x7 | Digits 0-9 of the status bar, colours 254 and 255 |
| 134-180 | 16x16 | Letters of the level pages, slots 48-63: A-Z, 0-9, then two ornaments, `?`, `!`, an ornament, `.`, `:`, `,` and three blanks (176-180 also end and space lines) |
| 181, 182 | 7x7 | `A`, `B`: background set on the title screen, at (208,127) |
| 183, 184 | 20x12, 15x10 | Easy-mode badge on the title screen, at (218,52) and (239,60) |

Sprites 181-184 use the title screen's colours.

## Status bar, pause, backgrounds

- `hud.bmp`, 320x22, slots 16-31 and 0. The game draws on it (`CS:0946`,
  `0991`, `0df2`): time at x=16 and score at x=96 (y=7, eight pixels a
  digit), the two eyes at (209,4) and (215,4), the green key count at x=257
  and the red one at x=307.
- `pause.bmp`, 320x176: a mask. 2 keeps the board; other values are colours.
- `background_a1..a5.bmp`, `background_b1..b5.bmp`, 320x176, slots 0 and
  32-39. A level names its background 0-4; menu F7 switches between sets.
  Set A contains illustrated river, basalt, fossil, crystal and root-filled
  caverns, using black and slots 33 and 36. Set B contains limestone,
  basalt, sedimentary rock, slate and sandstone relief textures, using
  slots 32-36. The engine tints both sets to the level's tile theme; the
  brighter shades are left out so the tiles stand out against them.

## Screens: `screen_*.bmp`, 320x200, own palette

| File | Shown | The game draws on it |
|---|---|---|
| `screen_title` | Menu | Sprites 181-184 as above. The menu text is part of the picture; the free one names F2 "MAPS", the map selector of enhanced mode, and has a line `F8 STATS` (y=135), the statistics of enhanced mode. With the original's picture, which has no such line, the game writes `F8 STATS` in `font.bmp` centred at y=184, on a strip of the commonest colour there |
| `screen_password` | F2 (Tab in the map selector) | Six 16x16 letters from (135,83): `password/00` is the empty cell, `01..26` are A-Z; their background is slot 254 |
| `screen_options` | F5 | The picture's lines F1-F5 (88-211, 62-147) are covered with their commonest colour and five lines written over them in `font.bmp`, 17 pixels apart from y=64, in the colour the picture's lines use most: F1 enhanced mode, F2 assets, F3 full background, F4 flat stone, F5 sound, each but F2 with `options/10` (ON) at x=217 or `11` (OFF) at x=214. The assets screen (F2) is the same picture with three lines, F1 images, F2 maps, F3 music, each ending in FREE or ORIG, and a fourth, NO ORIGINAL, without the original's files. The F10 line stays as drawn |
| `screen_jukebox` | F4 | Song list from (72,61), eight lines of 8 pixels; the playlist at y=147; help below. Glyphs `jukebox/000..049` and `050..099` alternate by line: A-Z, then `. , " -` space space `! ? ( ' : ;`, 0-9 and two spare (`CS:2e1a`) |
| `screen_scores` | F3, name entry, F8 | Text in `font.bmp` from (32,7), 14 pixels a line, in slot 15, which the game makes red; `birds/0..6` (black, slot 254) fly across. The statistics (F8 in enhanced mode) write up to twelve lines of 31 characters there, without birds |
| `screen_credits` | C, victory | White text (slot 15) scrolls through x=80..239 |
| `screen_eclipse`, `screen_dinosaur`, `screen_bones_park` | Attract sequence | Nothing. The file names are the original's; the free set shows a fossil discovery, a stepping-stone valley and cave paintings |

The map selector (F2 in enhanced mode) has no picture of its own: it draws
miniatures of the levels from `tiles_1..5.bmp` (pixels 2, 7 and 12 of each
tile, in the theme's `[tiles_n]` colours), writes in `font.bmp` and numbers
the maps with the status-bar digits, sprites 124-133.

`font.bmp`, 128x128: 128 characters of 8x16 from code 32, sixteen a row;
codes 128-159 are the accented letters of code page 437. Any non-zero pixel
is set.

`sine.bin`: 8192 little-endian words, `16384 * sin(2 pi n / 8192)`; the path
of the birds.

## Songs: `*.CMF`

Eight files, named in `texts.txt` (`[song_files]`, `DS:15ec`): the free set
calls them `1TUNDRA.CMF` to `8AMBER.CMF`. The format is the music driver's
own ("A.H."), not Creative's CMF.

| Position | Contents |
|---|---|
| 0 | `A.H.` and a zero byte |
| 5 | 99 words: the order of patterns, ended by 99 |
| 203 | Four words: pattern count, speed, rhythm mode (0 or 1), instrument count |
| 211 | Instruments, 69 bytes each: 13 words for the modulator, 13 for the carrier, two waveform words, a 13-byte name |
| then | Patterns: events of six bytes (row 0-63, channel 0-11, note, instrument, volume, effect), each pattern ended by a byte 255 |

- A row lasts `speed + 1` driver ticks of 1/18.2 s.
- Note 72 is middle C; 4 is a key-off; 1 in channel 0 ends a pattern early.
- Instruments count from 1. The 13 words are the AdLib ones: key scaling,
  multiple, feedback, attack, sustain level, sustained, decay, release,
  output level, tremolo, vibrato, envelope scaling, and FM (1) or additive (0).
- Volume 0-127. Effect 32 is none; 30 and 34 detune down and up.
- In rhythm mode channels 6-10 are bass drum, snare, tom, cymbal, hi-hat.

The song titles shown in the jukebox are in `texts.txt`.

## Levels: `levels.txt`

A hundred levels of 20x11 tiles; the header of the file explains the format,
which is also what `stone_assets export` writes. A header line ends with the
level's password, six letters (`password WEFARO`), on every level.

The free levels were made by a level generator (not part of this
repository). Each is one puzzle over the whole board, as in the original: open
space with walls to stop platforms against, the start ledge, the exit (often
in a pocket behind a gate, its key far off or behind a gate of the other
colour), cracked floor, sometimes teleports, and the platforms. Boards were
changed a little at a time and kept when a search of every state showed a
longer way through, more moves that cannot be undone, more gates and more
returns to ledges left before. Difficulty climbs quickly through the first twenty
levels and stays nearly level after fifty. The clock allows the fastest way
and a little time to think per move. Hourglasses lie in four levels only: in
two the clock cannot be beaten without one, in two taking it strands the
player.

Level rules (from `src/game.cpp`):

- The player walks onto anything except `A`, `L` and the lower-case tiles;
  a gate needs a key of its colour, which it uses up.
- Standing on a platform and walking towards empty space (`A` or `a`) in a
  direction the platform goes rides it: it crosses empty space until
  something else is in the way, and leaves empty space behind.
- With the selector (Space) any other platform can be sent off the same way
  from a distance.
- A cracked floor turns into empty space once the player has left it.
- With two teleports, stepping on one comes out at the other.
- Stepping on the exit ends the level.

## Texts and colours: `texts.txt`, `colours.txt`

Both are required: the program holds none of these texts and colours.

- `texts.txt`: the credits, the jukebox texts, the song file names, the score
  table's heading and built-in rows, the name entry text, and the pages
  between levels with their four quotes. Each `[section]` fills a fixed area
  of the program's data; the header of the file gives the encoding and the bytes the game writes into.
- `colours.txt`: the sixteen colours of each tile theme (slots 0-15), of each
  level palette (16-31) and of the page letters (48-63), the tables in the
  colour slot list above.
