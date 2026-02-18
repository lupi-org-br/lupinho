#include <stdlib.h>
#include <stdio.h>

#include "drawlist.h"
#include "font.h"

/*
Global vars
*/
extern Drawlist drawlist;
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

/*
Drawable Functions
*/
void draw(NodeDrawable *node) {
    switch(node->type) {
        case 't':
            draw_text((TextItem *) node->drawable);
            break;
        case 's':
            draw_tile((TileItem *) node->drawable);
            break;
        case 'w':
            draw_sprite((SpriteItem *) node->drawable);
            break;
    }
}

void clear_drawlist() {
    NodeDrawable *current = drawlist.root;
    while(current != NULL) {
        NodeDrawable *next = current->next;

        free(current->drawable);
        free(current);
        current = next;
    }
    drawlist.count = 0;
    drawlist.root = NULL;
}

void add_drawable(void *drawable, char type) {
    if(drawlist.count == 0) {
        drawlist.count++;

        NodeDrawable *node = (NodeDrawable *) malloc(sizeof(NodeDrawable));
        drawlist.root = node;
        node->drawable = drawable;
        node->type = type;
        node->next = NULL;
        return;
    }

    drawlist.count++;

    NodeDrawable *current = drawlist.root;
    for(; current->next != NULL; current = current->next) {}

    NodeDrawable *node = (NodeDrawable *) malloc(sizeof(NodeDrawable));
    current->next = node;
    node->next = NULL;
    node->drawable = drawable;
    node->type = type;
}

/**
Text Functions
**/
void add_text(char *text_s, int x, int y) {
    TextItem *text = (TextItem *) malloc(sizeof(TextItem));
    text->text = text_s;
    text->x = x;
    text->y = y;
    text->fontSize = 20;
    text->color = DARKGRAY;

    add_drawable(text, 't');
}

void draw_text(TextItem *text) {
    DrawText(text->text, text->x, text->y, text->fontSize, text->color);
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
Sprite Functions
**/
void add_sprite(SpriteInMemory *sprite_in_memory, int x, int y, bool flipped) {
    SpriteItem *sprite = (SpriteItem *) malloc(sizeof(SpriteItem));
    sprite->sprite_in_memory = sprite_in_memory;
    sprite->x = x;
    sprite->y = y;
    sprite->flipped = flipped;

    add_drawable(sprite, 'w');
}

void draw_sprite(SpriteItem *sprite) {
    int dest_width = sprite->sprite_in_memory->tile_width;
    int dest_height = sprite->sprite_in_memory->tile_height;
    int src_width = dest_width;

    if (sprite->flipped) {
        src_width = -src_width;
    }

    DrawTexturePro(
        sprite->sprite_in_memory->texture,
        (Rectangle) { 0, 0, src_width, sprite->sprite_in_memory->tile_height },
        (Rectangle) { sprite->x, sprite->y, dest_width, dest_height },
        (Vector2) { 0, 0 },
        0,
        WHITE
    );
}

/**
Tile Functions
**/
void add_tile(SpriteInMemory *sprite_in_memory, int tile_index, int x, int y, bool flipped) {
    TileItem *tile = (TileItem *) malloc(sizeof(TileItem));
    tile->sprite_in_memory = sprite_in_memory;
    tile->tile_index = tile_index;
    tile->x = x;
    tile->y = y;
    tile->flipped = flipped;

    add_drawable(tile, 's');
}

void draw_tile(TileItem *tile) {
    int src_x = tile->tile_index * tile->sprite_in_memory->tile_width;
    int src_y = 0;
    int src_width = tile->sprite_in_memory->tile_width;

    if (tile->flipped) {
        src_width = -src_width;
    }

    DrawTexturePro(
        tile->sprite_in_memory->texture,
        (Rectangle) { src_x, src_y, src_width, tile->sprite_in_memory->tile_height },
        (Rectangle) { tile->x, tile->y, tile->sprite_in_memory->tile_width, tile->sprite_in_memory->tile_height },
        (Vector2) { 0, 0 },
        0,
        WHITE
    );
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

/**
Sprites In Memory Functions
**/
Image sprite_in_memory_create_image_from_data(char *data, SpriteInMemory *sprite) {
    int sprite_width = sprite->tile_width * sprite->ntiles;
    int sprite_height = sprite->tile_height;

    Image image = GenImageColor(sprite_width, sprite_height, BLANK);

    Color *pixels = (Color *)image.data;

    int data_index = 0;
    for(int tile_index = 0; tile_index < sprite->ntiles; tile_index++) {
        for(int y = 0; y < sprite->tile_height; y++) {
            for(int x = 0; x < sprite->tile_width; x++) {
                int pixel_index = (x + (tile_index * sprite->tile_width)) + (sprite_width * y);

                pixels[pixel_index] = get_palette_color((int) data[data_index++]);
            }
        }
    }

    return image;
}

void add_sprite_in_memory(char *name, char *data, int width, int height, int ntiles) {
    SpriteInMemory *sprite = (SpriteInMemory *) malloc(sizeof(SpriteInMemory));
    strcpy(sprite->name, name);
    sprite->tile_width = width;
    sprite->tile_height = height;
    sprite->ntiles = ntiles;

    Image image = sprite_in_memory_create_image_from_data(data, sprite);
    sprite->texture = LoadTextureFromImage(image);
    UnloadImage(image);

    sprites_in_memory.count++;

    if (sprites_in_memory.count >= sprites_in_memory.max_count) {
        sprites_in_memory.max_count *= 2;
        sprites_in_memory.sprites = (SpriteInMemory **) realloc(sprites_in_memory.sprites, sizeof(SpriteInMemory *) * sprites_in_memory.max_count);
    }

    sprites_in_memory.sprites[sprites_in_memory.count - 1] = sprite;

    printf("Sprite %s added to sprites in memory\n", name);
}

SpriteInMemory* get_sprite_in_memory(char *name) {
    for(int i = 0; i < sprites_in_memory.count; i++) {
        if(strcmp(sprites_in_memory.sprites[i]->name, name) == 0) {
            return sprites_in_memory.sprites[i];
        }
    }
    return NULL;
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

