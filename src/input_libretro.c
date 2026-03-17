#include "input_backend.h"
#include "libretro/libretro.h"

static retro_input_state_t input_state_cb = NULL;
static retro_input_poll_t input_poll_cb = NULL;

static bool btn_state[INPUT_BCOUNT];
static bool btn_prev_state[INPUT_BCOUNT];

static int retro_btn_map[INPUT_BCOUNT] = {
    [INPUT_BUP]    = RETRO_DEVICE_ID_JOYPAD_UP,
    [INPUT_BDOWN]  = RETRO_DEVICE_ID_JOYPAD_DOWN,
    [INPUT_BLEFT]  = RETRO_DEVICE_ID_JOYPAD_LEFT,
    [INPUT_BRIGHT] = RETRO_DEVICE_ID_JOYPAD_RIGHT,
    [INPUT_BA]     = RETRO_DEVICE_ID_JOYPAD_A,
    [INPUT_BB]     = RETRO_DEVICE_ID_JOYPAD_Y,
    [INPUT_BX]     = RETRO_DEVICE_ID_JOYPAD_X,
    [INPUT_BL]     = RETRO_DEVICE_ID_JOYPAD_L,
    [INPUT_BR]     = RETRO_DEVICE_ID_JOYPAD_R,
};

void input_set_callbacks(retro_input_poll_t poll_cb, retro_input_state_t state_cb) {
    input_poll_cb = poll_cb;
    input_state_cb = state_cb;
}

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
    
    if (!input_poll_cb || !input_state_cb) return;
    
    input_poll_cb();
    
    for (int i = 0; i < INPUT_BCOUNT; i++) {
        btn_state[i] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, retro_btn_map[i]);
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
