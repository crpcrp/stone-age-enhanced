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
| E8 | Solver | Done; in the browser only the free set has pars |
| E9 | Level editor | Done; its F9 line on the original's picture not yet seen |

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
   affect each other. The packs the editor (E9) saves stay in memory too.
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
| `par <crc> <level> <ticks>\|unsolvable\|unknown` | E8 |
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

## E8: solver

A search that finds the fastest way through a map and proves it in the real
game. It gives no hints during play. Uses:

- **Par:** the fewest ticks to clear a map, as E3 counts them, shown beside
  the best in the selector and in the statistics.
- **Solvability** of a pack's maps (E7), and of a map the editor checks
  (E9): *solvable*, *unsolvable* (also when the exit can be reached only
  after the time has run out), or *unknown* if the search hits its limit.
- **A check on the free set:** all 100 levels solvable within their time.

**Model.** `solver.{hpp,cpp}` in `stone_core`, plain C++ on one level record,
not on a `Machine`. A state is where the game obeys keys: the player's cell,
or the selector's and whether a platform waits for a direction; the tiles
that can change (platforms, cracked and crumbling floors, hourglasses, keys
and gates); keys held; whether the teleports are armed; the floors still
crumbling and how long since they started; and two timing details: the
mover has just landed, or Space is still down. Rules from `game.cpp`, each
tied to the routine it mirrors:

- `A` and `a` are one tile to everything that moves (stone); `A`, `L`,
  `b`-`m` and a gate without its key block the player (`blocks`).
- A step takes 16 ticks; a platform or the selector 8 a cell.
- Walking toward stone on a platform that goes that way sets it off; it goes
  on while there is stone ahead (`walk`, `ride_platform`). On the tick it
  lands the game still sees stone under the player, so it cannot set off
  again before the next tick.
- Space brings the selector out (no tick of the countdown passes) and calls
  it back on anything but a platform that can go. On a platform with stone
  on exactly one of its ways it sets it off; a `G`, `H` or `I` with stone on
  several waits for a direction key (`drop_selector`). Space must go up
  before it counts again.
- A cracked floor starts to crumble when the player arrives on the next cell:
  it blocks at once, is stone 18 ticks later, and keeps the exit shut for 24
  (`arrive`, `exit_open`, `crumble_tiles`). A platform landing on one still
  crumbling would be drawn over, so the model does not let it.
- Teleports: with two, 86 ticks from arriving on one to obeying keys at the
  other; with more, 43 more for each further one offered before Space is
  held (`teleport`, `after_teleport_animation`). Leaving a teleport, or
  being stopped, arms them again (`probe`).
- An hourglass adds 50 seconds. A level starts with `time + time/4` seconds
  and is lost on the tick they run out.

**Search.** Dijkstra on ticks, with states past the countdown dropped; if
that finds nothing, again without the time, to tell *too slow* from
*unsolvable*. Waiting a tick is a move only while a floor crumbles, the one
thing that changes with time. States are kept as bytes (four bits a changing
tile) in one hash table. The search stops at 2 million states and reports
*unknown*; the free set needs at most 767 thousand, about 110 MB.

**Proof.** `prove()` plays a solution through `Game` on a `Machine` without a
window, from the start of the program: past the title, menu F2 with enhanced
mode off (the password screen), the level's password, its first page, then
the solution's keys. Each key is pressed when the game is about to obey keys
with the player (or selector) on the cell the model expects, and not before
the model's tick; so a model that is wrong shows as a run that goes astray. A
failing solution is a solver bug, not a verdict on the map. The run is kept as
a `--keys` file, which `stone_play --fresh --free --original` plays again;
scripted keys type while the game reads characters, and `tick text WORD`
types a word.

**Where it runs:**

- `stone_solve [--keys DIR] [--states N] [--no-proof] [--package FILE]
  levels.txt [level...]`: solves and proves each level, prints its verdict,
  par and the ticks the game took, and with `--keys` writes
  `DIR/levelNNN.keys`. It exits with 2 when a level is not solvable in time
  or a proof fails.
- **The free set's pars come with it:** `assets/pars.txt`, written by
  `stone_solve --pars`, holds a proved `par` line for each free level and
  is packed into `stoneage.dat`. Once enhanced mode is on, the player copies
  the lines it does not have yet into `STONEAGE.ENH`, in the browser too.
- The desktop player, on a thread of its own, solves the maps of the level
  set played that still have no `par` line (packs, the original's maps),
  and adds the lines to `STONEAGE.ENH` between ticks. Not in the browser,
  which has no threads: there a pack's maps have a par only if an imported
  `STONEAGE.ENH` brings one.
- Neither happens in scripted runs, so they stay the same every time. The
  lines carry the set's CRC, so a changed map is solved again; a newer
  solver does not replace old lines.
- **Selector:** the top line reads `MAP 12  41.3s  1250  PAR 38.0s`, closer
  together or without the points when that is too long; `UNSOLVABLE` when
  there is no way through. Nothing for a map not reached, or not solved yet.
- **Statistics:** a `PAR` column after `BEST` (`NONE` when unsolvable). To
  make room, `CLEAR` is now `WON`.

**Results:** every free level is solvable within its time, each solution
clears it in the real game, and the game takes exactly the ticks the model
says. `stone_solve` takes about a minute and a half for all 100 in a release
build, 4 seconds for the slowest.

## E9: level editor

Menu F9. The free title picture has an `F9 EDITOR` line; on the original's
picture `add_editor_line` closes the lines up to 8 pixels apart and adds
`F9 - EDITOR`, from letters of the picture where it has them (F, the dash,
T, the 9 from its 6 and the E from its F) and D, I, O and R drawn in their
colour. With enhanced mode off F9 does nothing.

- **Opening:** F9 opens the map selector (E1) for the editor: every map of
  the level set played is shown, and after the last map of a pack with
  fewer than 100 an empty slot, `NEW`. Enter edits the map under the
  cursor, L chooses another level set (E7), N starts a new pack (`N NEW` in
  the header), Esc goes back to the menu. An empty slot, and a new pack's
  first map, start with walls round the edge, floor inside, a start and an
  exit.
- **Screen:** the board as in a level (`Board::draw_all`) with a pulsing
  cursor. The 22-pixel strip below it shows the tiles before, in and after
  the hand, the record's time, theme, palette and background
  (`T060 TH0 PA0 BG0`), `*` while there are unsaved changes, and the last
  check: `PAR 5.5s` in green, `UNSOLVABLE`, `TOO SLOW` or an error in red,
  `UNKNOWN`, or `NOT CHECKED` once the level has changed since. A message
  (what a check warns of, how a play ended, where the level was saved)
  takes the place of the text until the next key.
- **Keys:**

| Key | Action |
|---|---|
| Arrows | Move the cursor; held, they repeat |
| Space, Enter | Place the tile in hand; held while moving, paint |
| Tab, Shift+Tab, PgUp, PgDn | Change the tile |
| 1-9, 0 | Floor, wall, empty space, start, exit, cracked floor, platform of all four ways, teleport, hourglass, outside |
| Del | Clear the cell to empty space (`A`) |
| F1 | Help |
| F2 | Play the level once, recording nothing, then return |
| F3 | Check the level |
| F4-F7 | Time (by 10), theme, palette, background; with Shift backwards |
| Ctrl+Z, Ctrl+Y | Undo, redo; a stroke of painting is one change |
| L | A menu of F1-F10, undo, redo and leaving, on the E6 line cursor |
| F10 | Check and save |
| Esc | Leave; with unsaved changes it asks for a second Esc |

- **Gamepad and touch (E6):** the D-pad moves, A and X place (and paint
  while held), LB and RB change the tile, B leaves, and Y (L) opens the menu
  for everything else. A tap on a cell moves the cursor there and a second
  tap places; taps on the tiles beside the hand change it, a tap on the hand
  opens the menu.
- **Tiles:** A-S, the outside `a` and the 3×3 rock (`b`-`j`), placed as one
  piece from the cursor right and down, inside the board only. Anything
  placed on a cell of a rock, or a rock over part of one, takes the whole
  rock away. `k`-`m` and the signs cannot be placed. A start moves: the old
  one becomes floor. `level_edit.{hpp,cpp}` in `stone_core` does this on a
  level's record, without a `Machine`.
- **Playing (F2):** as in a game, from the arrival to the end, with LIGHT as
  set. `Machine::test_level` holds the record, which `load_level` takes in
  place of the level set's, and `Machine::recording` is off, so nothing
  counts (E3, E4). The strip then says `CLEARED IN 12.3s`, `GAVE UP` or that
  the life was lost.
- **Checks (F3):** errors: not exactly one start (`R`), no exit (`S`).
  Warnings: a key without its gate, a gate without its key, a piece of a
  rock without the rest. Without errors the solver (E8) runs, with its 2
  million states, on the game's thread: the game stands still meanwhile,
  seconds for a hard level, in the browser too.
- **Saving (F10):** checks first; a level with errors is not saved, an
  unsolvable one is. A pack's map replaces its record; the empty slot adds a
  level, with a password made as for a pack level without one. A map of a
  built-in set, and the first map of a new pack, ask for a pack name (up to
  16 letters, digits, blanks, `-` or `_`, typed as in name entry, with a
  gamepad's letters too) and become level 1 of `levels/<name>.txt`, other
  characters as `_`; a name whose file is there already is refused. The
  file is written with `write_levels` and read back with `read_levels`
  before it is saved, so a saved pack always reads back as saved and stays
  editable by hand. The pack is then the level set played (`pack` in
  `STONEAGE.ENH`, E7), and a check of the level as saved writes its `par`
  line under the pack's new CRC. Every save changes the pack's CRC, so its
  progress, bests and statistics start again (rule 4).
- **Where packs go:** `Machine::save_pack` is the host's. The desktop
  writes into `levels/` beside the player, the browser into `/save/levels/`
  (in Export saves, E7). A scripted run or `--fresh` keeps saved packs in
  memory and lists them with the others (rule 5).

Mouse painting is out of scope: the game receives only scan bytes.

---

## Tests

`stone_tests` (`tests/`) runs without the original game. It covers every
`STONEAGE.ENH` line, including keeping unknown lines, the zip code, and
reading and writing level packs (E7). The solver (E8) is tried on a small
board for each rule, with its verdict and where the moves are plain its par;
given the asset package, as CTest gives it, each solution is proved in the
game too, and the package is checked to have a par for each free level
(`assets/pars.txt`); no test runs the solver on the free set. The tap tests (E6) compare
screenshots. `level_edit_tests` tries the editor's model (E9): the empty
slot, which the solver clears, placing tiles and the rock, the checks, and a
pack built as the editor builds it, read back the same. `editor_new_pack`
goes from the menu through the editor to a new pack, saved and played from
F2, and compares a screenshot of its level. Planned additions: scripted runs
through the selector and statistics.

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
6. **E8: the state limit** stays at 2 million: the hardest free level
   needs 767 thousand.
7. **E8: one model, checked by the game.** The solver models the rules
   rather than searching on a `Machine`, which runs the whole game a tick at
   a time; every solution it gives can be proved, and the free set is.
8. **E8: too slow counts as unsolvable** in `STONEAGE.ENH`: the player
   cannot clear such a map.
9. **E8: the free set's pars are shipped,** not searched for in the
   browser: a search on the page's one thread would stop the game for
   seconds a map. A Web Worker running the solver could do it for packs.
