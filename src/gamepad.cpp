// A gamepad as a keyboard (ENHANCEMENTS.md, E6); see gamepad.hpp.
#include "gamepad.hpp"
#include <SDL3/SDL_stdinc.h>
#include <cstdlib>
#include <cstring>
#include <utility>

namespace stone {
namespace {
// Scan codes.
const uint8_t esc = 0x01, key_q = 0x10, key_p = 0x19, enter = 0x1c, space = 0x39, arrow_up = 0x48,
              page_up = 0x49, arrow_left = 0x4b, arrow_right = 0x4d, arrow_down = 0x50,
              page_down = 0x51;
const uint64_t millisecond = 1000000;
// Back must be held this long to retry, so it is not pressed by accident.
const uint64_t back_hold = 500 * millisecond;
// Up or down held while typing: the letters go round after the first wait,
// then a letter each repeat.
const uint64_t first_repeat = 400 * millisecond, repeat = 110 * millisecond;
// Of the stick's 32,767 to either side, the middle that counts as nothing.
const int dead_zone = 12000;
} // namespace

Gamepad::Gamepad(Machine& machine, std::function<void(uint8_t)> key)
    : m(machine), key(std::move(key)) {}

Gamepad::~Gamepad() { close(); }

void Gamepad::open(SDL_JoystickID which) {
    pad = SDL_OpenGamepad(which);
    id = pad ? which : 0;
}

void Gamepad::close() {
    if (!pad)
        return;
    SDL_CloseGamepad(pad);
    pad = nullptr;
    id = 0;
}

void Gamepad::press(uint8_t code, bool pressed) { key(pressed ? code : uint8_t(code | 128)); }

void Gamepad::handle(const SDL_Event& event) {
    const uint64_t now = event.common.timestamp;
    switch (event.type) {
    case SDL_EVENT_GAMEPAD_ADDED:
        if (!pad)
            open(event.gdevice.which);
        return;
    case SDL_EVENT_GAMEPAD_REMOVED:
        if (event.gdevice.which != id)
            return;
        // What it held is let go, and the next one connected takes over.
        stick_x = stick_y = 0;
        stick(now);
        if (pad_direction != none) {
            pad_direction = none;
            direction(none, now);
        }
        if (back_sent)
            press(key_q, false);
        back_held = back_sent = false;
        close();
        {
            int count = 0;
            SDL_JoystickID* const pads = SDL_GetGamepads(&count);
            if (pads && count)
                open(pads[0]);
            SDL_free(pads);
        }
        return;
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
        if (event.gbutton.which == id)
            button(SDL_GamepadButton(event.gbutton.button), event.gbutton.down, now);
        return;
    case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        if (event.gaxis.which != id)
            return;
        if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX)
            stick_x = event.gaxis.value;
        else if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY)
            stick_y = event.gaxis.value;
        else
            return;
        stick(now);
        return;
    default: return;
    }
}

// Presses go to the game only in enhanced mode; releases always, so nothing
// stays held when it is switched off.
void Gamepad::button(SDL_GamepadButton which, bool pressed, uint64_t now) {
    if (pressed && !m.enhanced)
        return;
    const Direction directions[] = {up, down, left, right};
    if (which >= SDL_GAMEPAD_BUTTON_DPAD_UP && which <= SDL_GAMEPAD_BUTTON_DPAD_RIGHT) {
        const Direction to = directions[which - SDL_GAMEPAD_BUTTON_DPAD_UP];
        if (pressed)
            pad_direction = to;
        else if (pad_direction == to)
            pad_direction = none;
        else
            return;
        direction(pad_direction != none ? pad_direction : stick_direction, now);
        return;
    }
    if (which == SDL_GAMEPAD_BUTTON_BACK) {
        if (pressed) {
            back_held = true;
            back_since = now;
        } else {
            if (back_sent)
                press(key_q, false);
            back_held = back_sent = false;
        }
        return;
    }
    if (typing) {
        if (which == SDL_GAMEPAD_BUTTON_SOUTH && pressed)
            finish_typing();
        return;
    }
    uint8_t code;
    switch (which) {
    case SDL_GAMEPAD_BUTTON_SOUTH: code = enter; break;
    case SDL_GAMEPAD_BUTTON_WEST: code = space; break;
    case SDL_GAMEPAD_BUTTON_EAST: code = esc; break;
    case SDL_GAMEPAD_BUTTON_START: code = key_p; break;
    case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: code = page_up; break;
    case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: code = page_down; break;
    default: return;
    }
    press(code, pressed);
}

void Gamepad::stick(uint64_t now) {
    Direction to = none;
    if (m.enhanced && (std::abs(stick_x) > dead_zone || std::abs(stick_y) > dead_zone)) {
        if (std::abs(stick_x) > std::abs(stick_y))
            to = stick_x < 0 ? left : right;
        else
            to = stick_y < 0 ? up : down;
    }
    if (to == stick_direction)
        return;
    stick_direction = to;
    direction(pad_direction != none ? pad_direction : stick_direction, now);
}

// The arrow held changes: the old one is released and the new one pressed.
void Gamepad::direction(Direction to, uint64_t now) {
    if (to == held)
        return;
    const uint8_t arrows[] = {0, arrow_up, arrow_down, arrow_left, arrow_right};
    if (typing) {
        held = to;
        type(to);
        repeat_at = now + first_repeat;
        return;
    }
    if (held != none)
        press(arrows[held], false);
    held = to;
    if (held != none)
        press(arrows[held], true);
}

void Gamepad::update(uint64_t now) {
    if (m.reads_characters() != typing) {
        // Arrows held as typing starts are let go first; after it they
        // become keys again from the next press.
        if (!typing && held != none) {
            const uint8_t arrows[] = {0, arrow_up, arrow_down, arrow_left, arrow_right};
            press(arrows[held], false);
        }
        typing = !typing;
        held = none;
        pending = false;
        letter = length = 0;
    }
    if (back_held && !back_sent && now - back_since >= back_hold) {
        back_sent = true;
        press(key_q, true);
    }
    if (typing && (held == up || held == down) && now >= repeat_at) {
        type(held);
        repeat_at = now + repeat;
    }
}

// The password takes letters and digits; a name for the score table also
// blanks and a few signs.
const char* Gamepad::alphabet() const {
    return m.showing == Machine::password ? "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
                                          : "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 .-!?'";
}

// The letter under the cursor is typed, and replaced by deleting it and
// typing the next. The game takes six letters of a password and 21 of a
// name and ignores more, so no more are typed.
void Gamepad::type(Direction to) {
    const unsigned count = unsigned(std::strlen(alphabet()));
    const unsigned limit = m.showing == Machine::password ? 6 : 21;
    switch (to) {
    case up:
    case down:
        if (pending) {
            m.type(8);
            letter = (letter + (to == up ? 1 : count - 1)) % count;
        } else if (length >= limit)
            return;
        m.type(uint8_t(alphabet()[letter % count]));
        pending = true;
        break;
    case right:
        if (pending) {
            pending = false;
            ++length;
        } else if (m.showing != Machine::password && length < limit) {
            m.type(' ');
            ++length;
        }
        break;
    case left:
        if (pending)
            pending = false;
        else if (length)
            --length;
        else
            break;
        m.type(8);
        break;
    case none: break;
    }
}

void Gamepad::finish_typing() {
    if (pending) {
        pending = false;
        ++length;
    }
    m.type(13);
}
} // namespace stone
