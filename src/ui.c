#include <stdlib.h>
#include <stdio.h>

#include "ui.h"
#include "font.h"

/*
Global vars
*/
extern const int screenWidth;
extern const int screenHeight;
char frame_buffer[270][480];
Color palette[PALETTE_SIZE];

/*
Fill pattern
*/
uint8_t fill_pattern[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/*
Camera
*/
int camera_x = 0;
int camera_y = 0;

void set_camera(int x, int y) { camera_x = x; camera_y = y; }
void reset_camera(void)       { camera_x = 0; camera_y = 0; }

/*
Clip region
*/
int  clip_x = 0;
int  clip_y = 0;
int  clip_w = 0;
int  clip_h = 0;
bool clip_enabled = false;

void set_clip(int x, int y, int w, int h) {
    clip_x = x; clip_y = y; clip_w = w; clip_h = h;
    clip_enabled = true;
}
void reset_clip(void) { clip_enabled = false; }

/*
Frame buffer pixel write — enforces bounds and clip region.
All drawing functions must use this instead of writing frame_buffer directly.
*/
void fb_set(int x, int y, int color) {
    if (x < 0 || x >= screenWidth || y < 0 || y >= screenHeight) return;
    if (clip_enabled) {
        if (x < clip_x || x >= clip_x + clip_w ||
            y < clip_y || y >= clip_y + clip_h) return;
    }
    frame_buffer[y][x] = (char)color;
}

bool should_draw_pixel_with_pattern(int x, int y, uint8_t pattern[8]) {
    bool is_solid = true;
    for (int i = 0; i < 8; i++) {
        if (pattern[i] != 0) {
            is_solid = false;
            break;
        }
    }
    if (is_solid) return true;

    int row = y & 7;
    int col = x & 7;

    return (pattern[row] & (1 << (7 - col))) != 0;
}

/**
Line Functions
**/
void add_line(int x1, int y1, int x2, int y2, Color color, int color_index) {
    LineItem line = {
        .x1 = x1,
        .y1 = y1,
        .x2 = x2,
        .y2 = y2,
        .color_index = color_index,
    };

    draw_line(&line);
}

void draw_line(LineItem *line) {
    int x1 = line->x1 - camera_x;
    int y1 = line->y1 - camera_y;
    int x2 = line->x2 - camera_x;
    int y2 = line->y2 - camera_y;

    // Bresenham's line algorithm
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    int x = x1;
    int y = y1;

    while(1) {
        fb_set(x, y, line->color_index);

        if(x == x2 && y == y2) break;

        int e2 = 2 * err;
        if(e2 > -dy) { err -= dy; x += sx; }
        if(e2 < dx)  { err += dx; y += sy; }
    }
}

/**
Rect Functions
**/
void add_rect(int x, int y, int width, int height, bool filled, int color_index) {
    RectItem rect = {
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .filled = filled,
        .color_index = color_index,
    };
    memcpy(rect.fill_pattern, fill_pattern, sizeof(fill_pattern));

    draw_rect(&rect);
}

void draw_rect(RectItem *rect) {
    int rx = rect->x - camera_x;
    int ry = rect->y - camera_y;
    int rw = rect->width;
    int rh = rect->height;

    if(rect->filled) {
        bool has_pattern = false;
        for (int i = 0; i < 8; i++) {
            if (rect->fill_pattern[i] != 0) { has_pattern = true; break; }
        }

        for (int py = ry; py < ry + rh; py++) {
            for (int px = rx; px < rx + rw; px++) {
                if (has_pattern) {
                    if (should_draw_pixel_with_pattern(px, py, rect->fill_pattern))
                        fb_set(px, py, rect->color_index);
                } else {
                    fb_set(px, py, rect->color_index);
                }
            }
        }
    } else {
        // Top line
        for (int px = rx; px < rx + rw; px++) fb_set(px, ry, rect->color_index);
        // Bottom line
        for (int px = rx; px < rx + rw; px++) fb_set(px, ry + rh - 1, rect->color_index);
        // Left line
        for (int py = ry; py < ry + rh; py++) fb_set(rx, py, rect->color_index);
        // Right line
        for (int py = ry; py < ry + rh; py++) fb_set(rx + rw - 1, py, rect->color_index);
    }
}

/**
Circle Functions
**/
void add_circle(int center_x, int center_y, int radius, bool filled, int color_index, bool has_border, int border_color_index) {
    CircleItem circle = {
        .center_x = center_x,
        .center_y = center_y,
        .radius = radius,
        .filled = filled,
        .color_index = color_index,
        .has_border = has_border,
        .border_color_index = border_color_index,
    };
    memcpy(circle.fill_pattern, fill_pattern, sizeof(fill_pattern));

    draw_circle(&circle);
}

// Helper function to draw circle pixels using midpoint algorithm
void draw_circle_pixels(int cx, int cy, int x, int y, int color_index) {
    fb_set(cx + x, cy + y, color_index);
    fb_set(cx - x, cy + y, color_index);
    fb_set(cx + x, cy - y, color_index);
    fb_set(cx - x, cy - y, color_index);
    fb_set(cx + y, cy + x, color_index);
    fb_set(cx - y, cy + x, color_index);
    fb_set(cx + y, cy - x, color_index);
    fb_set(cx - y, cy - x, color_index);
}

void draw_circle(CircleItem *circle) {
    int cx = circle->center_x - camera_x;
    int cy = circle->center_y - camera_y;

    if(circle->filled) {
        bool has_pattern = false;
        for (int i = 0; i < 8; i++) {
            if (circle->fill_pattern[i] != 0) { has_pattern = true; break; }
        }

        int radius_squared = circle->radius * circle->radius;
        for (int py = cy - circle->radius; py <= cy + circle->radius; py++) {
            for (int px = cx - circle->radius; px <= cx + circle->radius; px++) {
                int dx = px - cx;
                int dy = py - cy;
                if (dx * dx + dy * dy <= radius_squared) {
                    if (has_pattern) {
                        if (should_draw_pixel_with_pattern(px, py, circle->fill_pattern))
                            fb_set(px, py, circle->color_index);
                    } else {
                        fb_set(px, py, circle->color_index);
                    }
                }
            }
        }
    }

    if(circle->has_border) {
        int x = 0;
        int y = circle->radius;
        int d = 1 - circle->radius;

        draw_circle_pixels(cx, cy, x, y, circle->border_color_index);

        while (x < y) {
            if (d < 0) {
                d = d + 2 * x + 3;
            } else {
                d = d + 2 * (x - y) + 5;
                y--;
            }
            x++;
            draw_circle_pixels(cx, cy, x, y, circle->border_color_index);
        }
    }
}

/**
Triangle Functions
**/
void add_triangle(int p1_x, int p1_y, int p2_x, int p2_y, int p3_x, int p3_y, int color_index) {
    TriangleItem triangle = {
        .p1_x = p1_x,
        .p1_y = p1_y,
        .p2_x = p2_x,
        .p2_y = p2_y,
        .p3_x = p3_x,
        .p3_y = p3_y,
        .color_index = color_index,
    };

    draw_triangle(&triangle);
}

// Helper function to draw a horizontal line for triangle filling
void draw_horizontal_line_fb(int x1, int x2, int y, int color_index) {
    if (x1 > x2) { int tmp = x1; x1 = x2; x2 = tmp; }
    for (int x = x1; x <= x2; x++) fb_set(x, y, color_index);
}

void draw_triangle(TriangleItem *triangle) {
    // Sort vertices by Y coordinate (v1.y <= v2.y <= v3.y)
    int x1 = triangle->p1_x - camera_x, y1 = triangle->p1_y - camera_y;
    int x2 = triangle->p2_x - camera_x, y2 = triangle->p2_y - camera_y;
    int x3 = triangle->p3_x - camera_x, y3 = triangle->p3_y - camera_y;

    // Bubble sort by Y
    if (y1 > y2) {
        int tmp = y1; y1 = y2; y2 = tmp;
        tmp = x1; x1 = x2; x2 = tmp;
    }
    if (y2 > y3) {
        int tmp = y2; y2 = y3; y3 = tmp;
        tmp = x2; x2 = x3; x3 = tmp;
    }
    if (y1 > y2) {
        int tmp = y1; y1 = y2; y2 = tmp;
        tmp = x1; x1 = x2; x2 = tmp;
    }

    // Degenerate triangle (all points on same horizontal line)
    if (y1 == y3) {
        int minX = x1 < x2 ? (x1 < x3 ? x1 : x3) : (x2 < x3 ? x2 : x3);
        int maxX = x1 > x2 ? (x1 > x3 ? x1 : x3) : (x2 > x3 ? x2 : x3);
        draw_horizontal_line_fb(minX, maxX, y1, triangle->color_index);
        return;
    }

    // Draw triangle using scanline algorithm
    for (int y = y1; y <= y3; y++) {
        bool secondHalf = y > y2 || y2 == y1;
        int segmentHeight = secondHalf ? y3 - y2 : y2 - y1;

        if (segmentHeight == 0) continue;

        float alpha = (float)(y - y1) / (float)(y3 - y1);
        float beta = secondHalf ? (float)(y - y2) / (float)(y3 - y2) : (float)(y - y1) / (float)(y2 - y1);

        int xa = x1 + (int)((x3 - x1) * alpha);
        int xb = secondHalf ? x2 + (int)((x3 - x2) * beta) : x1 + (int)((x2 - x1) * beta);

        draw_horizontal_line_fb(xa, xb, y, triangle->color_index);
    }
}

/**
Palette Functions
**/
void palset(int position, int bgr555) {
    if (position < 0 || position >= PALETTE_SIZE) return;

    // Extract BGR555 components (5 bits each)
    int r5 = (bgr555 >> 0) & 0x1F;
    int g5 = (bgr555 >> 5) & 0x1F;
    int b5 = (bgr555 >> 10) & 0x1F;

    // Scale from 5-bit (0-31) to 8-bit (0-255)
    palette[position].r = (r5 << 3) | (r5 >> 2);
    palette[position].g = (g5 << 3) | (g5 >> 2);
    palette[position].b = (b5 << 3) | (b5 >> 2);
    palette[position].a = 255;
}

Color get_palette_color(int index) {
    if (index == 0) {
        return (Color) {0, 0, 0, 0}; // transparent
    }

    return palette[index];
}

/**
Clear Functions
**/
void add_clear(int color_index) {
    ClearItem clear = {
        .color_index = color_index,
    };

    draw_clear(&clear);
}

void draw_clear(ClearItem *clear) {
    // Fill entire frame buffer with the color index
    for (int y = 0; y < screenHeight; y++) {
        memset(frame_buffer[y], clear->color_index, screenWidth);
    }
}

/*
* Print (bitmap font) Functions
*/
void draw_print(const char *text, int x, int y, int color_index) {
    int cursor_x = x - camera_x;
    int base_y   = y - camera_y;

    for (int i = 0; text[i] != '\0'; i++) {
        unsigned char c = (unsigned char)text[i];

        if (c < 32 || c > 126) continue;

        const uint8_t *glyph = font_data[c - 32];

        for (int col = 0; col < FONT_CHAR_WIDTH; col++) {
            for (int row = 0; row < FONT_CHAR_HEIGHT; row++) {
                if (glyph[col] & (1 << row))
                    fb_set(cursor_x + col, base_y + row, color_index);
            }
        }

        cursor_x += FONT_CHAR_ADVANCE;
    }
}

/*
* Frame Buffer Functions
*/
Texture scene;

Image generate_image_from_frame_buffer() {
    Image image = GenImageColor(screenWidth, screenHeight, BLANK);
    Color *pixels = (Color *)image.data;

    for(int i = 0; i < screenHeight; i++) {
        for(int j = 0; j < screenWidth; j++) {
            int pixel_index = j + (screenWidth * i);
            pixels[pixel_index] = get_palette_color(frame_buffer[i][j]);
        }
    }

    return image;
}

void draw_frame_buffer() {
    Image image = generate_image_from_frame_buffer();
    scene = LoadTextureFromImage(image);
 
    Rectangle source = { 0, 0, screenWidth, screenHeight };
    Rectangle dest = { 0, 0, screenWidth, screenHeight };
    Vector2 origin = { 0, 0 };

    DrawTexturePro(scene, source, dest, origin, 0, WHITE);
    UnloadImage(image);
}

void clear_frame_buffer() {
    for(int i = 0; i < screenHeight; i++) {
        memset(frame_buffer[i], 0, screenWidth);
    }

    UnloadTexture(scene);
}

/**
Tile Functions
**/
void draw_tile(const char *path, int width, int height, int tile_index, int x, int y, bool flipped) {
    x -= camera_x;
    y -= camera_y;

    int tile_size = width * height;
    long data_offset = (long)tile_index * tile_size;

    FILE *f = fopen(path, "rb");
    if (!f) {
        printf("Warning: Could not open tile file %s\n", path);
        return;
    }

    fseek(f, data_offset, SEEK_SET);

    unsigned char *data = (unsigned char *) malloc(tile_size);
    if (!data) { fclose(f); return; }

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
}

/**
Sprite Functions
**/
void draw_spr(const char *path, int width, int height, int x, int y, bool flipped) {
    x -= camera_x;
    y -= camera_y;

    FILE *f = fopen(path, "rb");
    if (!f) {
        printf("Warning: Could not open sprite file %s\n", path);
        return;
    }

    int data_size = width * height;
    unsigned char *data = (unsigned char *) malloc(data_size);
    if (!data) { fclose(f); return; }

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
}

/**
Fill Pattern Functions
**/
void set_fillp(uint8_t *bytes, int n) {
    for (int i = 0; i < n && i < 8; i++) fill_pattern[i] = bytes[i];
    for (int i = n; i < 8; i++)          fill_pattern[i] = 0;
}

/**
Map Functions
**/
void draw_map_layer(MapLayerData *data, int map_width, int tile_size, int cam_x, int cam_y) {
    FILE *f = fopen(data->path, "rb");
    if (!f) {
        printf("Warning: Could not open tileset file %s\n", data->path);
        return;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char *file_buf = (unsigned char *)malloc(file_size);
    if (!file_buf) { fclose(f); return; }

    fread(file_buf, 1, file_size, f);
    fclose(f);

    int tile_pixels = data->tile_w * data->tile_h;

    for (int map_idx = 0; map_idx < data->count; map_idx++) {
        if (data->tile_ids[map_idx] < 0) continue;

        int tile_id_flags = data->tile_ids[map_idx];
        bool flip_x = (tile_id_flags & 1024) != 0;
        bool flip_y = (tile_id_flags & 2048) != 0;
        int tile_id = tile_id_flags & ~(1024 | 2048);

        long offset = (long)tile_id * tile_pixels;
        if (offset < 0 || offset + tile_pixels > file_size) continue;

        int col = map_idx % map_width;
        int row = map_idx / map_width;
        int sx  = col * tile_size + cam_x;
        int sy  = row * tile_size + cam_y;

        unsigned char *tile_data = file_buf + offset;
        for (int ty = 0; ty < data->tile_h; ty++) {
            for (int tx = 0; tx < data->tile_w; tx++) {
                unsigned char idx = tile_data[ty * data->tile_w + tx];
                if (idx == 0) continue;
                int px = flip_x ? (sx + data->tile_w - 1 - tx) : (sx + tx);
                int py = flip_y ? (sy + data->tile_h - 1 - ty) : (sy + ty);
                fb_set(px, py, idx);
            }
        }
    }

    free(file_buf);
}


