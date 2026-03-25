#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"
#include "ui.h"
#include "lua_api.h"

#if !defined(PLATFORM_WEB)
#include "zip.h"
#endif

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#if defined(PLATFORM_WEB)
static volatile int game_is_loading = 0;
#endif

#if !defined(PLATFORM_WEB)
static char temp_game_dir[512] = "";
#endif

void UpdateDrawFrame() {
#if defined(PLATFORM_WEB)
    if (game_is_loading) return;
#endif
    lua_api_call_update();

    BeginDrawing();

    ClearBackground(BLACK);

    draw_frame_buffer();

    #ifndef PRODUCTION
    // DrawFPS(10, 10); // DEBUG
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

#if !defined(PLATFORM_WEB)
static void print_usage(const char *program_name) {
    printf("Usage: %s [game.lupi | game_directory]\n", program_name);
    printf("\nLupi Emulator - Run Lupi games on Linux\n");
    printf("\nArguments:\n");
    printf("  game.lupi         Path to a .lupi archive file\n");
    printf("  game_directory    Path to an unpacked game directory\n");
    printf("\nExamples:\n");
    printf("  %s balao-gatinho.lupi\n", program_name);
    printf("  %s game-example/\n", program_name);
}

static int ends_with(const char *str, const char *suffix) {
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) return 0;
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}
#endif

#if defined(PLATFORM_WEB)
int main() {
    lua_api_init();

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lupi Emulator");
    SetTargetFPS(60);

    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);

    CloseWindow();
    lua_api_close();
    return 0;
}
#else
int main(int argc, char *argv[]) {
    const char *game_dir = NULL;
    int extracted_to_tmp = 0;

    // Parse command-line arguments
    if (argc < 2) {
        fprintf(stderr, "Error: No game specified\n");
        print_usage(argv[0]);
        return 1;
    }

    if (argc > 2) {
        fprintf(stderr, "Error: Too many arguments\n");
        print_usage(argv[0]);
        return 1;
    }

    // Check for help flag
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    // Determine if input is a .lupi file or directory
    if (ends_with(argv[1], ".lupi")) {
        // Extract .lupi file to temp directory
        printf("Extracting %s...\n", argv[1]);
        if (extract_lupi_to_tmp(argv[1], temp_game_dir, sizeof(temp_game_dir)) != 0) {
            fprintf(stderr, "Failed to extract %s\n", argv[1]);
            return 1;
        }
        printf("Extracted to: %s\n", temp_game_dir);
        game_dir = temp_game_dir;
        extracted_to_tmp = 1;
    } else {
        // Use directory directly
        game_dir = argv[1];
    }

    // Initialize Lua
    lua_api_init();

    // Set up the game
    lua_api_setup_game(game_dir);

    // Initialize Raylib window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lupi Emulator");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }

    // Cleanup
    CloseWindow();
    lua_api_close();

    // Clean up temp directory if we extracted a .lupi file
    if (extracted_to_tmp) {
        cleanup_lupi_tmp(temp_game_dir);
    }

    return 0;
}
#endif
