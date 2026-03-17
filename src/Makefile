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

# Linux Target (native desktop)
linux: $(SRC_NATIVE)
	$(CC_NATIVE) $(SRC_NATIVE) -o ../dist/lupinho $(NATIVE_INCLUDE) $(CFLAGS) $(NATIVE_LIBS) $(OPTIMIZATION) $(DEBUG_FLAGS)

# Clean generated files
clean:
	rm -f $(OUTPUT) game.js game.wasm lupi_emulator ../dist/lupinho

# Help
help:
	@echo "Lupi Emulator Makefile"
	@echo "======================"
	@echo ""
	@echo "Targets:"
	@echo "  make web         - Build for WebAssembly (development mode with debug)"
	@echo "  make production  - Build for WebAssembly (optimized, no debug)"
	@echo "  make linux       - Build for Linux desktop (native executable)"
	@echo "  make clean       - Remove generated files"
	@echo ""
	@echo "Note: For WebAssembly builds, Lua must be compiled with Emscripten."
	@echo "      If you get linking errors, you may need to compile Lua with emcc."

.PHONY: web production linux native clean help
