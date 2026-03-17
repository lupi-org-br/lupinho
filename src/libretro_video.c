#include <stdint.h>
#include <string.h>
#include "ui.h"
#include "libretro/libretro.h"
#include "libretro_core.h"

static uint16_t video_buffer[SCREEN_HEIGHT * SCREEN_WIDTH];

void draw_frame_buffer_libretro(void) {
    if (!video_cb) return;
    
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            Color c = get_palette_color(frame_buffer[y][x]);
            video_buffer[y * SCREEN_WIDTH + x] = 
                ((c.r >> 3) << 11) | ((c.g >> 3) << 5) | (c.b >> 3);
        }
    }
    
    video_cb(video_buffer, SCREEN_WIDTH, SCREEN_HEIGHT, sizeof(uint16_t) * SCREEN_WIDTH);
}
