#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "libretro_core.h"
#include "ui.h"
#include "lua_api.h"
#include "input_backend.h"
#include "zip.h"

retro_video_refresh_t video_cb;
retro_audio_sample_batch_t audio_batch_cb;
retro_input_poll_t input_poll_cb;
retro_input_state_t input_state_cb;
static retro_environment_t environ_cb;
static retro_log_printf_t log_cb;

static char current_game_dir[512] = "";
static bool game_loaded = false;

unsigned retro_api_version(void) {
    return RETRO_API_VERSION;
}

unsigned retro_get_region(void) {
    return RETRO_REGION_NTSC;
}

void retro_get_system_info(struct retro_system_info *info) {
    memset(info, 0, sizeof(*info));
    info->library_name     = "Lupi";
    info->library_version  = "1.0";
    info->need_fullpath    = false;
    info->valid_extensions = "lupi";
    info->block_extract    = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info) {
    memset(info, 0, sizeof(*info));
    info->timing.fps            = 60.0;
    info->timing.sample_rate    = 0;
    
    info->geometry.base_width   = SCREEN_WIDTH;
    info->geometry.base_height  = SCREEN_HEIGHT;
    info->geometry.max_width    = SCREEN_WIDTH;
    info->geometry.max_height   = SCREEN_HEIGHT;
    info->geometry.aspect_ratio = 0;
}

void retro_set_environment(retro_environment_t cb) {
    environ_cb = cb;
    
    bool no_content = true;
    cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);
    
    struct retro_log_callback log;
    if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log)) {
        log_cb = log.log;
    }
}

void retro_set_audio_sample(retro_audio_sample_t cb) {
    (void)cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {
    audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb) {
    input_poll_cb = cb;
    if (log_cb) log_cb(RETRO_LOG_INFO, "[Lupi] Input poll callback set\n");
}

void retro_set_input_state(retro_input_state_t cb) {
    input_state_cb = cb;
    if (log_cb) log_cb(RETRO_LOG_INFO, "[Lupi] Input state callback set\n");
}

void retro_set_video_refresh(retro_video_refresh_t cb) {
    video_cb = cb;
}

void retro_init(void) {
    if (log_cb) log_cb(RETRO_LOG_INFO, "[Lupi] Initializing...\n");
    game_loaded = false;
}

void retro_deinit(void) {
    if (log_cb) log_cb(RETRO_LOG_INFO, "[Lupi] Deinitializing...\n");
    if (game_loaded) {
        lua_api_close();
        game_loaded = false;
    }
}

void retro_reset(void) {
    if (game_loaded) {
        lua_api_close();
        lua_api_init();
        reset_ui_state();
        lua_api_setup_game(current_game_dir);
    }
}

bool retro_load_game(const struct retro_game_info *game) {
    if (!game) return false;
    
    if (log_cb) log_cb(RETRO_LOG_INFO, "[Lupi] Loading game, size: %zu bytes\n", game->size);
    
    if (!create_temp_dir(current_game_dir)) {
        if (log_cb) log_cb(RETRO_LOG_ERROR, "[Lupi] Failed to create temp dir\n");
        return false;
    }
    
    if (extract_lupi_from_memory(game->data, game->size, current_game_dir) != 0) {
        if (log_cb) log_cb(RETRO_LOG_ERROR, "[Lupi] Failed to extract game\n");
        return false;
    }
    
    if (log_cb) log_cb(RETRO_LOG_INFO, "[Lupi] Extracted to: %s\n", current_game_dir);
    
    lua_api_init();
    input_init();
    
    if (log_cb) log_cb(RETRO_LOG_INFO, "[Lupi] Lua and input initialized\n");
    
    lua_api_setup_game(current_game_dir);
    
    if (log_cb) log_cb(RETRO_LOG_INFO, "[Lupi] Game setup complete\n");
    
    game_loaded = true;
    
    return true;
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info) {
    (void)game_type;
    (void)info;
    (void)num_info;
    return false;
}

void retro_unload_game(void) {
    if (game_loaded) {
        lua_api_close();
        game_loaded = false;
    }
    
    if (current_game_dir[0] != '\0') {
        cleanup_lupi_tmp(current_game_dir);
        current_game_dir[0] = '\0';
    }
}

void retro_run(void) {
    if (!game_loaded) return;
    
    input_poll();
    
    if (globalLuaState) {
        lua_api_call_update();
    }
    
    draw_frame_buffer_libretro();
    
    clear_frame_buffer();
}

size_t retro_serialize_size(void) {
    return 512 * 1024;
}

bool retro_serialize(void *data, size_t size) {
    if (!game_loaded || size < retro_serialize_size()) return false;
    
    unsigned char *buffer = (unsigned char *)data;
    memset(buffer, 0, size);
    
    memcpy(buffer + 64, &frame_buffer, sizeof(frame_buffer));
    memcpy(buffer + 64 + sizeof(frame_buffer), palette, sizeof(palette));
    
    int camera_data[2] = {camera_x, camera_y};
    memcpy(buffer + 128, camera_data, sizeof(camera_data));
    
    return true;
}

bool retro_unserialize(const void *data, size_t size) {
    if (!game_loaded || size < retro_serialize_size()) return false;
    
    const unsigned char *buffer = (const unsigned char *)data;
    
    memcpy(&frame_buffer, buffer + 64, sizeof(frame_buffer));
    memcpy(palette, buffer + 64 + sizeof(frame_buffer), sizeof(palette));
    
    int camera_data[2];
    memcpy(camera_data, buffer + 128, sizeof(camera_data));
    camera_x = camera_data[0];
    camera_y = camera_data[1];
    
    return true;
}

void retro_cheat_reset(void) {}

void retro_cheat_set(unsigned index, bool enabled, const char *code) {
    (void)index;
    (void)enabled;
    (void)code;
}

size_t retro_get_memory_size(unsigned id) {
    (void)id;
    return 0;
}

void *retro_get_memory_data(unsigned id) {
    (void)id;
    return NULL;
}

void retro_controller_info(unsigned port, struct retro_controller_info *info) {
    (void)port;
    info->types = NULL;
    info->num_types = 0;
}

void retro_set_controller_port_device(unsigned port, unsigned device) {
    (void)port;
    (void)device;
}
