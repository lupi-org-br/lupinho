#ifndef LIBRETRO_CORE_H
#define LIBRETRO_CORE_H

#include "libretro/libretro.h"

extern retro_video_refresh_t video_cb;
extern retro_input_poll_t input_poll_cb;
extern retro_input_state_t input_state_cb;

void libretro_init_callbacks(void);

#endif
