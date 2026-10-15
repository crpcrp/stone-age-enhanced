# Plan: a browser version on GitHub Pages

This plan is for a version of the player that runs in a web browser. GitHub
Actions builds it on every push to `main` and publishes it on GitHub Pages.
The site plays with the free assets in `assets/`. A player who owns the
original game can add a zip of its files on the page; the zip stays in their
browser and never reaches the server. Each milestone below ends with the
check that shows it is done; milestones 1 to 5 and 7 are done.

## Decisions

- **Toolchain:** Emscripten. It compiles C++ to WebAssembly, and SDL3 and
  ymfm build with it unchanged. The emsdk version is pinned in the
  workflow, as SDL3 is pinned in `CMakeLists.txt`.
- **No threads in the web build.** `Machine` runs readable code on a thread
  of its own (`machine.cpp`, `Machine::Turns`). On the web the same
  hand-off uses Emscripten fibers instead, so the build needs no
  `SharedArrayBuffer` and no cross-origin isolation headers, which GitHub
  Pages cannot send. The desktop build keeps its thread.
- **One host, two loops.** The body of the loop in `play.cpp` becomes one
  step function. The desktop calls it in its `while` loop as now; the web
  build calls it once per browser frame. Everything the game does stays in
  `src/`'s portable core, as it is today.
- **Saves in the browser.** `STONEAGE.ENH`, `STONEAGE.HI`, `FREEMAPS.HI`
  and `original.dat` live in IndexedDB (Emscripten's IDBFS), mounted where
  the desktop player keeps them beside itself.
- **Original files only from the player.** The site never holds the
  original's files or an `original.dat`; the workflow builds from the
  repository alone, which holds none of them (`LEGAL.md`).

## Milestones

### 1. A web build that runs the free assets locally

- `CMakePresets.json`: a `wasm` preset (configured with `emcmake`), output in
  `build/web/`.
- `CMakeLists.txt`, under `if(EMSCRIPTEN)`:
  - SDL3 built from source as a static library (`SDL_SHARED OFF`,
    `SDL_STATIC ON`); the current forced `SDL_SHARED ON` stays for the
    other builds.
  - No `Threads::Threads` for `stone_core`.
  - `stone_assets` cannot run inside a wasm build, so `stoneage.dat` is
    packed by a native `stone_assets` built first (the `core` preset) and
    passed in as `STONE_PACKAGE`. Emscripten preloads it into the virtual
    file system (`--preload-file`).
  - Link options: `-sASYNCIFY` (for the fibers), `-fexceptions` (the core
    throws and catches `std::runtime_error`), `-sALLOW_MEMORY_GROWTH`,
    `-lidbfs.js`, and an HTML shell (`web/shell.html`).
- `machine.cpp`: `Machine::Turns` gets a second implementation behind
  `#ifdef __EMSCRIPTEN__`, built on `emscripten_fiber_init` /
  `emscripten_fiber_swap`. `pass(true)` swaps to the routine's fiber,
  `pass(false)` swaps back; `begin_thread`, `take_turns` and
  `give_turn_back` keep their meaning. A failure is caught inside the
  fiber and rethrown by `take_turns`, as now. A machine is abandoned in the
  middle of a routine only when the page closes, so the web version needs
  no `Abandoned` unwinding: its fiber is never resumed.
- `play.cpp`: the state the loop uses (window, texture, clock, scan,
  stream, machine and the rest) moves into one structure, and the loop's
  body into its `step()`. On the web, `emscripten_set_main_loop_arg` calls
  `step()` every frame; `SDL_Delay(1)` is desktop only. When the game ends
  (INT 21h/4Ch), the web page shows "Play again" in place of closing.
- The command-line options stay. In the web build they come from the page's
  URL (`?original`, `?free`), so a link can start a run without enhancements.

**Done when:** `build/web/` served by `python3 -m http.server` shows the
title screen in Chromium, and a level plays with music.

**Done.** With emsdk 4.0.10 active:

```sh
cmake --preset core && cmake --build --preset core   # packs build/stoneage.dat
emcmake cmake --preset wasm && cmake --build --preset wasm
cd build/web && python3 -m http.server
```

The fibers were checked with exceptions thrown and caught across a swap.
The desktop player's scripted runs give the same frames as before.

### 2. Saves that last

- At start-up, before the game reads anything, `/save` is mounted as IDBFS
  and read in with `FS.syncfs(true)`; the player starts once it is loaded.
- `score_directory` is `/save/` on the web (`SDL_GetBasePath()` returns
  `/`, which is not persistent).
- `machine.save_file` calls `FS.syncfs(false)` after each write, so a
  score or a setting is kept even when the tab is closed straight after.
- The page gets **Export saves** and **Import saves** buttons (a zip of the
  `/save` files), because a browser may delete a site's storage.

**Done when:** a new high score and the map reached are still there after
reloading the page, and after export, clearing site data and import.

**Done.** `web/saves.js` (linked with `--pre-js`) mounts `/save` in
`preRun` behind a run dependency, so `main` waits for the saves, and
stores `/save` one `syncfs` at a time. The zip is made and read in C++
(`src/zip.cpp`, with the inflate of `package.cpp` now in `package.hpp`;
milestone 3 uses it too), through `stone_export_saves` and
`stone_import_saves`. Export carries `STONEAGE.ENH`, `STONEAGE.HI`,
`FREEMAPS.HI` and `original.dat`; import takes those names in any case,
in a folder or not, replaces only those it holds (an `original.dat` must
open as a package) and reloads the page. `stone_tests` checks the zip
reader and writer.

Checked in Chromium with Playwright: a setting changed in Options is in
IndexedDB within 300 ms and is there after an immediate reload; the
exported zip brings it back in a new profile; an imported `STONEAGE.ENH`
with map 7 reached opens Maps with 1 to 7; a file that is not a zip and a
zip without saves are refused with a message. The scores go through the
same `save_file` as the settings; a new high score was not played for, as
no key script that clears a free map is in the repository yet.

### 3. The original game from a zip

- The page has **Add the original game (zip)**. The zip must hold
  `SA_V1_00.EXE` (452,788 bytes) and the eight `.CMF` songs, at its top or
  in one folder, with names in any case.
- Unpacking is done in C++ with `stone::read_zip` (`src/zip.cpp`, from
  milestone 2), which reads stored and deflated entries and reports
  anything else as an unsupported zip.
- The entries are written to `/tmp/original_data/` in memory, each under
  the exact name the executable gives it (`song_files` in `assets.cpp`;
  the virtual file system tells case apart, a DOS zip may not), and
  `stone::pack_original` packs them into `/save/original.dat`, as
  `find_original_package` does on the desktop. The executable's SHA-256
  is checked first, so a wrong file gets a clear message, not a broken game.
  The loose files are then deleted; only `original.dat` is kept, in the
  player's own browser.
- After packing, the page restarts the game, which then finds
  `original.dat` and offers ORIG in Options F2 (Assets), as on the desktop.
- **Remove the original game** deletes `original.dat`. The asset choices in
  `STONEAGE.ENH` fall back to the free set when it is missing, as they
  already do.

**Done when:** with a zip of the original, Options F2 switches images, maps
and music to ORIG, and the choice and the scores in `STONEAGE.HI` survive a
reload. A zip with a wrong executable is refused with a message.

**Done.** `web/original.js` (a second `--pre-js`) gives the page
`Module.addOriginal`, `Module.removeOriginal` and `Module.hasOriginal`.
`stone_add_original` only asks: packing runs the game's code on fibers
(`PaletteProbe` in `assets.cpp`), so the next browser frame does it in
`play.cpp`'s `add_original`, as the game's own ticks are run, and then calls
`Module.originalPacked`. The frames now go on after the game ends, so the
zip can still be added then. The executable is found at the zip's top or in
one folder; its size and SHA-256 (`stone::sha256`, checked by
`stone_tests`) are checked before anything is written, and the songs are
looked for beside it under the names it gives. On the web
`find_original_package` takes `/save/original.dat` as it is, and one that
does not open is left out with a message in the console, so the free set
still plays. **Remove the original game** shows only while `original.dat`
is there and asks first.

Checked in Chromium with Playwright: a zip without the executable, one with
it two folders deep, and one with a 452,788-byte executable that is not the
original are each refused with a message, and leave nothing in `/save` or
`/tmp`; an `original.dat` put in `/save` is found after a reload, and
**Remove the original game** deletes it from IndexedDB. The original game
is not in the repository, so packing a real zip of it in the browser is
still to be checked by someone who has it: Options F2 should then offer
ORIG.

### 4. The page

`web/shell.html` holds the canvas and a little HTML and CSS around it:

- **Click to play.** Browsers start sound only after the user clicks or
  presses a key, and the click also gives the canvas the keyboard.
- The canvas scales to the window at a whole-number multiple of 320×200
  where it fits, with `image-rendering: pixelated`. There is a fullscreen
  button.
- **Keys:** `SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT` is set to the canvas, so
  the page keeps its keys while the game does not have focus. While it has
  focus, F1–F10, Tab, Space and the arrows must not reach the browser (F5
  would reload the page, Space would scroll it). Where SDL does not already
  prevent their default action, a `keydown` listener in the shell does.
- In fullscreen, Esc always leaves fullscreen. Where the browser has the
  Keyboard Lock API (Chromium), the fullscreen button locks Esc for the game.
  Elsewhere the page says that Esc leaves fullscreen first.
- A short key list under the canvas (from `README.md`, "Run"), the
  buttons from milestones 2 and 3, and the footer described under Licences below.

**Done when:** every key in `README.md`'s list works in Chromium and
Firefox without the browser acting on it, and name entry in the high-score
table accepts typed text.

**Done.** The game stands still until the canvas has the keyboard: a
button over it says "Click to play" (and "Play again" once the game has
ended), and it comes back when the canvas loses focus, as the desktop
player stands still without focus (`stone_focus` in `play.cpp`, which also
releases a held key). The canvas is sized in device pixels, so a whole
multiple stays sharp on a 1.25 or 1.5 screen; the window is made with
`SDL_WINDOW_HIGH_PIXEL_DENSITY` and `SDL_WINDOW_OPENGL` (made without it,
SDL recreates it for WebGL and drops the page's size). Fullscreen shows the
canvas and its button on a black screen. SDL already stops F1–F12, Tab, the
arrows and Backspace, and Space except as typed text; the shell's listener
adds PgUp/PgDn, Home and End. The footer's source link comes from
`STONE_SOURCE_URL` (the repository by default; the workflow gives the commit).

Checked in Chromium with Playwright, at device pixel ratios 1, 1.25 and
1.5: the canvas is 3× or 4× 320×200 in device pixels and follows the
window; F1–F10, Tab, Space, the arrows, PgUp/PgDn, Home, End, Esc and
Backspace neither reload, scroll nor move focus; fullscreen fills the
screen at a whole multiple and returns the canvas's size after. With a
score table any score reaches (imported with **Import saves**), the name
typed after F3 and Esc, corrected with Backspace and ended with Enter is
in the exported `FREEMAPS.HI`. Firefox could not be installed in the cloud
session (its download is blocked there), so the keys in Firefox, and Esc
in fullscreen with the Keyboard Lock API in a headed Chromium, are still to
be checked by hand.

### 5. Build and publish with GitHub Actions

`.github/workflows/web.yml`:

1. Check out the repository.
2. Build the `core` preset with the runner's compiler, and pack
   `assets/` into `stoneage.dat` with that `stone_assets`.
3. Install the pinned emsdk (cached between runs), configure the `wasm`
   preset with `STONE_PACKAGE` pointing at that package, and build.
4. Put `index.html`, the `.js`, `.wasm` and `.data` files, `LICENSE`,
   `LEGAL.md` and `third_party/licenses/` into one folder.
5. Upload it with `actions/upload-pages-artifact`, and publish it with
   `actions/deploy-pages`. Pull requests run steps 1 to 4 only, so a broken
   web build shows on the pull request and is never published.

One-off setting in the repository: **Settings → Pages → Build and
deployment → Source: GitHub Actions**. GitHub Pages on a free account needs
the repository to be public. The site's address is
`https://crpcrp.github.io/stone-age-enhanced/`; every path in the page is
relative, so it works from that subfolder.

**Done when:** a push to `main` updates the site, and a pull request shows
the web build's check.

**Done**, as written, with these details: the core's `stone_tests` run
after it is built; emsdk is installed with `mymindstorm/setup-emsdk`,
which caches it; Ninja comes from apt. The steps were run in the cloud
session as the workflow runs them. The site goes up only once Pages'
source is set to GitHub Actions in the repository's settings (above);
until then the deploy job fails on `main`.

### 6. Checks

- **Same game as the desktop.** The web build reads `?ticks=N&keys=URL`
  as `--ticks` and `--keys` do, and then copies the frame into an element
  of the page. A Playwright script (Chromium is installed in the cloud
  sessions) runs a few key scripts on both builds and compares the web frame
  with the desktop player's `--screenshot`, pixel for pixel. A run starts
  with `machine.seconds` at 0, as scripted desktop runs do, so the same
  keys give the same game.
- `stone_tests` keeps running in the native build; the web build adds no
  game logic of its own to test.
- The workflow runs the Playwright check before it publishes.

### 7. Documents

- `README.md`: a link to the site under a "Play in the browser" heading,
  and the zip of the original under it.
- `BUILDING.md`: the `wasm` preset; "Distribution" changes from "source
  only, never a built player" to source plus the web version built by the
  workflow; "Limits" names the browser limits below.
- `LEGAL.md`: the web version, and that the original's files added on the
  page stay in the player's browser.
- This file is deleted once the milestones are done; what remains true
  moves into those three.

**Done**, except deleting this file, which waits for milestone 6. `README.md`
has "Play in the browser"; `BUILDING.md` has the `wasm` preset under "Web
version", the browser limits under "Limits" and the web version under
"Distribution"; `LEGAL.md` has the web version and a section for rights
holders.

## Licences

The web version links SDL3 (zlib) and ymfm (BSD 3-Clause); it linked DOSBox's
GPL-licensed DBOPL until the player's own licence, which grants none, was
found not to allow that. The page's footer names the licences, links to
`LICENSE`, `LEGAL.md` and the notices published with it, and links to the
source at the exact commit it was built from (`github.sha` in the workflow).

## Limits of the web version

- **Browser keys:** the browser always keeps some keys (Ctrl+W, Ctrl+T,
  Ctrl+N). The game uses none of them. Esc in fullscreen is covered under
  milestone 4.
- **Phones and tablets:** the game needs a keyboard. Touch buttons for the
  arrows, Space, Esc, P, Q and F1–F10 can come later; they are not part of
  this plan.
- **Saves:** they belong to one browser on one site and can be deleted by
  it; export and import (milestone 2) are the way to keep them.
- **Frame pacing:** 70 ticks a second on a 60 Hz screen gives one frame in
  six two ticks, as the desktop player already does.
- **Exposure:** a public, playable page is more visible than a source
  repository, and the program is derived from the original's code
  (`LEGAL.md`). A takedown sent to GitHub would cover the site and the
  repository together. Accepted by choosing GitHub Pages.

## Order and size

Milestones 1 and 2 come first and are most of the work; the fiber hand-off
in `machine.cpp` is the part that needs the most care. Milestones 3 to 5
are a day or two together. Milestones 6 and 7 finish it. Each milestone is
one pull request, and the site is first published at the end of milestone 5.
