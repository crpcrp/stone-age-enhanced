#pragma once
// A gamepad as a keyboard (ENHANCEMENTS.md, E6). Not original: enhanced mode
// only. The first gamepad connected is used, and gamepads can come and go
// while the game runs. Its buttons become the scan bytes the keyboard sends,
// so the game and its determinism are untouched:
//   D-pad, left stick   arrow keys (the stick has a dead zone and gives four
//                       directions: the stronger axis wins)
//   A (south)           Enter
//   X (west)            Space
//   B (east)            Esc
//   Start               P (pause)
//   Back                Q (retry), once it has been held half a second
//   LB, RB              PgUp, PgDn
// While the game reads characters (the password, a name for the score table)
// up and down change the letter under the cursor, right accepts it and moves
// on, left deletes and A finishes with Enter.
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>
#include "machine.hpp"
#include <cstdint>
#include <functional>

namespace stone {
class Gamepad {
public:
    // `key` gets each scan byte, 80h added for a release.
    Gamepad(Machine& machine, std::function<void(uint8_t)> key);
    ~Gamepad();
    Gamepad(const Gamepad&) = delete;
    Gamepad& operator=(const Gamepad&) = delete;
    // A gamepad event; others are left alone.
    void handle(const SDL_Event& event);
    // Once a frame, with the time in nanoseconds: Back held long enough, the
    // letters going round while up or down is held.
    void update(uint64_t now);

private:
    enum Direction { none, up, down, left, right };
    void open(SDL_JoystickID id);
    void close();
    void press(uint8_t code, bool pressed);
    void button(SDL_GamepadButton button, bool pressed, uint64_t now);
    void direction(Direction to, uint64_t now);
    void stick(uint64_t now);
    // Typing: what a direction or A does while the game reads characters.
    void type(Direction to);
    void finish_typing();
    const char* alphabet() const;

    Machine& m;
    std::function<void(uint8_t)> key;
    SDL_Gamepad* pad = nullptr;
    SDL_JoystickID id = 0;
    int stick_x = 0, stick_y = 0;
    Direction pad_direction = none, stick_direction = none, held = none;
    uint64_t back_since = 0, repeat_at = 0;
    bool back_held = false, back_sent = false;
    // Typing: the letter shown under the cursor (a place in the alphabet),
    // whether it is shown and not yet accepted, and how many are accepted.
    bool typing = false, pending = false;
    unsigned letter = 0, length = 0;
};
} // namespace stone
