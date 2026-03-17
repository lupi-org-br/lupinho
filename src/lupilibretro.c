#include "libretro_core.h"
#include "input_backend.h"

void libretro_init_callbacks(void) {
    input_set_callbacks(input_poll_cb, input_state_cb);
}
