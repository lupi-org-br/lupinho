#ifndef LUPI_INPUT_H
#define LUPI_INPUT_H

#include <stdbool.h>

#ifdef LIBRETRO
/* Numeric values match raylib GamepadButton so existing Lua games keep working. */
#define GAMEPAD_BUTTON_UNKNOWN            0
#define GAMEPAD_BUTTON_LEFT_FACE_UP       1
#define GAMEPAD_BUTTON_LEFT_FACE_RIGHT    2
#define GAMEPAD_BUTTON_LEFT_FACE_DOWN     3
#define GAMEPAD_BUTTON_LEFT_FACE_LEFT     4
#define GAMEPAD_BUTTON_RIGHT_FACE_UP      5
#define GAMEPAD_BUTTON_RIGHT_FACE_RIGHT   6
#define GAMEPAD_BUTTON_RIGHT_FACE_DOWN    7
#define GAMEPAD_BUTTON_RIGHT_FACE_LEFT    8
#define GAMEPAD_BUTTON_LEFT_TRIGGER_1     9
#define GAMEPAD_BUTTON_LEFT_TRIGGER_2    10
#define GAMEPAD_BUTTON_RIGHT_TRIGGER_1   11
#define GAMEPAD_BUTTON_RIGHT_TRIGGER_2   12
#define GAMEPAD_BUTTON_MIDDLE_LEFT       13
#define GAMEPAD_BUTTON_MIDDLE            14
#define GAMEPAD_BUTTON_MIDDLE_RIGHT      15
#define GAMEPAD_BUTTON_LEFT_THUMB        16
#define GAMEPAD_BUTTON_RIGHT_THUMB       17
#else
#include "raylib.h"
#endif

#define LUPI_MAX_PADS    2
#define LUPI_MAX_BUTTONS 18

void lupi_input_begin_frame(void);
void lupi_input_set(int pad, int button, bool down);
bool lupi_button_down(int pad, int button);
bool lupi_button_pressed(int pad, int button);

#endif /* LUPI_INPUT_H */
