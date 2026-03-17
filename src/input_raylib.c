#include "raylib.h"
#include "input_backend.h"

static bool btn_state[INPUT_BCOUNT];
static bool btn_prev_state[INPUT_BCOUNT];

static int key_map[INPUT_BCOUNT] = {
    [INPUT_BUP]    = KEY_UP,
    [INPUT_BDOWN]  = KEY_DOWN,
    [INPUT_BLEFT]  = KEY_LEFT,
    [INPUT_BRIGHT] = KEY_RIGHT,
    [INPUT_BA]     = KEY_Z,
    [INPUT_BB]     = KEY_X,
    [INPUT_BX]     = KEY_A,
    [INPUT_BL]     = KEY_Q,
    [INPUT_BR]     = KEY_W,
};

static int gp_map[INPUT_BCOUNT] = {
    [INPUT_BUP]    = GAMEPAD_BUTTON_LEFT_FACE_UP,
    [INPUT_BDOWN]  = GAMEPAD_BUTTON_LEFT_FACE_DOWN,
    [INPUT_BLEFT]  = GAMEPAD_BUTTON_LEFT_FACE_LEFT,
    [INPUT_BRIGHT] = GAMEPAD_BUTTON_LEFT_FACE_RIGHT,
    [INPUT_BA]     = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,
    [INPUT_BB]     = GAMEPAD_BUTTON_RIGHT_FACE_UP,
    [INPUT_BX]     = GAMEPAD_BUTTON_RIGHT_FACE_LEFT,
    [INPUT_BL]     = GAMEPAD_BUTTON_LEFT_TRIGGER_1,
    [INPUT_BR]     = GAMEPAD_BUTTON_RIGHT_TRIGGER_1,
};

void input_init(void) {
    for (int i = 0; i < INPUT_BCOUNT; i++) {
        btn_state[i] = false;
        btn_prev_state[i] = false;
    }
}

void input_poll(void) {
    for (int i = 0; i < INPUT_BCOUNT; i++) {
        btn_prev_state[i] = btn_state[i];
    }
    
    for (int i = 0; i < INPUT_BCOUNT; i++) {
        bool keyboard = IsKeyDown(key_map[i]);
        bool gamepad = IsGamepadButtonDown(0, gp_map[i]);
        btn_state[i] = keyboard || gamepad;
    }
}

bool input_btn_down(input_button_t btn) {
    if (btn < 0 || btn >= INPUT_BCOUNT) return false;
    return btn_state[btn];
}

bool input_btn_pressed(input_button_t btn) {
    if (btn < 0 || btn >= INPUT_BCOUNT) return false;
    return btn_state[btn] && !btn_prev_state[btn];
}
