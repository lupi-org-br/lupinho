#ifndef FUR_PLAYER_H
#define FUR_PLAYER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Furnace module player. One engine, looped playback, stereo float output. */

int fur_player_init(void);
void fur_player_shutdown(void);
int fur_player_sample_rate(void);

/* Copies the file bytes. name is a hint such as "song.fur". */
int fur_player_load(const unsigned char *data, size_t len, const char *name);
void fur_player_stop(void);
int fur_player_playing(void);

/* Interleaved stereo float, engine sample rate. Silence when nothing plays. */
void fur_player_render(float *interleaved, unsigned int frames);

#ifdef __cplusplus
}
#endif

#endif
