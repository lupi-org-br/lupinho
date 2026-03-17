#ifndef INPUT_BACKEND_H
#define INPUT_BACKEND_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    INPUT_BUP = 0,
    INPUT_BDOWN,
    INPUT_BLEFT,
    INPUT_BRIGHT,
    INPUT_BA,
    INPUT_BB,
    INPUT_BX,
    INPUT_BL,
    INPUT_BR,
    INPUT_BCOUNT
} input_button_t;

void input_init(void);
void input_poll(void);
bool input_btn_down(input_button_t btn);
bool input_btn_pressed(input_button_t btn);

#ifdef LIBRETRO
#include "libretro/libretro.h"
void input_set_callbacks(retro_input_poll_t poll_cb, retro_input_state_t state_cb);
#endif

#define GAMEPAD_BUTTON_LEFT_FACE_UP    INPUT_BUP
#define GAMEPAD_BUTTON_LEFT_FACE_DOWN  INPUT_BDOWN
#define GAMEPAD_BUTTON_LEFT_FACE_LEFT  INPUT_BLEFT
#define GAMEPAD_BUTTON_RIGHT_FACE_RIGHT INPUT_BRIGHT
#define GAMEPAD_BUTTON_RIGHT_FACE_DOWN INPUT_BDOWN
#define GAMEPAD_BUTTON_RIGHT_FACE_UP   INPUT_BB
#define GAMEPAD_BUTTON_RIGHT_FACE_LEFT INPUT_BX
#define GAMEPAD_BUTTON_LEFT_TRIGGER_1  INPUT_BL
#define GAMEPAD_BUTTON_RIGHT_TRIGGER_1 INPUT_BR

#endif
