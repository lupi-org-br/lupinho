# 🎮 Lupinho

**Lupinho** is a web-based simulator for **Lupi**, a Brazilian game console. It runs directly in your browser using WebAssembly, allowing you to write and play games using a simple Lua scripting API.

### 🕹️ [Try the Demo](https://lupinho.juneira.com/)

> The demo game is **"balão-gatinho"**. Use the **arrow keys** to move and **Z** as the action button.

## ✨ Features

- 🌐 **Browser-based** — Runs on WebAssembly, no installation required
- 📜 **Lua scripting** — Write your games in simple, easy-to-learn Lua
- 🎨 **2D Graphics** — Draw text, lines, rectangles, circles, and triangles
- 🖼️ **Sprites & Tiles** — Load and draw sprite sheets with flip support
- 🗺️ **Tilemap System** — Draw large maps with camera scrolling
- 🎮 **Gamepad & Keyboard** — Input support for both gamepads and keyboard
- ⚡ **60 FPS** — Smooth gameplay at 60 frames per second
- 🔧 **Raylib powered** — Built on the lightweight Raylib graphics library

## 🛠️ Tech Stack

| Component | Technology |
|-----------|------------|
| Language | C99 |
| Compiler | Emscripten (emcc) |
| Graphics | Raylib |
| Scripting | Lua 5.4 |
| Platform | WebAssembly |
| Screen | 480×270 px @ 60 FPS |

## 📁 Project Structure

```
lupinho/
├── dist/               # Compiled WebAssembly output
│   ├── game.html
│   ├── game.js
│   ├── game.wasm
│   └── game.data
├── src/
│   ├── main.c          # Main entry point
│   ├── lua_api.c       # Lua bindings for ui.* API
│   ├── ui.c/h          # Rendering system & frame buffer
│   ├── types.h         # Draw item type definitions
│   ├── font.h          # Embedded bitmap font
│   ├── Makefile
│   └── libs/
│       ├── lua-web/    # Lua compiled for WebAssembly
│       └── raylib-web/ # Raylib compiled for WebAssembly
├── game-example/       # Example game directory
│   ├── game.lua        # Game entry point
│   ├── palette.lua     # Color palette definition
│   ├── lupi_manifest.txt
│   ├── img/            # Bitmap sprites
│   └── map/            # Tilemap data
└── README.md
```

## 🚀 Getting Started

### Prerequisites

- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) installed and activated

### Building

```bash
cd src

# Development build (with FPS counter)
make web

# Production build (optimized, no debug)
make production
```

### Running

After building, serve the `dist/` folder with any HTTP server:

```bash
# Using Node.js
npx serve ../dist
```

Then open `http://localhost:8080` in your browser.

## 📖 Lua API

Games are written in Lua. Your script should define the `update()` function, which is called every frame (60 times per second).

### `update()`
Called every frame. Update your game logic and draw here.

### Color Palette System

Lupinho uses a palette-based color system with 256 colors in BGR555 format (5 bits per channel, Big Endian).

| Function | Description |
|----------|-------------|
| `ui.palset(index, bgr555)` | Set a palette color at the specified index (0-255) |

### Drawing Functions

All drawing functions use palette indices (0-255) for colors:

| Function | Description |
|----------|-------------|
| `ui.rect(x1, y1, x2, y2, color)` | Draw rectangle outline |
| `ui.rectfill(x1, y1, x2, y2, color)` | Draw filled rectangle |
| `ui.draw_rect(x, y, w, h, filled, color)` | Draw rectangle (filled or outline) |
| `ui.circfill(x, y, radius, color)` | Draw filled circle |
| `ui.draw_circle(cx, cy, r, filled, color, border, border_color)` | Draw circle with optional border |
| `ui.trisfill(x1, y1, x2, y2, x3, y3, color)` | Draw filled triangle |
| `ui.line(x1, y1, x2, y2, color)` | Draw a line |
| `ui.print(text, x, y, color)` | Draw text using bitmap font |

### Screen Functions

| Function | Description |
|----------|-------------|
| `ui.cls(color)` | Clear screen with palette color |
| `ui.camera(x, y)` | Set camera offset; `ui.camera()` resets |
| `ui.clip(x, y, w, h)` | Set clipping region; `ui.clip()` resets |
| `ui.fillp(b1, b2, ...)` | Set 8x8 fill pattern (1 byte per row) |

### Sprites & Tiles

| Function | Description |
|----------|-------------|
| `ui.spr(sprite_table, x, y, flipped)` | Draw a sprite |
| `ui.tile(tileset_table, tile_index, x, y)` | Draw a tile (index bit 10 flips horizontally) |

### Map Functions

| Function | Description |
|----------|-------------|
| `ui.map(layer_table, cam_x, cam_y)` | Draw a tilemap layer |

### Input Functions

| Function | Description |
|----------|-------------|
| `ui.btn(button, pad)` | Check if button is held (gamepad or keyboard) |
| `ui.btnp(button, pad)` | Check if button was just pressed |

Button constants: `UP`, `DOWN`, `LEFT`, `RIGHT`, `BTN_Z`, `BTN_Q`, `BTN_E`, `BTN_F`, `BTN_G`

### Utility Functions

| Function | Description |
|----------|-------------|
| `ui.log(message)` | Print to console |
| `ui.mid(a, b, c)` | Return middle value of three numbers |

### Example Game

```lua
-- Define your palette
Palette = {
    0x0000, 0x1516, 0x25B4, 0x20A6, 0x1DFD, 0x46FE, 0x7FFF, 0x2532
}

-- Set palette colors
for i = 1, #Palette do
    ui.palset(i - 1, Palette[i])
end

-- Game variables
x = 200
t = 0

function update()
    t = t + 0.05
    y = math.sin(t) * 25

    ui.print("Bem-vindo ao Lupi!", 280, 180 + math.floor(y))
    ui.rect(50, 50, 130, 130, 1)
    ui.circfill(200, 100, 20, 3)
    ui.trisfill(20, 250, 100, 250, 55, 350, 4)
    
    -- Input example
    if ui.btnp(BTN_Z) then
        ui.log("Button Z pressed!")
    end
end
```

## 🧹 Cleanup

```bash
make clean
```

## 🤝 Contributing

We encourage contributions! However, **please open an issue before submitting a PR** so we can discuss the changes. Issues can be written in **Portuguese, Spanish, or English**.

We don't have an official communication channel yet, but this README will be updated once we create one.

## 📝 License

This project is open source.

---

<p align="center">
  Made with ❤️ in Brazil 🇧🇷
</p>
