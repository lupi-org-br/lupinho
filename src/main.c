#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "ui.h"
#include "lua_api.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

void UpdateDrawFrame() {
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

int main() {
    lua_api_init();

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lupi Emulator");

    lua_api_setup_game("game-example");

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
