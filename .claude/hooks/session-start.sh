#!/bin/bash
# Claude Code cloud sessions: build the player without a display, so that
# scripted runs (--keys, --ticks, --screenshot) work in the container, and
# build stone_tests. The cloud container has no X11 or Wayland, so SDL is
# built for the console only and draws off-screen.
# It runs in the background while the session starts: the build is done
# when build/cmake-headless/ready exists.
set -euo pipefail

if [ "${CLAUDE_CODE_REMOTE:-}" != "true" ]; then
  exit 0
fi

echo '{"async": true, "asyncTimeout": 600000}'

# SDL's drivers for a run without a screen or sound card.
echo 'export SDL_VIDEO_DRIVER=offscreen' >> "$CLAUDE_ENV_FILE"
echo 'export SDL_AUDIO_DRIVER=dummy' >> "$CLAUDE_ENV_FILE"

cd "$CLAUDE_PROJECT_DIR"
build_tree=build/cmake-headless
rm -f "$build_tree/ready"
cmake -S . -B "$build_tree" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DSDL_UNIX_CONSOLE_BUILD=ON -DSDL_X11=OFF -DSDL_WAYLAND=OFF -DSDL_ALSA=OFF >/dev/null
cmake --build "$build_tree" >/dev/null
touch "$build_tree/ready"
