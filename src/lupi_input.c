#include "lupi_input.h"

#ifdef LIBRETRO

static bool button_down[LUPI_MAX_PADS][LUPI_MAX_BUTTONS];
static bool button_prev[LUPI_MAX_PADS][LUPI_MAX_BUTTONS];

void lupi_input_begin_frame(void) {
    for (int pad = 0; pad < LUPI_MAX_PADS; pad++) {
        for (int b = 0; b < LUPI_MAX_BUTTONS; b++) {
            button_prev[pad][b] = button_down[pad][b];
            button_down[pad][b] = false;
        }
    }
}

void lupi_input_set(int pad, int button, bool down) {
    if (pad < 0 || pad >= LUPI_MAX_PADS) return;
    if (button < 0 || button >= LUPI_MAX_BUTTONS) return;
    if (down) button_down[pad][button] = true;
}

bool lupi_button_down(int pad, int button) {
    if (pad < 0 || pad >= LUPI_MAX_PADS) return false;
    if (button < 0 || button >= LUPI_MAX_BUTTONS) return false;
    return button_down[pad][button];
}

bool lupi_button_pressed(int pad, int button) {
    if (pad < 0 || pad >= LUPI_MAX_PADS) return false;
    if (button < 0 || button >= LUPI_MAX_BUTTONS) return false;
    return button_down[pad][button] && !button_prev[pad][button];
}

#else

static int keyboard_key_for_button(int button) {
    switch (button) {
        case GAMEPAD_BUTTON_LEFT_FACE_UP:     return KEY_W;
        case GAMEPAD_BUTTON_LEFT_FACE_DOWN:   return KEY_S;
        case GAMEPAD_BUTTON_LEFT_FACE_LEFT:   return KEY_A;
        case GAMEPAD_BUTTON_LEFT_FACE_RIGHT:  return KEY_D;
        case GAMEPAD_BUTTON_RIGHT_FACE_RIGHT: return KEY_J;
        case GAMEPAD_BUTTON_RIGHT_FACE_DOWN:  return KEY_K;
        case GAMEPAD_BUTTON_RIGHT_FACE_UP:    return KEY_L;
        case GAMEPAD_BUTTON_RIGHT_FACE_LEFT:  return KEY_M;
        case GAMEPAD_BUTTON_LEFT_TRIGGER_1:   return KEY_G;
        case GAMEPAD_BUTTON_RIGHT_TRIGGER_1:  return KEY_H;
        default: return -1;
    }
}

void lupi_input_begin_frame(void) {}
void lupi_input_set(int pad, int button, bool down) {
    (void)pad;
    (void)button;
    (void)down;
}

bool lupi_button_down(int pad, int button) {
    if (IsGamepadButtonDown(pad, button)) return true;
    int key = keyboard_key_for_button(button);
    return key != -1 && IsKeyDown(key);
}

bool lupi_button_pressed(int pad, int button) {
    if (IsGamepadButtonPressed(pad, button)) return true;
    int key = keyboard_key_for_button(button);
    return key != -1 && IsKeyPressed(key);
}

#endif
