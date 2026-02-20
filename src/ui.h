#ifndef UI_H
#define UI_H

#include <lua.h>

#include "types.h"

/*
Draw Functions
*/
void add_line(int x1, int y1, int x2, int y2, Color color, int color_index);
void draw_line(LineItem *line);

void add_rect(int x, int y, int width, int height, bool filled, int color_index);
void draw_rect(RectItem *rect);

void add_circle(int center_x, int center_y, int radius, bool filled, int color_index, bool has_border, int border_color_index);
void draw_circle(CircleItem *circle);

void add_clear(int color_index);
void draw_clear(ClearItem *clear);

void add_triangle(int p1_x, int p1_y, int p2_x, int p2_y, int p3_x, int p3_y, int color_index);
void draw_triangle(TriangleItem *triangle);

/*
Palette Functions
*/
#define PALETTE_SIZE 256
extern Color palette[PALETTE_SIZE];
void palset(int position, int bgr555);
Color get_palette_color(int index);

/*
Fill Pattern Functions
*/
extern uint8_t fill_pattern[8];

/*
Sprites Lua Global
*/
void inject_sprites_global(lua_State *L, const char *manifest_path, const char *game_dir);

/*
Camera Functions
*/
extern int camera_x;
extern int camera_y;
void set_camera(int x, int y);
void reset_camera(void);

/*
Clip Functions
*/
void set_clip(int x, int y, int w, int h);
void reset_clip(void);

/*
Frame Buffer Functions
*/
extern char frame_buffer[270][480];
void fb_set(int x, int y, int color);
void clear_frame_buffer();
void draw_frame_buffer();

/*
Print (bitmap font) Functions
*/
void draw_print(const char *text, int x, int y, int color_index);

/*
Lua Functions
*/
int lua_draw_line(lua_State *L);
int lua_draw_rect(lua_State *L);
int lua_rect(lua_State *L);
int lua_rectfill(lua_State *L);
int lua_draw_circle(lua_State *L);
int lua_circfill(lua_State *L);
int lua_trisfill(lua_State *L);
int lua_palset(lua_State *L);
int lua_tile(lua_State *L);
int lua_spr(lua_State *L);
int lua_btn(lua_State *L);
int lua_btnp(lua_State *L);
int lua_fillp(lua_State *L);
int lua_log(lua_State *L);
int lua_cls(lua_State *L);
int lua_map(lua_State *L);

// TODO
int lua_camera(lua_State *L);
int lua_clip(lua_State *L);
int lua_preload_spritesheet(lua_State *L);
int lua_draw_sprite(lua_State *L);
int lua_print(lua_State *L);
int lua_set_pallet(lua_State *L);
#endif // UI_H
