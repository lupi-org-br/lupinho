# Emscripten Compiler
CC = emcc

# Native Compiler (Linux)
CC_NATIVE = gcc

# Source Files
SRC = main.c ui.c lua_api.c lupi_input.c

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
SRC_NATIVE = main.c ui.c lua_api.c zip.c lupi_input.c

# Libretro core
LIBRETRO_SRC = ui.c lua_api.c zip.c lupi_input.c libretro/libretro.c
LUA_LIBRETRO_SRC = \
	libs/linux/lua-5.5.0/src/lapi.c \
	libs/linux/lua-5.5.0/src/lcode.c \
	libs/linux/lua-5.5.0/src/lctype.c \
	libs/linux/lua-5.5.0/src/ldebug.c \
	libs/linux/lua-5.5.0/src/ldo.c \
	libs/linux/lua-5.5.0/src/ldump.c \
	libs/linux/lua-5.5.0/src/lfunc.c \
	libs/linux/lua-5.5.0/src/lgc.c \
	libs/linux/lua-5.5.0/src/llex.c \
	libs/linux/lua-5.5.0/src/lmem.c \
	libs/linux/lua-5.5.0/src/lobject.c \
	libs/linux/lua-5.5.0/src/lopcodes.c \
	libs/linux/lua-5.5.0/src/lparser.c \
	libs/linux/lua-5.5.0/src/lstate.c \
	libs/linux/lua-5.5.0/src/lstring.c \
	libs/linux/lua-5.5.0/src/ltable.c \
	libs/linux/lua-5.5.0/src/ltm.c \
	libs/linux/lua-5.5.0/src/lundump.c \
	libs/linux/lua-5.5.0/src/lvm.c \
	libs/linux/lua-5.5.0/src/lzio.c \
	libs/linux/lua-5.5.0/src/lauxlib.c \
	libs/linux/lua-5.5.0/src/lbaselib.c \
	libs/linux/lua-5.5.0/src/lcorolib.c \
	libs/linux/lua-5.5.0/src/ldblib.c \
	libs/linux/lua-5.5.0/src/liolib.c \
	libs/linux/lua-5.5.0/src/lmathlib.c \
	libs/linux/lua-5.5.0/src/loadlib.c \
	libs/linux/lua-5.5.0/src/loslib.c \
	libs/linux/lua-5.5.0/src/lstrlib.c \
	libs/linux/lua-5.5.0/src/ltablib.c \
	libs/linux/lua-5.5.0/src/lutf8lib.c \
	libs/linux/lua-5.5.0/src/linit.c

# Host OS: native libretro core (.so / .dylib / .dll)
ifeq ($(OS),Windows_NT)
	LIBRETRO_EXT = dll
	LIBRETRO_SOFLAGS = -shared
	LIBRETRO_LIBS = -lz -lm
	LIBRETRO_LUAFLAGS =
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LIBRETRO_EXT = dylib
		LIBRETRO_SOFLAGS = -dynamiclib
		LIBRETRO_LIBS = -lz -lm
		LIBRETRO_LUAFLAGS = -DLUA_USE_POSIX
	else
		LIBRETRO_EXT = so
		LIBRETRO_SOFLAGS = -shared
		LIBRETRO_LIBS = -lz -lm -ldl
		LIBRETRO_LUAFLAGS = -DLUA_USE_POSIX
	endif
endif

LIBARCHIVE_CFLAGS := $(shell pkg-config --cflags libarchive 2>/dev/null)
LIBARCHIVE_LIBS := $(shell pkg-config --libs libarchive 2>/dev/null)
ifeq ($(LIBARCHIVE_LIBS),)
	LIBARCHIVE_LIBS = -larchive
endif

LIBRETRO_OUTPUT = ../dist/lupinho_libretro.$(LIBRETRO_EXT)

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
	$(CC) $(SRC) -o $(OUTPUT) $(RAYLIB_INCLUDE) $(LUA_WEB_INCLUDE) $(CFLAGS) $(EMFLAGS) $(OPTIMIZATION) $(LUA_WEB_LIB) $(RAYLIB_LIB)

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
	$(CC) $(SRC) -o ../dist/webgame/game.html $(RAYLIB_INCLUDE) $(LUA_WEB_INCLUDE) $(CFLAGS) $(EMFLAGS) $(OPTIMIZATION) -DAUTO_LOAD_GAME $(LUA_WEB_LIB) $(RAYLIB_LIB) --preload-file /tmp/lupi-web-game@/loaded_game
	@echo "Build complete! Output in ../dist/webgame/"

# Linux Target (native desktop)
linux: $(SRC_NATIVE)
	$(CC_NATIVE) $(SRC_NATIVE) -o ../dist/lupinho $(NATIVE_INCLUDE) $(CFLAGS) $(NATIVE_LIBS) $(OPTIMIZATION)

# macOS Target (native desktop; compiles Lua for Darwin, does not overwrite Linux liblua.a)
macos: $(SRC_NATIVE) $(LUA_55_SRC)
	@mkdir -p ../dist
	$(CC_NATIVE) $(SRC_NATIVE) $(LUA_55_SRC) -o ../dist/lupinho \
		$(MACOS_INCLUDE) $(CFLAGS) -DLUA_USE_POSIX \
		$(MACOS_LIBS) $(OPTIMIZATION)

# Linux Static Target (portable binary with static raylib)
# Note: Still requires libarchive.so.13 on target system
static: $(SRC_NATIVE) $(RAYLIB_LINUX_STATIC)
	$(CC_NATIVE) $(SRC_NATIVE) -o ../dist/lupinho-static $(NATIVE_INCLUDE) $(CFLAGS) $(STATIC_LIBS_FLAGS) $(OPTIMIZATION) -s

# Libretro core (RetroArch). Output: dist/lupinho_libretro.so (or .dylib on macOS)
libretro: $(LIBRETRO_SRC) $(LUA_LIBRETRO_SRC)
	@mkdir -p ../dist
	$(CC_NATIVE) -fPIC $(LIBRETRO_SOFLAGS) -o $(LIBRETRO_OUTPUT) \
		$(LIBRETRO_SRC) $(LUA_LIBRETRO_SRC) \
		-I. -Ilibretro $(LUA_55_INCLUDE) \
		$(CFLAGS) -DLIBRETRO $(LIBRETRO_LUAFLAGS) $(OPTIMIZATION) \
		$(LIBRETRO_LIBS)
	cp -f libretro/lupinho_libretro.info ../dist/lupinho_libretro.info

# Cross-compile Libretro core for Windows (MinGW-w64, POSIX host).
# Output: dist/lupinho_libretro.dll
MINGW_CC ?= x86_64-w64-mingw32-gcc
libretro-win: $(LIBRETRO_SRC) $(LUA_LIBRETRO_SRC)
	@mkdir -p ../dist
	$(MINGW_CC) -shared -o ../dist/lupinho_libretro.dll \
		$(LIBRETRO_SRC) $(LUA_LIBRETRO_SRC) \
		-I. -Ilibretro $(LUA_55_INCLUDE) \
		$(CFLAGS) -DLIBRETRO $(OPTIMIZATION) \
		-lz -lm -static-libgcc
	cp -f libretro/lupinho_libretro.info ../dist/lupinho_libretro.info

# Clean generated files
clean:
	rm -rf ../dist/webgame/
	rm -f $(OUTPUT) game.js game.wasm lupi_emulator ../dist/lupinho ../dist/lupinho-static $(LIBRETRO_OUTPUT) ../dist/lupinho_libretro.dll

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
	@echo "  make libretro    - Build Libretro core for this OS: dist/lupinho_libretro.{so,dylib,dll}"
	@echo "  make libretro-win - Cross-compile Libretro core for Windows: dist/lupinho_libretro.dll"
	@echo "  make clean       - Remove generated files"
	@echo ""
	@echo "Dependencies:"
	@echo "  make web         - Emscripten SDK"
	@echo "  make linux       - pacman: raylib libarchive"
	@echo "  make static      - pacman: libarchive (raylib statically linked)"
	@echo "  make libretro    - zlib"
	@echo "  make libretro-win - mingw-w64 gcc + mingw zlib (see help)"
	@echo ""
	@echo "Note: For WebAssembly builds, Lua must be compiled with Emscripten."
	@echo "      If you get linking errors, you may need to compile Lua with emcc."

.PHONY: web production linux static libretro libretro-win clean help
