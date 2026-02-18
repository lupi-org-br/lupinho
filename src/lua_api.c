#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "drawlist.h"
#include "raylib.h"

//----------------------------------------------------------------------------------
// ui.draw_text(text:string, x:number, y:number)
//----------------------------------------------------------------------------------
int lua_draw_text(lua_State *L) {
    char *text = luaL_checkstring(L, 1);
    int x = (int)luaL_optnumber(L, 2, 10);
    int y = (int)luaL_optnumber(L, 3, 10);

    add_text(text, x, y);

    return 0;
}

//----------------------------------------------------------------------------------
// ui.draw_line(x1:number, y1:number, x2:number, y2:number, color:number)
//----------------------------------------------------------------------------------
int lua_draw_line(lua_State *L) {
    int x1 = (int)luaL_checknumber(L, 1);
    int y1 = (int)luaL_checknumber(L, 2);
    int x2 = (int)luaL_checknumber(L, 3);
    int y2 = (int)luaL_checknumber(L, 4);
    int color = (int)luaL_checknumber(L, 5);

    add_line(x1, y1, x2, y2, get_palette_color(color), color);

    return 0;
}

//----------------------------------------------------------------------------------
// ui.draw_rect(x:number, y:number, width:number, height:number, filled:bool, color:number)
//----------------------------------------------------------------------------------
int lua_draw_rect(lua_State *L) {
    int x = (int)luaL_checknumber(L, 1);
    int y = (int)luaL_checknumber(L, 2);
    int width = (int)luaL_checknumber(L, 3);
    int height = (int)luaL_checknumber(L, 4);
    bool filled = lua_toboolean(L, 5);
    int color = (int)luaL_checknumber(L, 6);

    add_rect(x, y, width, height, filled, color);

    return 0;
}

//----------------------------------------------------------------------------------
// ui.rect(x1, y1, x2, y2, color)
//----------------------------------------------------------------------------------
int lua_rect(lua_State *L) {
    int x1 = (int)luaL_checknumber(L, 1);
    int y1 = (int)luaL_checknumber(L, 2);
    int x2 = (int)luaL_checknumber(L, 3);
    int y2 = (int)luaL_checknumber(L, 4);
    int color = (int)luaL_checknumber(L, 5);

    add_rect(x1, y1, (x2 - x1), (y2 - y1), false, color);

    return 0;
}

//----------------------------------------------------------------------------------
// ui.rectfill(x1, y1, x2, y2, color)
//----------------------------------------------------------------------------------
int lua_rectfill(lua_State *L) {
    int x1 = (int)luaL_checknumber(L, 1);
    int y1 = (int)luaL_checknumber(L, 2);
    int x2 = (int)luaL_checknumber(L, 3);
    int y2 = (int)luaL_checknumber(L, 4);
    int color = (int)luaL_checknumber(L, 5);

    add_rect(x1, y1, (x2 - x1), (y2 - y1), true, color);

    return 0;
}

//----------------------------------------------------------------------------------
// ui.draw_circle(center_x:number, center_y:number, radius:number, filled:bool, color:number, border:bool, border_color:number)
//----------------------------------------------------------------------------------
int lua_draw_circle(lua_State *L) {
    int center_x = (int)luaL_checknumber(L, 1);
    int center_y = (int)luaL_checknumber(L, 2);
    int radius = (int)luaL_checknumber(L, 3);
    bool filled = lua_toboolean(L, 4);
    int color = (int)luaL_checknumber(L, 5);
    bool border = lua_toboolean(L, 6);
    int border_color = (int)luaL_checknumber(L, 7);

    add_circle(center_x, center_y, radius, filled, color, border, border_color);

    return 0;
}

//----------------------------------------------------------------------------------
// ui.circfill(x, y, radius, color)
//----------------------------------------------------------------------------------
int lua_circfill(lua_State *L) {
    int center_x = (int)luaL_checknumber(L, 1);
    int center_y = (int)luaL_checknumber(L, 2);
    int radius = (int)luaL_checknumber(L, 3);
    int color = (int)luaL_checknumber(L, 4);

    add_circle(center_x, center_y, radius, true, color, true, color);

    return 0;
}

//----------------------------------------------------------------------------------
// ui.trisfill(p1_x:int, p1_y:int, p2_x:int, p2_y:int, p3_x:int, p3_y:int, color:int)
//----------------------------------------------------------------------------------
int lua_trisfill(lua_State *L) {
    int p1_x = (int)luaL_checknumber(L, 1);
    int p1_y = (int)luaL_checknumber(L, 2);
    int p2_x = (int)luaL_checknumber(L, 3);
    int p2_y = (int)luaL_checknumber(L, 4);
    int p3_x = (int)luaL_checknumber(L, 5);
    int p3_y = (int)luaL_checknumber(L, 6);
    int color = (int)luaL_checknumber(L, 7);

    add_triangle(p1_x, p1_y, p2_x, p2_y, p3_x, p3_y, color);

    return 0;
}

//----------------------------------------------------------------------------------
// ui.palset(position:number, color:number)
//----------------------------------------------------------------------------------
int lua_palset(lua_State *L) {
    int position = (int)luaL_checknumber(L, 1);
    int color = (int)luaL_checknumber(L, 2);

    palset(position, color);

    return 0;
}

//----------------------------------------------------------------------------------
// ui.tile(spritesheet:table, tile_index:number, x:number, y:number)
// tile_index can have bit 10 (1024) set to flip horizontally
//----------------------------------------------------------------------------------
int lua_tile(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "path");
    const char *path = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "width");
    int width = (int)luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "height");
    int height = (int)luaL_checknumber(L, -1);
    lua_pop(L, 1);

    int tile_index_with_flags = (int)luaL_checknumber(L, 2);
    int x = (int)luaL_checknumber(L, 3) - camera_x;
    int y = (int)luaL_checknumber(L, 4) - camera_y;

    bool flipped = (tile_index_with_flags & 1024) != 0;
    int tile_index = tile_index_with_flags & ~1024;

    FILE *f = fopen(path, "rb");
    if (!f) {
        printf("Warning: Could not open tile file %s\n", path);
        return 0;
    }

    int tile_size = width * height;
    long data_offset = (long)tile_index * tile_size;
    fseek(f, data_offset, SEEK_SET);

    unsigned char *data = (unsigned char *) malloc(tile_size);
    if (!data) {
        fclose(f);
        return 0;
    }

    fread(data, 1, tile_size, f);
    fclose(f);

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            unsigned char idx = data[row * width + col];
            if (idx == 0) continue;

            int px = flipped ? (x + width - 1 - col) : (x + col);
            int py = y + row;

            fb_set(px, py, idx);
        }
    }

    free(data);
    return 0;
}

//----------------------------------------------------------------------------------
// ui.spr(spritesheet:table, x:number, y:number, flipped:bool = false)
//----------------------------------------------------------------------------------
int lua_spr(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "path");
    const char *path = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "width");
    int width = (int)luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "height");
    int height = (int)luaL_checknumber(L, -1);
    lua_pop(L, 1);

    int x = (int)luaL_checknumber(L, 2) - camera_x;
    int y = (int)luaL_checknumber(L, 3) - camera_y;

    bool flipped = false;
    if (lua_gettop(L) > 3) {
        flipped = lua_toboolean(L, 4);
    }

    FILE *f = fopen(path, "rb");
    if (!f) {
        printf("Warning: Could not open sprite file %s\n", path);
        return 0;
    }

    int data_size = width * height;
    unsigned char *data = (unsigned char *) malloc(data_size);
    if (!data) {
        fclose(f);
        return 0;
    }

    fread(data, 1, data_size, f);
    fclose(f);

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            unsigned char idx = data[row * width + col];
            if (idx == 0) continue;

            int px = flipped ? (x + width - 1 - col) : (x + col);
            int py = y + row;

            fb_set(px, py, idx);
        }
    }

    free(data);
    return 0;
}

//----------------------------------------------------------------------------------
// Helper function to get the keyboard key corresponding to a gamepad button
//----------------------------------------------------------------------------------
static int get_keyboard_key_for_button(int button) {
    switch (button) {
        // D-pad / Arrow keys
        case GAMEPAD_BUTTON_LEFT_FACE_UP:    return KEY_UP;
        case GAMEPAD_BUTTON_LEFT_FACE_DOWN:  return KEY_DOWN;
        case GAMEPAD_BUTTON_LEFT_FACE_LEFT:  return KEY_LEFT;
        case GAMEPAD_BUTTON_LEFT_FACE_RIGHT: return KEY_RIGHT;
        // Action buttons
        case GAMEPAD_BUTTON_RIGHT_FACE_RIGHT: return KEY_Z;  // BTN_Z
        case GAMEPAD_BUTTON_RIGHT_FACE_DOWN:  return KEY_Z;  // BTN_Z (alternative)
        case GAMEPAD_BUTTON_RIGHT_FACE_UP:    return KEY_X;  // BTN_Q
        case GAMEPAD_BUTTON_RIGHT_FACE_LEFT:  return KEY_A;  // BTN_E
        case GAMEPAD_BUTTON_LEFT_TRIGGER_1:   return KEY_Q;  // BTN_F
        case GAMEPAD_BUTTON_RIGHT_TRIGGER_1:  return KEY_W;  // BTN_G
        default: return -1;
    }
}

//----------------------------------------------------------------------------------
// ui.btn(button:number, pad:number) -> bool
// Checks both gamepad and keyboard input
//----------------------------------------------------------------------------------
int lua_btn(lua_State *L) {
    int button = (int)luaL_checknumber(L, 1);
    int pad = (int)luaL_optnumber(L, 2, 0);

    bool is_down = IsGamepadButtonDown(pad, button);

    if (!is_down) {
        int key = get_keyboard_key_for_button(button);
        if (key != -1) {
            is_down = IsKeyDown(key);
        }
    }

    lua_pushboolean(L, is_down);

    return 1;
}

//----------------------------------------------------------------------------------
// ui.btnp(button:number, pad:number) -> bool
// Checks both gamepad and keyboard input (pressed this frame)
//----------------------------------------------------------------------------------
int lua_btnp(lua_State *L) {
    int button = (int)luaL_checknumber(L, 1);
    int pad = (int)luaL_optnumber(L, 2, 0);

    bool is_pressed = IsGamepadButtonPressed(pad, button);

    if (!is_pressed) {
        int key = get_keyboard_key_for_button(button);
        if (key != -1) {
            is_pressed = IsKeyPressed(key);
        }
    }

    lua_pushboolean(L, is_pressed);

    return 1;
}

//----------------------------------------------------------------------------------
// ui.cls(color:int)
//----------------------------------------------------------------------------------
int lua_cls(lua_State *L) {
    int color = (int)luaL_checknumber(L, 1);
    add_clear(color);

    return 0;
}

//----------------------------------------------------------------------------------
// ui.log(message:string)
//----------------------------------------------------------------------------------
int lua_log(lua_State *L) {
    char *message = luaL_checkstring(L, 1);
    printf("[LUPINHO] %s\n", message);
    return 0;
}

//----------------------------------------------------------------------------------
// ui.fillp([pattern_bytes...])
//----------------------------------------------------------------------------------
int lua_fillp(lua_State *L) {
    int nargs = lua_gettop(L);

    if (nargs == 0) {
        memset(fill_pattern, 0, sizeof(fill_pattern));
        return 0;
    }

    for (int i = 0; i < nargs && i < 8; i++) {
        fill_pattern[i] = (uint8_t)(int)luaL_checknumber(L, i + 1);
    }

    // Fill remaining bytes with 0 if fewer than 8 arguments
    for (int i = nargs; i < 8; i++) {
        fill_pattern[i] = 0;
    }

    return 0;
}


// TODO

//----------------------------------------------------------------------------------
// ui.camera(x, y) — offset all drawing; ui.camera() resets to (0, 0)
//----------------------------------------------------------------------------------
int lua_camera(lua_State *L) {
    if (lua_gettop(L) == 0) {
        reset_camera();
    } else {
        int x = (int)luaL_checknumber(L, 1);
        int y = (int)luaL_checknumber(L, 2);
        set_camera(x, y);
    }
    return 0;
}

//----------------------------------------------------------------------------------
// ui.clip(x, y, w, h) — restrict drawing to region; ui.clip() resets
//----------------------------------------------------------------------------------
int lua_clip(lua_State *L) {
    if (lua_gettop(L) == 0) {
        reset_clip();
    } else {
        int x = (int)luaL_checknumber(L, 1);
        int y = (int)luaL_checknumber(L, 2);
        int w = (int)luaL_checknumber(L, 3);
        int h = (int)luaL_checknumber(L, 4);
        set_clip(x, y, w, h);
    }
    return 0;
}

//----------------------------------------------------------------------------------
// ui.preload_spritesheet(spritesheet)
//----------------------------------------------------------------------------------
int lua_preload_spritesheet(lua_State *L) {
    int spritesheet = (int)luaL_checknumber(L, 1);

    return 0;
}

//----------------------------------------------------------------------------------
// ui.draw_sprite(x, y, sprite_index, size)
//----------------------------------------------------------------------------------
int lua_draw_sprite(lua_State *L) {
    int x = (int)luaL_checknumber(L, 1);
    int y = (int)luaL_checknumber(L, 2);
    int sprite_index = (int)luaL_checknumber(L, 3);
    int size = (int)luaL_checknumber(L, 4);

    return 0;
}

//----------------------------------------------------------------------------------
// ui.print(text, x, y, color)
//----------------------------------------------------------------------------------
int lua_print(lua_State *L) {
    const char *text = luaL_checkstring(L, 1);
    int x = (int)luaL_checknumber(L, 2);
    int y = (int)luaL_checknumber(L, 3);
    int color = (int)luaL_checknumber(L, 4);

    draw_print(text, x, y, color);

    return 0;
}

//----------------------------------------------------------------------------------
// ui.set_pallet(start_index, count, palette_table)
//----------------------------------------------------------------------------------
int lua_set_pallet(lua_State *L) {
    int start_index = (int)luaL_checknumber(L, 1);
    int count = (int)luaL_checknumber(L, 2);
    luaL_checktype(L, 3, LUA_TTABLE);  // Check that argument 3 is a table

    return 0;
}

//----------------------------------------------------------------------------------
// Parse lupi_manifest.txt and inject a nested `Sprites` Lua global.
// Each bitmap entry becomes: Sprites[namespace][key] = { path, width, height, ntiles }
//----------------------------------------------------------------------------------
void inject_sprites_global(lua_State *L, const char *manifest_path, const char *game_dir) {
    FILE *f = fopen(manifest_path, "r");
    if (!f) {
        printf("Warning: Could not open manifest %s\n", manifest_path);
        return;
    }

    lua_newtable(L); // Sprites table

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        long id;
        int size;
        char path[256];
        char json[256];

        if (sscanf(line, "%ld %d %255s %255[^\n]", &id, &size, path, json) < 4)
            continue;

        if (!strstr(json, "\"type\":\"bitmap\""))
            continue;

        int width = 0, height = 0, ntiles = 1;
        char *p;

        p = strstr(json, "\"width\":");
        if (p) sscanf(p + 8, "%d", &width);

        p = strstr(json, "\"height\":");
        if (p) sscanf(p + 9, "%d", &height);

        p = strstr(json, "\"tiles\":");
        if (p) sscanf(p + 8, "%d", &ntiles);

        char *slash = strchr(path, '/');
        if (!slash) continue;

        *slash = '\0';
        const char *ns = path;
        const char *key = slash + 1;

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s/%s", game_dir, ns, key);

        // Get or create namespace sub-table
        lua_getfield(L, -1, ns);
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushvalue(L, -1);
            lua_setfield(L, -3, ns);
        }

        // Create entry table and set fields
        lua_newtable(L);
        lua_pushstring(L, full_path);
        lua_setfield(L, -2, "path");
        lua_pushinteger(L, width);
        lua_setfield(L, -2, "width");
        lua_pushinteger(L, height);
        lua_setfield(L, -2, "height");
        lua_pushinteger(L, ntiles);
        lua_setfield(L, -2, "ntiles");

        lua_setfield(L, -2, key); // ns_table[key] = entry
        lua_pop(L, 1);            // pop ns_table
    }

    fclose(f);
    lua_setglobal(L, "Sprites");
}
