#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "ui.h"
#include "lua_api.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#if defined(PLATFORM_WEB)
static volatile int game_is_loading = 0;
#endif

void UpdateDrawFrame() {
#if defined(PLATFORM_WEB)
    if (game_is_loading) return;
#endif
    lua_api_call_update();

    BeginDrawing();

    ClearBackground(RAYWHITE);

    draw_frame_buffer();

    #ifndef PRODUCTION
    DrawFPS(10, 10); // DEBUG
    #endif

    EndDrawing();

    clear_frame_buffer();
}

#if defined(PLATFORM_WEB)
EMSCRIPTEN_KEEPALIVE
void lupi_load_game(const char *game_dir) {
    game_is_loading = 1;
    lua_api_close();
    lua_api_init();
    reset_ui_state();
    lua_api_setup_game(game_dir);
    game_is_loading = 0;
}
#endif

int main() {
    lua_api_init();

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lupi Emulator");

    SetTargetFPS(60);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif

    CloseWindow();
    lua_api_close();
    return 0;
}
