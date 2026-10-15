# Enhancements

What enhanced mode adds to the original game, and what is planned. The code
refers to entries here by number (`ENHANCEMENTS.md, E1`).

| | Enhancement | State |
|---|---|---|
| E0 | Enhanced mode switch | Done |
| E1 | Map selector | Done |
| E2 | Remember the last map | Done |
| E3 | Best time and score per map | Done |
| E4 | Statistics | Done |
| E5 | Display presets | Done, without fullscreen |
| E6 | Gamepad and touch | Done; not yet tried on real phones or gamepads |
| E7 | Level packs | Done; not yet tried in the browser |
| E8 | Solver | Planned |
| E9 | Level editor | Planned |

E8 and E9 are listed in the order of work; each builds on those before it.

## Rules for every enhancement

1. **One switch.** Enhanced mode (Options F1) turns every enhancement on or
   off together. With it off, or with `--original`, the game matches the
   original in every frame, palette, music call and score-file write, and
   the comparison tests pass unchanged. The one exception is touch (E6),
   which is on in both modes: it only sends keys, as a keyboard does.
2. **No new state in the original's memory.** Enhancements keep their state
   in C++ members of `Machine` or in local objects, never in the original's
   data segment, so the two programs stay comparable byte for byte.
3. **`STONEAGE.ENH` holds everything saved.** A text file of lines
   `key value...` (`settings.hpp`). Lines a version does not know are kept,
   so older and newer versions can share the file. `STONEAGE.HI` stays the
   original's. Exception: each level pack's score table (E7), which uses the
   original's format in a file of its own.
4. **Progress belongs to a level set.** Every record for a map carries the
   CRC-32 of its level set (`Game::level_set()`), so the original's levels,
   the free set and each pack keep separate records.
5. **Scripted runs never save.** A run with `--ticks` keeps `STONEAGE.ENH`
   in memory and never writes it, so tests and a player's progress cannot
   affect each other.
6. **Cheats record nothing.** Games started with `LVLnnn` and levels played
   while MAYDAY is on (`cheat_mode()`) record no bests and no statistics.

## Saved lines

| Line | Entry |
|---|---|
| `enhanced 1\|0` | E0; on when missing |
| `light 1\|0` | E0 |
| `images\|music original\|free` | Asset set per part (Options F2); `original` when missing |
| `pack free\|original\|<file name>` | E7; without it, `maps free` of earlier versions gives `free`, anything else `original` |
| `display sharp\|crt\|smooth` | E5; `sharp` when missing |
| `reached <crc> <level>` | E1 |
| `last <crc> <level>` | E2 |
| `best <crc> <level> <ticks> <points>` | E3 |
| `stats <crc> <level> <attempts> <clears> <lives lost> <give-ups> <ticks>` | E4 |
| `totals <ticks> <games> <cleared> <steps> <hourglasses> <keys> <gates> <teleports> <platforms>` | E4 |

---

## E0: enhanced mode

Options F1 switches it; `--original` (`?original` on the web) turns it off
for one run without changing the setting.

In enhanced mode LIGHT (menu F6: double time, a quarter of the time bonus)
is saved whenever F6 switches it, and a run starts with it as it was left.
With enhanced mode off it starts off and is not saved, as in the original.

## E1: map selector

Menu F2 shows all maps of the level set (100, or fewer in a pack, E7) as
miniatures, 20 to a page, drawn from the level's tiles. Maps not reached are
grey. The arrow keys, PgUp/PgDn, Home and End move the cursor; Enter or
Space starts a new game (new score, five lives) at any map reached. Tab or P
opens the original's password screen; L the list of level sets (E7); Esc
goes back. With enhanced mode off, F2 is the password screen.

## E2: remember the last map

The selector opens on the map last started (`last`) if it has been reached,
otherwise on the furthest map. Home jumps to map 1, End to the furthest.
`record_level()` writes `last` and `reached` together, and only when one
changes.

## E3: best time and score per map

- *Time:* ticks from the end of the arrival (`appear()`) to stepping onto
  the exit; pauses do not count. Shown in seconds with tenths.
- *Points:* the score after the completed page, bonus included, minus the
  score when the level started. The six-digit score wraps past 999,999 and
  the points wrap with it.
- Fewest ticks and most points are kept separately, so they can come from
  different runs. A map with a best counts as cleared.

`record_best()` (`selector.cpp`) runs after `award_bonus()`, on the
completed page or before the victory credits after level 100. A new best
shows a line on the completed page: `NEW BEST TIME`, `NEW BEST SCORE` or
`NEW BEST TIME, SCORE`. In the selector the top line shows the chosen
map's best time and points, and a cleared map has a triangle in the
top-right corner of its miniature.

`Machine::recording` says whether a level counts, for E3 and E4: enhanced
mode on, not a cheat (rule 6).

## E4: statistics

Menu F8 opens a statistics screen on the score screen's picture.

- Page 1: the totals (time played, games, levels cleared, steps, hourglasses
  taken, keys collected, gates opened, teleports used, platforms moved) and
  how many maps of the current set are cleared.
- Then the maps of the current set with at least one attempt, ten to a
  page: attempts, clears, lives lost, give-ups and best time. Every attempt
  ends in exactly one of clear, lost life (time out or Q) or give-up (Esc).

The arrow keys and PgUp/PgDn turn pages, Home and End jump to the first and
last; Tab or S sorts maps by number or by attempts; Esc goes back.

`Machine::tally` counts during a level and `record_stats()` (`stats.cpp`)
writes it once after the wipe that ends the level. Steps are counted when
the player sets off for the next cell, so platform rides and the selector
are not steps. A key counts when picked up (not the 256th, which is left
lying), a gate when it opens, a teleport when entered, a platform whenever
one is launched, also by the selector.

The free title picture has an `F8 STATS` line. On the original's picture
the seven menu lines move up 8 pixels and `add_stats_line` builds
`F8 - STATS` from the picture's own letters. With enhanced mode off F8 does
nothing.

## E5: display presets

Options F2 (setup), F3 VIEW cycles the preset; it takes effect at once and
is saved when F10 leaves. All of it is in the host (`play.cpp`,
`host_sdl.hpp`); `GameView` reads `Machine::display` every frame.

- `sharp` (default): square pixels at the largest whole multiple that fits.
- `crt`: 4:3, as a DOS monitor showed 320×200, with SDL's
  `SDL_SCALEMODE_PIXELART` (sharp-bilinear) and a scanline overlay drawn
  with `SDL_BLENDMODE_MOD` (a sin² beam, 45% dark between rows); the picture
  is drawn 1.2 times brighter to make up. Below three window rows per game
  row the scanlines are left out.
- `smooth`: 4:3, Scale2x twice on the CPU (to 1280×800) when the picture
  changes, drawn with linear filtering.

With enhanced mode off the window uses SDL's letterboxed 320×200
presentation with nearest scaling. Screenshots (`--screenshot`) are taken
from the game's picture, so presets do not change them. Fullscreen
(Alt+Enter, F11) is not done.

## E6: gamepad and touch

Both send the scan bytes and characters a keyboard sends, so the game and
its determinism are untouched. The host queues their keys one a tick, so a
press and its release both reach the game.

### Gamepad (enhanced mode only)

SDL3's gamepad API (`gamepad.{hpp,cpp}`). The first gamepad connected is
used; gamepads can be plugged in and out while the game runs. Scripted runs
open none. Presses count only in enhanced mode, releases always.

| Gamepad | Key |
|---|---|
| D-pad, left stick (dead zone, stronger axis wins) | Arrow keys |
| A (south) | Enter |
| X (west) | Space |
| B (east) | Esc |
| Y (north) | L (level sets, E7) |
| Start | P (pause) |
| Back, held half a second | Q (retry) |
| LB, RB | PgUp, PgDn |

While the game reads characters (password, name entry): up and down change
the letter (held, they repeat), right accepts it, left deletes, A sends
Enter.

### Line cursor and taps

The menu, Options, setup and the jukebox are driven by function keys.
`LineCursor` (`lines.cpp`) puts each screen's lines (a box in game pixels
and its key) in `Machine::lines`, and restores the parent's when a
sub-screen closes.

- In enhanced mode a pulsing triangle (colour 253) stands left of the
  current line. The arrow keys move it, wrapping round; Enter or Space
  sends the line's key. Function keys still work. The menu remembers its
  line in `Machine::menu_line`.
- A tap (an SDL finger event, any touch screen, both modes) acts on the
  nearest line within six pixels, mapped through the view
  (`GameView::to_game`). In the map selector a tap moves the cursor to a
  map (`Machine::tapped`) and a second tap plays it.

Line tables: the menu's free lines from (107,65), ten pixels apart, and the
original's measured from its picture; Options and setup from
`draw_option_lines`, F10 at y=156; the jukebox's from its texts (songs are
F1-F8, help lines starting with `F9`, `DEL`, `F10` or `ESC` send that key).
The free pictures' positions are listed in [ASSETS.md](ASSETS.md).

### Touch on the web page

Shown only on touch screens (`pointer: coarse`).

- **Buttons** in the strips beside the 16:10 picture: the arrows as a cross
  on the left; Space, Enter and Esc on the right; P, Q and Tab above the
  left group; PgUp, PgDn and a Keys drawer (F1-F10) above the right. Held
  upright, all buttons go below the picture. They call `stone_key(scan,
  down)`.
- **Typing.** While the game reads characters a text field over the picture
  brings up the phone's keyboard and passes text through `stone_type`.
- **Fullscreen.** On Android the first tap requests fullscreen and a
  landscape lock; the game goes on if either is refused. On iOS, adding the
  page to the home screen (`manifest.webmanifest`: fullscreen, landscape)
  does the same. A ✕ leaves fullscreen. Held upright, the page asks to turn
  the phone.
- **Not getting in the way.** Buttons and taps beside them cancel
  `pointerdown`, so the canvas keeps focus; `touch-action: none` stops
  scrolling and zoom; lifting a finger, `pointercancel` or leaving the page
  releases the key.

`tests/taps/` taps the menu to Options and moves the cursor, and taps the
selector to map 13, comparing each screenshot by SHA-256.

---

## E7: level packs

Level sets to choose from: the free maps, the original's (with its files,
[BUILDING.md](BUILDING.md#the-original-game)) and packs of your own, text
files in a folder `levels/` beside the player.

- **Format:** that of `levels.txt` (its header explains it), with 1 to 100
  levels, an optional first line `name <text>` (the file name without
  `.txt` otherwise) and an optional `password` on each level. A level
  without one gets a password made from the set's CRC, unlike any other of
  the set. `levels.{hpp,cpp}` in `stone_core` reads and writes the format,
  for the packer and the player alike.
- **Choosing:** L in the map selector (or tapping `L SETS` in its header,
  or Y on a gamepad) lists the free maps, the original's when its files are
  there, and each pack, sorted by file name, with its name, number of maps
  and the maps reached and cleared. The set played is in the colour of a
  cleared map's mark. A pack that does not read is listed as `DOES NOT
  READ`, with its first error and line under the list when the cursor is on
  it, and cannot be chosen. Enter plays the set from now on, Esc or L goes
  back; the selector opens again on the set chosen. The choice is saved as
  `pack free`, `pack original` or `pack <file name>` and is the set for F1,
  F2 and the password screen. Options F2 (setup) no longer chooses the maps.
- **Loading:** the host (`play.cpp`) reads the set and gives it to
  `Assets`, which hands out its records in place of the resource at file
  offset `157e0` and writes its passwords over `level_passwords()`
  (DS:1f24), as for the maps of an asset set. Records are padded to 100 with
  copies of the last, so `level_record_size()`, `Game::level_set()` and the
  selector's arithmetic stay the same. `Machine::levels` is the real count,
  `Machine::pack` the saved name. A pack that has gone or no longer reads
  gives way to the original's maps, or without them the free ones.
- **Fewer than 100 levels:** the victory credits follow the pack's last
  level; the selector shows only its maps, statistics count only them, and
  the password screen and `LVLnnn` know only them. This holds with enhanced
  mode off too, where the set saved is still the one played; the built-in
  sets have 100 levels, so nothing changes for them.
- **High scores:** each pack has `scores/<crc>.HI` beside the player, in
  `STONEAGE.HI`'s format, starting from the built-in default table.
  Choosing a set loads its table; tables are saved whenever they change, so
  nothing of the old one is lost. In enhanced mode the score screen names
  the pack on its bottom line, the table moved up 7 pixels to make room.
  The built-in sets keep their files: `STONEAGE.HI` for the original's
  maps, `FREEMAPS.HI` for the free ones.
- **In the browser** the folders are `/save/levels/` and `/save/scores/`.
  Export saves puts both in the zip; Import saves takes `levels/*.txt` and
  `scores/*.HI` from it, which is how a pack gets into the browser.

`stone_tests` reads packs of 1, 37 and 100 levels, names, passwords made and
given, broken files with their lines, and a pack written and read back.

## E8: solver (planned)

A search that finds a solution to a map and proves it in the real game. It
gives no hints during play. Uses:

- **Solvability** when a pack is loaded (E7) and when the editor saves (E9):
  *solvable*, *unsolvable*, or *unknown* if the search hits its limit.
- **Par:** the fewest ticks to clear the map, shown beside the best in the
  selector and statistics.
- **A check on the free set:** all 100 levels solvable within their time.

**Model.** `solver.{hpp,cpp}` in `stone_core`, plain C++ on one level record,
not on a `Machine`. State: the player's cell, the selector, changeable tiles
(platforms `C`-`I`, cracked floors `K` and `k`-`m`, hourglasses `J`, keys
`O` `P`, gates `M` `N`), keys held, whether teleports are armed, and the
time left. Rules from `game.cpp`, each tied to the routine it mirrors:

- `A`, `a`-`m` and `L` block the player; a gate blocks until a key of its
  colour is held (`blocks`).
- A step takes 16 ticks; a platform moves 2 pixels a tick, 8 ticks a cell.
- A platform sets off when the player walks towards empty space (`A` or
  `a`) in a direction it can go, and keeps going while there is empty space
  ahead (`ride_platform`).
- The selector flies over everything and can launch a platform from afar
  (`drop_selector`); the countdown stops while it comes out.
- A cracked floor crumbles once left; the exit does not open while a floor
  is crumbling (`exit_open`).
- Teleports lead to the next teleport (`step_on`, `next_teleport`); with
  more than two, the player moves through them in turn until Space is held.
- An hourglass adds 50 seconds. A level starts with `time + time/4`
  seconds.

**Search.** Breadth-first over moves, ignoring time, for solvability; then
Dijkstra on ticks for par and the time check. States differing only in
tiles the player can no longer reach are merged. The search stops at a
state limit (about 2 million, to be tuned) and reports *unknown*.

**Proof.** A solution is a `--keys` file, played through `Game` on a
`Machine` without a window; it must clear the level. A failing solution is
a solver bug, not a verdict on the map.

**Where it runs:** `stone_solve levels.txt [level]` on the command line,
and in the player on a background thread when a set is loaded. Results are
cached as `par <crc> <level> <ticks>|unsolvable|unknown`.

**Done when** every free level is solved and its solution clears it in the
real game, and small boards for each rule give the expected results.

## E9: level editor (planned)

Menu F9 (an `F9 EDITOR` line, written by code on the original's picture as
for E4).

- **Opening:** choose a new pack (named as in name entry, saved as
  `levels/<name>.txt`) or an existing pack's map in the selector. An empty
  slot starts with walls round the edge, a start and an exit. Editing a
  built-in map saves a copy into a pack.
- **Screen:** the board as in a level (`Board::draw_all`) with a cursor;
  the 22-pixel strip below shows the tile in hand and its neighbours, the
  level's time, theme, palette and background, and the last check's result.
- **Keys** (on the E6 line cursor for gamepads):

| Key | Action |
|---|---|
| Arrows | Move the cursor |
| Space, Enter | Place the tile in hand; held while moving, paint |
| Tab, Shift+Tab, PgUp, PgDn | Change the tile; number keys pick common ones |
| Del | Clear the cell to empty space (`A`) |
| F1 | Help |
| F2 | Play the level once, recording nothing, then return |
| F3 | Check the level |
| F4-F7 | Time, theme, palette, background |
| Ctrl+Z, Ctrl+Y | Undo, redo |
| F10 | Check and save |
| Esc | Leave, asking if there are unsaved changes |

The 3×3 rock (`b`-`j`) is placed and removed as one piece; `k`-`m` cannot be
placed. **Checks:** exactly one start (`R`) and at least one exit (`S`);
keys without gates or gates without keys (warning); the solver (E8), where
an unsolvable level can still be saved with a warning and a solvable one
shows its par. **Saving** goes through E7's parser, so a saved pack always
reads back and stays editable by hand. Mouse painting is out of scope: the
game receives only scan bytes.

**Done when** a level can be built from nothing, tested, checked, saved and
played from its pack, and the saved file reads back the same.

---

## Tests

`stone_tests` (`tests/`) runs without the original game. It covers every
`STONEAGE.ENH` line, including keeping unknown lines, the zip code, and
reading and writing level packs (E7). The tap tests (E6) compare
screenshots. Planned additions: the solver on small boards and on the whole
free set (E8), and scripted runs through the selector, statistics and
editor.

With each enhancement, the comparison tests with the original executable
are run with enhanced mode off. They are outside this repository.

## Decisions

1. **E5 and E6 follow the switch,** except touch, which is on in both modes
   so the game can be played on a phone.
2. **E3 measures play ticks,** not the time left on the countdown, which
   hourglasses distort.
3. **E7: F1 plays the active pack** from its first level.
4. **E7: each pack has its own score table** (`scores/<crc>.HI`);
   `STONEAGE.HI` stays the original maps' and `FREEMAPS.HI` the free ones'.
5. **E7: the maps are a level set,** chosen in the selector's list with the
   packs, not a part of Options F2 like the images and the music.
6. **E8: the state limit** of 2 million is a guess, to be tuned on the
   hardest free levels.
