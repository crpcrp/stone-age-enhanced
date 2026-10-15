# Enhancements

What the enhanced game adds to the original, and the plan for what comes
next. The code refers to the entries here by number (`ENHANCEMENTS.md, E1`).

- **E0 to E5** are done (E5 without fullscreen, for now).
- **E6 to E9** are planned. Their order below is the order of work: each
  one builds on those before it.

## Rules for every enhancement

1. **One switch.** Enhanced mode (E0, Options F1) turns every enhancement on
   or off together. There is no separate switch for each feature. With
   enhanced mode off, or with `--original`, the game is the original's in
   every frame, palette, music call and score-file write, and the comparison
   tests with the original executable still pass unchanged.
2. **No new state in the original's memory.** Enhancements keep their state
   in C++ members of `Machine` or in local objects, as the map selector
   does, and never in the data segment the original uses. The two programs
   then stay comparable byte for byte.
3. **`STONEAGE.ENH` holds everything saved.** It is a text file of lines
   `key value...` (`settings.hpp`). Lines that a version does not know are
   kept as they are, so older and newer versions can share the file.
   `STONEAGE.HI` stays the original's. The one exception is each level
   pack's score table (E7), which keeps the original's file format in a
   file of its own.
4. **Progress belongs to a level set.** Everything recorded for a map
   carries the CRC-32 of its level set (`Game::level_set()`), as `reached`
   does already. The original's levels, the free set and every level pack
   (E7) therefore keep their own records.
5. **Scripted runs never save.** A run with `--ticks`/`--keys` does not
   write `STONEAGE.ENH`, so tests cannot change a player's progress and
   saved progress cannot change a test.
6. **Cheats record nothing.** Games started by `LVLnnn`, and levels played
   after MAYDAY (`cheat_mode()`), update no bests and no statistics.

---

## E0: enhanced mode (done)

Options F1 switches it. It is saved as `enhanced 1|0` and is on when the
file has no such line. `stone_play --original` turns it off for one run.

## E1: map selector (done)

F2 in the menu shows all 100 maps as miniatures, 20 to a page, and starts a
game at any map already reached. The furthest map reached is saved as
`reached <crc> <level>`. Tab or P opens the original's password screen.

---

## E2: remember the last map (done)

**What.** The map selector opens on the map last started, not the furthest
one.

- Save `last <crc> <level>` whenever a map of that set is started (the same
  place `record_level()` runs).
- In `MapSelector::run()`, start the cursor at `last` when that map has been
  reached, and otherwise at `furthest - 1` as now.
- End jumps to the furthest map, as now. Home jumps to map 1.

**Files.** `settings.{hpp,cpp}` (`last`, `set_last`), `selector.cpp`.

**Done when** the selector opens on the map you left, also after restarting
the game, and each level set remembers its own.

**As built.** `record_level()` writes `last` and `reached` together, and
writes nothing when neither changes. Rule 5 is in place in `play.cpp`: a
scripted run keeps `STONEAGE.ENH` in memory, reading the saved file once at
its start and never writing it. `stone_tests` (`tests/`) checks the new
lines.

## E3: best time and best score for each map (done)

**What.** The game records your best result on every map you have cleared
and shows it in the map selector.

**What is measured.**
- *Time:* the number of ticks from the end of the arrival (`appear()`) to
  stepping onto the exit, counted in `begin_tick()`. Pausing does not
  count. The selector shows it as seconds with tenths (`41.3`).
- *Points:* the score at the end of the completed page (bonus included)
  minus the score when the level started.
- A map counts as *cleared* once it has a best. This replaces nothing:
  `reached` still records how far you have got.

**Saved as** `best <crc> <level> <ticks> <points>`. The two numbers are kept
separately, so the fastest run and the highest-scoring run can be different
runs.

**In the selector.**
- The header shows the map number, its best time and its best points, plus
  its par once E8 exists: `MAP 12  41.3s  1250  PAR 32.0`. This line
  replaces "SELECT MAP".
- A cleared map gets a small mark in the top-right corner of its
  miniature. Maps reached but not cleared get none. Maps not reached stay
  grey.
- When a level ends with a new best, the completed page says so in one line
  (in the sprite font, under the score).

**Files.** `machine.hpp` (tick counter and score at the start of the
level), `game.cpp` (counting), `screens.cpp` (`run_game` records the result
after the completed page), `settings.{hpp,cpp}`, `selector.cpp`.

**Done when** the bests survive a restart, are only ever improved, and are
not recorded for cheat or `LVLnnn` games.

**As built.** `Machine` holds `timing`, `level_ticks` and `level_score`.
`run_game` sets them as each level starts: `timing` is off for `LVLnnn`
starts and whenever MAYDAY is on. `begin_tick()` counts every tick, and
`run_game` zeroes the count once `appear()` is over. `record_best()`
(`selector.cpp`) runs after `award_bonus()`, on the completed page or
before the victory credits after level 100; `Settings::improve_best` keeps
the fewer ticks and the more points and says which it beat. The page's
line, at y=71 between the lives and the quote, reads `NEW BEST TIME`,
`NEW BEST SCORE` or `NEW BEST TIME, SCORE`. The selector rounds ticks to
tenths of a second at 70 ticks a second, and marks a cleared map with a
yellow triangle. The score's six digits go round past 999,999, and the
points are counted round with them. `stone_tests` checks the `best` lines.

## E4: statistics (done)

**What.** A statistics screen, opened with F8 in the menu.

**Counted for each map** (`stats <crc> <level> ...`):
- attempts, clears, lives lost, give-ups (Esc), and ticks played.

**Counted overall** (`totals ...`):
- time played, games started, levels cleared, steps walked, hourglasses
  taken, keys collected, gates opened, teleports used and platforms ridden.

**Where the counting happens.** Counters live in `Machine` and are written
to `STONEAGE.ENH` once at the end of every level, never every tick:
- `run_game`: the outcome of every level.
- `walk`/`arrive`: steps.
- `step_on`: hourglasses, keys, gates and teleports.
- `launch_platform`: platforms.

**The screen.** It uses the score screen's picture (`screen_scores`) and
writes in `font.bmp`, 14 pixels a line, as the score table does.
- Page 1 holds the totals.
- Page 2 lists the maps, ten to a page, with attempts, clears, lives lost
  and best time. You can sort it by attempts to find your hardest map.
- The arrow keys and PgUp/PgDn move through it, and Esc leaves.

**The menu entry.** The free set's title picture (`screen_title.bmp`) gets
an `F8 STATS` line, and `ASSETS.md` is updated to match. With the
original's title picture, which has no room drawn for it, the line is
written by code at the bottom of the screen, the way `draw_options` writes
its lines.

**Files.** `machine.hpp`, `game.cpp`, `screens.cpp`, `settings.{hpp,cpp}`,
a new `stats.cpp`, `assets/screen_title.bmp`, `ASSETS.md`.

**Done when** the numbers add up after a few games: the attempts on a map
equal its clears plus its lives lost plus its give-ups.

**As built.** `Machine::recording` (E3's `timing`, renamed) says whether a
level counts, for bests and statistics alike, and `Machine::tally` counts
what the player does in it; `run_game` clears the tally as each level
starts. Steps are counted in `walk` when the player sets off for the next
cell, so neither platform rides nor the selector count as steps. A key is
counted when it is picked up (not the 256th, which is left lying), a gate
when it opens, a teleport when the player goes into one. `launch_platform`
counts every platform set off, also by the selector, so the screen says
`PLATFORMS MOVED`. `record_stats()` (`stats.cpp`) runs once after the wipe
that ends each level and writes
`stats <crc> <level> <attempts> <clears> <lives lost> <give-ups> <ticks>`
and `totals <ticks> <games> <cleared> <steps> <hourglasses> <keys> <gates>
<teleports> <platforms>`; ticks are those E3 counts, from the end of the
arrival. A lost life is the time running out or Q. Page 1 also says how
many maps of the current set have a best. The map pages list only maps
with an attempt, with a column for give-ups beside the four planned, so
each row visibly adds up; Tab or S sorts them by attempts, the most first,
and Home and End jump to the first and last page. With enhanced mode off
F8 is ignored and the menu is the original's. `stone_tests` checks the
`stats` and `totals` lines. On the original's title picture the seven
lines move up 8 pixels (the A/B sprite with them) and `F8 - STATS` is
added as the eighth step of their stair, made of the picture's own
letters (`add_stats_line`), instead of a line at the bottom.

## E5: display: scaling, aspect ratio and a CRT look (done, without fullscreen)

All of this is in the host (`play.cpp`, `host_sdl.hpp`). Nothing in the
game changes. With enhanced mode off, the window is the plain resizable
window it is today.

**Settings,** saved in `STONEAGE.ENH`:

| Line | Values | What it does |
|---|---|---|
| `display` | `sharp`, `crt` or `smooth` | One of the presets below. |

Fullscreen (a `fullscreen` line, Alt+Enter and F11) was planned here and is
left out for now.

**The presets:**
- `sharp`: square pixels, scaled by whole numbers only. This is the
  default.
- `crt`: 4:3, as a DOS monitor showed 320×200, so each pixel is 1.2 times
  as tall as it is wide. The picture is scaled up by nearest pixel to a
  whole multiple, then smoothly to the 4:3 size ("sharp-bilinear"), and
  dark scanlines are laid over it: an overlay texture drawn at the
  window's size, which needs no shaders.
- `smooth`: 4:3, filling as much of the window as it can, without
  scanlines, with the staircases of the pixel art rounded off (Scale2x).

**Controls.** Options F2 (now `SETUP`) gets a fourth line, `F4. VIEW`,
which cycles through the presets and writes the preset's name where the
other lines write FREE or ORIG.

**How the host learns of a change.** The game writes the new setting and
sets a `Machine` member. The host reads that member every frame and applies
it.

**Files.** `play.cpp`, `host_sdl.hpp`, `screens.cpp` (options),
`settings.{hpp,cpp}`, `machine.hpp`, `README.md` (keys).

**Done when** each preset looks right in a window at 1080p, 1440p and on
an odd window size, and screenshots (`--screenshot`) are unchanged, since
they are taken from the game's picture rather than from the window.

**As built.** `Machine::display` holds the preset (a `Settings::Display`);
the host sets it from `STONEAGE.ENH` at the start, and `GameView`
(`host_sdl.hpp`) reads it with `Machine::enhanced` every frame. With
enhanced mode off it sets SDL's letterboxed 320×200 presentation and
nearest scaling, as before; in enhanced mode it places the picture itself.
The Options screen has no room for a sixth line above F10, so the preset
is on the second screen, F2, renamed `SETUP`: `F4. VIEW` takes effect at
once and is saved when F10 leaves. The preset is named `crt`, not the
plan's `classic`, so that `F4. VIEW` and the name fit the 15 characters
of a line.
- `sharp` draws at the largest whole multiple that fits; a window smaller
  than 320×200 gets as much as fits.
- `crt` uses SDL 3.4's `SDL_SCALEMODE_PIXELART`, which is sharp-bilinear
  done by the renderer. The scanlines are a column one pixel wide and as
  tall as the picture in the window, worked out for its rows (the average
  of a sin² beam over each row, 45% dark between two of the game's rows),
  drawn with `SDL_BLENDMODE_MOD`; the picture under it is drawn 1.2 times
  brighter (`SDL_SetRenderColorScale`) to make up. With fewer than three
  window rows to a game row there is no room for lines, and they are left
  out.
- `smooth` runs Scale2x twice on the CPU (320×200 to 1280×800), only when
  the game's picture changes, and draws that with linear filtering.

## E6: gamepad

With enhanced mode off, gamepads are ignored, as rule 1 says.

Through SDL3's gamepad API (add `SDL_INIT_GAMEPAD`). The host turns
buttons into the same scan bytes the keyboard sends, so the game and its
determinism are untouched. The first gamepad connected is used, and
gamepads can be plugged in and out while the game runs.

**Buttons:**

| Gamepad | Key | Use |
|---|---|---|
| D-pad, left stick | Arrow keys | The stick needs a dead zone and only gives four directions: the stronger axis wins. |
| A (south) | Enter | |
| X (west) | Space | |
| B (east) | Esc | |
| Start | P | Pause. |
| Back/Select | Q | Retry. It must be held for half a second, so it is not pressed by accident. |
| LB, RB | PgUp, PgDn | Pages of the selector and of the statistics. |

**Screens that use function keys.** The menu, Options, the jukebox and the
editor are driven by function keys, which a gamepad does not have. In
enhanced mode these screens get a cursor:
- The arrow keys move a pulsing marker from line to line, and Enter or
  Space acts as the function key of that line.
- The keyboard can use the cursor too, and the function keys keep working.
- The positions of the lines are a small table for each screen. For the
  free pictures they are documented in `ASSETS.md`. For the original's
  pictures they are measured once and compiled in.

**Typing a name for the score table.** This reads characters, not keys. On
a gamepad:
- Up and down change the letter under the cursor.
- Right accepts it and moves on, and left deletes.
- A finishes, sending Enter.

The host does this by typing the characters into the game through
`Machine::type`, as the keyboard does.

**Files.** `play.cpp`, a new `gamepad.{hpp,cpp}` beside it, and
`screens.cpp` (the line cursor).

**Done when** a whole game can be played on an Xbox-style and a
PlayStation-style gamepad, from the menu through the score table, without
touching the keyboard.

## E7: level packs

**What.** You can play your own sets of levels: text files in a folder
`levels/` beside the player.

**The file format** is the one `levels.txt` uses, with three differences:
- A pack holds 1 to 100 levels.
- `password` is optional. A missing one is made from the pack's CRC, so
  the completed page still shows one.
- A first line `name <text>` gives the pack its title. Without it, the
  title is the file name.

**Choosing a pack.**
- L in the map selector opens a list of the packs: the built-in set first,
  then each file. Each row shows its name, its number of levels, and the
  maps reached and cleared.
- A file that does not parse is listed with the first error and its line
  number, and cannot be chosen.
- The chosen pack is saved as `pack <file name>`. It is the active set for
  F1 (new game), F2 (selector) and the password screen until another one is
  chosen.

**How the game loads it.**
- The level records are the resource at file offset `157e0`.
- `Machine` gets a member holding a pack's records. While it is set,
  `supply_resource` hands those out instead, and the pack's passwords are
  written over `level_passwords()` (DS:1f24) for as long as it is active.
- The records are padded to 100 with empty slots, so `level_record_size()`
  and the selector's arithmetic stay the same.
- The parser moves from `assets.cpp` into a small `levels.{hpp,cpp}` in
  `stone_core`, so that the packer, the player, the editor (E9) and the
  solver (E8) all use one parser.

**What changes with fewer than 100 levels.**
- `run_game` ends with the victory credits after the pack's last level,
  not after level 100.
- The selector shows only the pack's maps, with as many pages as it needs.

**High scores.** Each pack has its own score table.
- The built-in set keeps `STONEAGE.HI`, as now.
- A pack's table is saved as `scores/<crc>.HI` beside the player, where
  `<crc>` is the pack's CRC-32 in eight hex digits. It is in the same
  format as `STONEAGE.HI`: the table with every byte inverted.
- `load_scores` and `save_scores` choose the file from the active set, so
  name entry, the score screen (menu F3) and the birds all work unchanged.
  The score screen's title names the pack.
- A pack that has no table yet starts from the default table built into
  the game.
- The table in memory is the original's (`score_table()`). Switching packs
  saves the old pack's table and loads the new one's before anything else
  reads it.

**Files.** New `levels.{hpp,cpp}`, `assets.cpp` (uses it), `machine.hpp`,
`machine.cpp`, `selector.cpp` (list and paging), `screens.cpp`
(`run_game`, password screen).

**Done when** a pack of 1, 37 and 100 levels plays from start to victory, a
broken file is reported and not loaded, and switching packs keeps each
pack's progress and score table separate. With the built-in set active,
`STONEAGE.HI` is read and written exactly as before.

## E8: solver: is the map solvable, and what is its par

**What.** A program that searches for a solution to a map and proves it
works. It gives no hints during play. It is used for three things:
- **Solvability**, checked when the editor saves (E9) and when a pack is
  loaded (E7). A map is marked *solvable*, *unsolvable*, or *unknown* when
  the search stops at its limit.
- **Par:** the fewest ticks the solver needs to clear the map. It is shown
  next to the player's best in the selector (E3) and in the statistics
  (E4).
- **A check on the free set:** all 100 of its levels must be solvable
  within their time.

**The model.** A new module `solver.{hpp,cpp}` in `stone_core`, in plain
C++, not on a `Machine`, so that it is fast. It works on one level record.

The state of a search is:
- where the player is, and whether the selector is out and where;
- the tiles that can change: platforms (`C`-`I`), cracked floors (`K` and
  `k`-`m`), hourglasses (`J`), keys (`O`, `P`) and gates (`M`, `N`);
- how many red and green keys the player holds;
- whether the teleports are armed;
- the time left, in ticks.

The rules it follows come from `game.cpp`, each with the address of the
original routine it mirrors:
- `A`, `a`-`m` and `L` block the player. Gates block until the player
  holds a key of their colour (`blocks`).
- A step takes 16 ticks. A platform moves 2 pixels a tick, so a cell takes
  8 ticks.
- A platform sets off when the player walks into stone (`A`) in a direction
  it can go, and keeps going while there is stone ahead
  (`ride_platform`). The key of the opposite direction turns it round.
- The selector flies over everything and can start a platform from afar
  (`drop_selector`). The countdown stops while it comes out.
- A cracked floor crumbles once the player has left it, and an exit does
  not let the player out while a floor is still crumbling (`exit_open`).
- Teleports take the player to the other teleport (`step_on`,
  `next_teleport`). With more than two, the player moves through them in
  turn until Space is held.
- An hourglass adds 50 seconds.
- The time a level starts with is `time + time/4` seconds.

**The search.**
1. A breadth-first search over the moves, ignoring time, finds out whether
   the map can be solved at all.
2. A Dijkstra search on ticks then finds the par and checks that it fits in
   the time.
3. States that differ only by tiles the player can no longer reach count as
   the same state.
4. The search stops at a state limit, such as 2 million states, and then
   reports *unknown*.

**Proving a solution.** A solution is a list of key presses by tick, the
format `--keys` reads. That list is played through the real game (`Game` on
a `Machine`, with no window) and must clear the level. This catches any
place where the model and the game disagree. A solution that fails is a
bug in the solver, not a verdict on the map.

**Where it runs.**
- A command-line tool, `stone_solve levels.txt [level]`, for level makers
  and for the tests.
- In the player, on a background thread (`Threads` is linked already),
  started when a set or pack is loaded.
- Results are cached as `par <crc> <level> <ticks>|unsolvable|unknown`, so
  each map is solved once.

**Files.** New `solver.{hpp,cpp}` and `solve_main.cpp`, a `stone_solve`
target in `CMakeLists.txt`, and `selector.cpp` and `stats.cpp` for showing
the par.

**Done when** every free-set level is solved and its solution clears it in
the real game, and small handmade boards for each rule give the expected
results.

## E9: level editor

**What.** You can make and change levels in the game, opened with F9 in
the menu. The title picture gets an `F9 EDITOR` line, written by code with
the original's picture, as for E4.

**Opening a level.** The editor first asks for a pack:
- *New pack*, which asks for a name typed as in name entry and makes
  `levels/<name>.txt`;
- or an existing pack, whose maps it shows in the selector in a picking
  mode.

An empty slot gives a new level: walls round the edge, a start and an exit.
The built-in sets are never changed: editing one of their maps saves a copy
into a pack.

**The screen.** The board is drawn as in a level (`Board::draw_all`), with
its theme, palette and background, and a cursor over the current cell. The
22-line strip below the board shows:
- the tile in hand, with the tiles before and after it;
- the level's time, theme, palette and background;
- the result of the last check.

**Keys** (all on the line cursor of E6 for gamepads):

| Key | What it does |
|---|---|
| Arrow keys | Move the cursor. |
| Space or Enter | Put the tile in hand. Held while moving, it paints. |
| Tab and Shift+Tab, or PgUp and PgDn | Change the tile in hand. Number keys pick the common tiles. |
| Del | Clears the cell to stone (`A`). |
| F1 | Help. |
| F2 | Play the level now: a one-level game that records no progress, best or statistics (`counts = false`). The level ends back in the editor. |
| F3 | Check the level (below). |
| F4 to F7 | Change the time, theme, palette and background. |
| Ctrl+Z, Ctrl+Y | Undo and redo the editor's changes. |
| F10 | Save. |
| Esc | Leave, asking first if there are unsaved changes. |

The 3×3 rock (`b`-`j`) is placed and removed as one piece. The tiles the
game writes itself (`k`-`m`) cannot be placed.

**Checking a level.** F3 runs these checks, and F10 runs them before it
saves:
- exactly one start (`R`) and at least one exit (`S`);
- no keys without gates, and no gates without keys, as a warning only;
- the solver (E8). An *unsolvable* level can still be saved, with a
  warning. A *solvable* one shows its par.

**Saving.** The editor writes the pack in the `levels.txt` format, through
the parser of E7. A pack file it wrote can therefore always be read back,
and it can still be edited by hand.

**Later, not part of this plan:** painting with the mouse. The game only
receives scan bytes, so the mouse would need a new way into the game from
the host.

**Files.** New `editor.cpp` and `game.hpp` (its entry point), `screens.cpp`
(menu F9), `levels.{hpp,cpp}` (writing), `assets/screen_title.bmp` and
`ASSETS.md`.

**Done when** a level can be built from nothing, tested, checked, saved,
and then played from the pack in the selector, and the saved file reads
back the same.

---

## Tests

`stone_tests` (CTest, `tests/`) runs without the original game. It was added
with E2 and grows with each enhancement:
- `Settings`: reading and writing every new line, and keeping lines it
  does not know.
- E7: pack parsing, both good files and every error, and writing a pack
  and reading it back.
- E8: the solver on small boards for each rule, and on all of the free
  set, with each solution proven in the real game.
- Scripted runs of the free set (`--keys`, `--ticks`, `--screenshot`) that
  go through the selector, the statistics and the editor, each compared
  with a screenshot.

With each enhancement, the comparison tests with the original executable
are also run with enhanced mode off. They are outside this repository, so
whoever holds the original runs them.

## Decisions

Decided:

1. **E5 and E6 follow the switch.** With enhanced mode off, the display
   presets and the gamepad are all off, as rule 1 says.
2. **E7, F1 plays the active pack.** F1 (new game) starts the pack chosen
   in the selector, from its first level.
3. **E7, each pack has its own high scores,** in its own score table
   (`scores/<crc>.HI`). `STONEAGE.HI` stays the built-in set's.

Still open, with the plan's defaults. Each can be changed before its
enhancement is built:

4. **E3, measuring time.** The default is play ticks. Alternative: time
   left on the countdown, which hourglasses distort.
5. **E8, the state limit.** 2 million states is a guess. It is tuned on the
   hardest free-set levels.
