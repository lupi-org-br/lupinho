# Emscripten Compiler
CC = emcc

# Native Compiler (Linux)
CC_NATIVE = gcc

# Source Files
SRC = main.c ui.c lua_api.c

# Output File
OUTPUT = ../dist/game.html

# Raylib Directories
RAYLIB_SRC = libs/web/raylib-web
RAYLIB_INCLUDE = -Ilibs/web/raylib-web
RAYLIB_LIB = libs/web/raylib-web/libraylib.web.a

# Lua Configuration
LUA_WEB_INCLUDE = -Ilibs/web/lua-web
LUA_WEB_LIB = libs/web/lua-web/liblua.web.a

# Lua 5.5.0 (compiled with binary literal support)
LUA_55_INCLUDE = -Ilibs/linux/lua-5.5.0/src
LUA_55_LIB = libs/linux/lua-5.5.0/src/liblua.a

# Native (Linux) Libraries
NATIVE_LIBS = -lraylib $(LUA_55_LIB) -lm -ldl -lpthread -lX11 -larchive
NATIVE_INCLUDE = $(LUA_55_INCLUDE)

# Native (Linux) Static Libraries
RAYLIB_LINUX_STATIC = libs/linux/raylib/src/libraylib.a
STATIC_LIBS = $(RAYLIB_LINUX_STATIC) $(LUA_55_LIB)
STATIC_LIBS_FLAGS = -Wl,-Bstatic $(RAYLIB_LINUX_STATIC) $(LUA_55_LIB) -Wl,-Bdynamic -lm -ldl -lpthread -lX11 -larchive

# Native (Linux) Source Files
SRC_NATIVE = main.c ui.c lua_api.c zip.c

# Compilation Flags
CFLAGS = -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces

# Emscripten Web Flags
EMFLAGS = -s USE_GLFW=3
# Note: ASYNCIFY removed - not needed and causes ~50% performance overhead
EMFLAGS += -s TOTAL_MEMORY=67108864
EMFLAGS += -s STACK_SIZE=1048576
EMFLAGS += -s FORCE_FILESYSTEM=1
EMFLAGS += --shell-file libs/web/raylib-web/minshell.html
EMFLAGS += -DPLATFORM_WEB

EMFLAGS += -s EXPORTED_FUNCTIONS='["_main", "_lupi_load_game", "_malloc", "_free"]'
EMFLAGS += -s EXPORTED_RUNTIME_METHODS='["ccall", "FS"]'

# Optimization
OPTIMIZATION = -O2

# Debug/Production Flags
# Production flags without closure compiler (causes issues with Raylib)
DEBUG_FLAGS = -DDEBUG_MODE
PROD_FLAGS = -O3 -s ASSERTIONS=0 -s DISABLE_EXCEPTION_CATCHING=1 -s ELIMINATE_DUPLICATE_FUNCTIONS=1 -DPRODUCTION

# Web Target (development with debug)
web: $(SRC)
	$(CC) $(SRC) -o $(OUTPUT) $(RAYLIB_INCLUDE) $(LUA_WEB_INCLUDE) $(CFLAGS) $(EMFLAGS) $(OPTIMIZATION) $(DEBUG_FLAGS) $(LUA_WEB_LIB) $(RAYLIB_LIB)

# Production Target (optimized, no debug)
production: $(SRC)
	$(CC) $(SRC) -o $(OUTPUT) $(RAYLIB_INCLUDE) $(LUA_WEB_INCLUDE) $(CFLAGS) $(EMFLAGS) $(PROD_FLAGS) $(LUA_WEB_LIB) $(RAYLIB_LIB)

# Web Game Target (embed .lupi file)
web-game: $(SRC)
	@if [ -z "$(GAME_PATH)" ]; then echo "Error: GAME_PATH is required. Usage: make web-game GAME_PATH=game.lupi"; exit 1; fi
	@echo "Extracting $(GAME_PATH)..."
	@rm -rf /tmp/lupi-web-game
	@mkdir -p /tmp/lupi-web-game
	@unzip -q "$(GAME_PATH)" -d /tmp/lupi-web-game -x "*.git*" "*.lupi"
	@echo "Building with embedded game..."
	@mkdir -p ../dist/webgame
	$(CC) $(SRC) -o ../dist/webgame/game.html $(RAYLIB_INCLUDE) $(LUA_WEB_INCLUDE) $(CFLAGS) $(EMFLAGS) $(OPTIMIZATION) $(DEBUG_FLAGS) -DAUTO_LOAD_GAME $(LUA_WEB_LIB) $(RAYLIB_LIB) --preload-file /tmp/lupi-web-game@/loaded_game
	@echo "Build complete! Output in ../dist/webgame/"

# Linux Target (native desktop)
linux: $(SRC_NATIVE)
	$(CC_NATIVE) $(SRC_NATIVE) -o ../dist/lupinho $(NATIVE_INCLUDE) $(CFLAGS) $(NATIVE_LIBS) $(OPTIMIZATION) $(DEBUG_FLAGS)

# Linux Static Target (portable binary with static raylib)
# Note: Still requires libarchive.so.13 on target system
static: $(SRC_NATIVE) $(RAYLIB_LINUX_STATIC)
	$(CC_NATIVE) $(SRC_NATIVE) -o ../dist/lupinho-static $(NATIVE_INCLUDE) $(CFLAGS) $(STATIC_LIBS_FLAGS) $(OPTIMIZATION) $(DEBUG_FLAGS) -s

# Clean generated files
clean:
	rm -rf ../dist/webgame/
	rm -f $(OUTPUT) game.js game.wasm lupi_emulator ../dist/lupinho ../dist/lupinho-static

# Help
help:
	@echo "Lupi Emulator Makefile"
	@echo "======================"
	@echo ""
	@echo "Targets:"
	@echo "  make web         - Build for WebAssembly (development mode with debug)"
	@echo "  make production  - Build for WebAssembly (optimized, no debug)"
	@echo "  make web-game GAME_PATH=game.lupi - Build for Web with embedded .lupi game (output: dist/webgame/)"
	@echo "  make linux       - Build for Linux desktop (requires raylib, libarchive)"
	@echo "  make static      - Build portable Linux binary (static raylib + Lua)"
	@echo "  make clean       - Remove generated files"
	@echo ""
	@echo "Dependencies:"
	@echo "  make web         - Emscripten SDK"
	@echo "  make linux       - pacman: raylib libarchive"
	@echo "  make static      - pacman: libarchive (raylib statically linked)"
	@echo ""
	@echo "Note: For WebAssembly builds, Lua must be compiled with Emscripten."
	@echo "      If you get linking errors, you may need to compile Lua with emcc."

.PHONY: web production linux static clean help
