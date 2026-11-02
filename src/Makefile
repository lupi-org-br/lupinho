# Emscripten Compiler
CC = emcc

# Native Compiler (Linux / macOS)
CC_NATIVE = gcc

# MinGW (native gcc on Windows, cross-compiler elsewhere)
ifeq ($(OS),Windows_NT)
CC_MINGW ?= gcc
AR_MINGW ?= ar
else
CC_MINGW ?= x86_64-w64-mingw32-gcc
AR_MINGW ?= x86_64-w64-mingw32-ar
endif

# osxcross on Linux (`o64-clang` / `oa64-clang`); Apple clang on Darwin
ifeq ($(shell uname -s),Darwin)
CC_OSX_X86 ?= clang -arch x86_64
CC_OSX_ARM ?= clang -arch arm64
else
CC_OSX_X86 ?= o64-clang
CC_OSX_ARM ?= oa64-clang
endif

# Source Files
SRC = main.c ui.c lua_api.c music.c

# Output File
OUTPUT = ../dist/game.html

# Raylib Directories
RAYLIB_SRC = libs/web/raylib-web
RAYLIB_INCLUDE = -Ilibs/web/raylib-web
RAYLIB_WIN_DIR = libs/windows
RAYLIB_LINUX_DIR = libs/linux/raylib
RAYLIB_MACOS_DIR = libs/macos
RAYLIB_MACOS_ARCH ?= x86_64
RAYLIB_MACOS_ARCH_DIR = $(RAYLIB_MACOS_DIR)/$(RAYLIB_MACOS_ARCH)
RAYLIB_MODULES = rcore rshapes rtextures rtext utils rglfw rmodels raudio
RAYLIB_LINUX_OBJS = $(addprefix $(RAYLIB_LINUX_DIR)/,$(addsuffix .o,$(RAYLIB_MODULES)))
RAYLIB_WIN_OBJS = $(addprefix $(RAYLIB_WIN_DIR)/,$(addsuffix .o,$(RAYLIB_MODULES)))
RAYLIB_MACOS_OBJS = $(addprefix $(RAYLIB_MACOS_ARCH_DIR)/,$(addsuffix .o,$(RAYLIB_MODULES)))
RAYLIB_WEB_OBJDIR = libs/web/raylib-build
RAYLIB_WEB_MODULES = rcore rshapes rtextures rtext utils rmodels raudio
RAYLIB_WEB_OBJS = $(addprefix $(RAYLIB_WEB_OBJDIR)/,$(addsuffix .o,$(RAYLIB_WEB_MODULES)))
RAYLIB_WEB_CFLAGS = -std=gnu99 -D_GNU_SOURCE -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 \
	-Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing -Os \
	-I$(RAYLIB_SRC) -I$(RAYLIB_SRC)/external/glfw/include
RAYLIB_CFLAGS = -std=c99 -D_GNU_SOURCE -DPLATFORM_DESKTOP_GLFW -DGRAPHICS_API_OPENGL_33 \
	-Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing -O1 \
	-I$(RAYLIB_SRC) -I$(RAYLIB_SRC)/external/glfw/include

# Lua 5.5.0 (compiled from source on each target)
LUA_55_INCLUDE = -Ilibs/linux/lua-5.5.0/src
LUA_55_DIR = libs/linux/lua-5.5.0/src
# lua.c / luac.c are the standalone interpreter and compiler, not the library.
LUA_55_SRC = $(filter-out $(LUA_55_DIR)/lua.c $(LUA_55_DIR)/luac.c,$(wildcard $(LUA_55_DIR)/*.c))

# Native (Linux) Libraries — raylib objects built from vendored sources
NATIVE_INCLUDE = $(LUA_55_INCLUDE) $(RAYLIB_INCLUDE)
NATIVE_LIBS = $(RAYLIB_LINUX_OBJS) -lGL -lm -lpthread -ldl -lrt -lX11 -lXcursor -lXinerama -lXrandr -lXi -lz

# Native (Linux / macOS / Windows) Source Files
SRC_NATIVE = main.c ui.c lua_api.c zip.c music.c

# Furnace (.fur) playback. Sources live outside this repo; set FURNACE_SRC to override.
FURNACE_SRC ?= $(abspath ../../furnace)
FURNACE_TAG ?= host
FURNACE_BUILD = music/furnace-build-$(FURNACE_TAG)
FURNACE_LIB = $(FURNACE_BUILD)/libfurengine.a
FUR_CXX ?= c++
FUR_CXXFLAGS = -std=c++14 -DHAVE_SNDFILE \
	-include cstring -include climits \
	-I$(FURNACE_SRC)/src -I$(FURNACE_SRC)/src/engine \
	-I$(FURNACE_SRC)/extern/fmt/include \
	-I$(FURNACE_SRC)/extern/blip_buf \
	-I$(FURNACE_SRC)/extern/libsndfile-modified/include
FUR_PLAYER_OBJ = music/fur_player-$(FURNACE_TAG).o
ifeq ($(shell uname -s),Darwin)
FUR_STDLIB = -lc++
FUR_WHOLE = -Wl,-force_load,$(FURNACE_LIB)
else
FUR_STDLIB = -lstdc++
FUR_WHOLE = -Wl,--whole-archive $(FURNACE_LIB) -Wl,--no-whole-archive
endif
FUR_EXTRA_LIBS = $(FUR_WHOLE) $(FUR_STDLIB) `cat $(FURNACE_BUILD)/furengine-libs.txt 2>/dev/null`

ifeq ($(shell uname -s)-$(shell uname -m),Darwin-arm64)
FURNACE_TAG_ARM = host
else
FURNACE_TAG_ARM = arm64
endif

ifeq ($(shell uname -s),Darwin)
FURNACE_OSX_CMAKE_X86 = -DCMAKE_OSX_ARCHITECTURES=x86_64
FURNACE_OSX_CMAKE_ARM =
else
FURNACE_OSX_CMAKE_X86 = -DCMAKE_SYSTEM_NAME=Darwin -DCMAKE_SYSTEM_PROCESSOR=x86_64 \
	-DCMAKE_C_COMPILER=$(CC_OSX_X86) -DCMAKE_CXX_COMPILER=$(subst clang,clang++,$(CC_OSX_X86)) \
	-DCMAKE_OSX_SYSROOT=/osxcross/SDK/MacOSX.sdk \
	-DCMAKE_AR=/osxcross/bin/apple-ar -DCMAKE_RANLIB=/osxcross/bin/apple-ranlib \
	-DWITH_RENDER_METAL=OFF -DWITH_RENDER_OPENGL=OFF -DWITH_RENDER_SDL=OFF \
	-DSYSTEM_ZLIB=OFF
FURNACE_OSX_CMAKE_ARM = -DCMAKE_SYSTEM_NAME=Darwin -DCMAKE_SYSTEM_PROCESSOR=arm64 \
	-DCMAKE_C_COMPILER=$(CC_OSX_ARM) -DCMAKE_CXX_COMPILER=$(subst clang,clang++,$(CC_OSX_ARM)) \
	-DCMAKE_OSX_SYSROOT=/osxcross/SDK/MacOSX.sdk \
	-DCMAKE_AR=/osxcross/bin/apple-ar -DCMAKE_RANLIB=/osxcross/bin/apple-ranlib \
	-DWITH_RENDER_METAL=OFF -DWITH_RENDER_OPENGL=OFF -DWITH_RENDER_SDL=OFF \
	-DSYSTEM_ZLIB=OFF
endif

# Tag is the path stem. Target-specific FURNACE_TAG is applied too late to name the file.
music/furnace-build-%/libfurengine.a:
	sh music/build_furnace.sh $* "$(FURNACE_SRC)" $(FURNACE_CMAKE_EXTRA)

music/fur_player-%.o: music/fur_player.cpp music/fur_player.h
	$(FUR_CXX) -c music/fur_player.cpp -o $@ $(FUR_CXXFLAGS) $(OPTIMIZATION)

# macOS (osxcross `o64-clang` or native clang)
MACOS_INCLUDE = $(LUA_55_INCLUDE) $(RAYLIB_INCLUDE)
MACOS_LIBS = $(RAYLIB_MACOS_OBJS) -lz -lm \
	-framework OpenGL -framework Cocoa -framework IOKit \
	-framework CoreAudio -framework CoreVideo
CC_OSX ?= $(CC_OSX_X86)

# Windows MinGW
WIN_INCLUDE = $(LUA_55_INCLUDE) $(RAYLIB_INCLUDE)
WIN_LIBS = $(RAYLIB_WIN_OBJS) -lopengl32 -lgdi32 -lwinmm -lz -pthread -static-libgcc
WIN_CFLAGS = $(CFLAGS) -D_WIN32_WINNT=0x0601

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

# Web (Lua 5.5.0 + vendored raylib, compiled with emcc)
web production web-game: FURNACE_TAG=web
web production web-game: FUR_CXX=em++
web production web-game: FUR_WHOLE=-Wl,--whole-archive $(FURNACE_LIB) -Wl,--no-whole-archive
web production web-game: FUR_STDLIB=

# C sources stay on emcc. em++ rejects -std=c99, and the web target used to
# override CC for every prerequisite, including these Raylib objects.
WEB_C_SRC = $(SRC) $(LUA_55_SRC)
WEB_C_OBJS_web = $(patsubst %.c,libs/web/c-build/web/%.o,$(WEB_C_SRC))
WEB_C_OBJS_production = $(patsubst %.c,libs/web/c-build/production/%.o,$(WEB_C_SRC))
WEB_C_OBJS_webgame = $(patsubst %.c,libs/web/c-build/webgame/%.o,$(WEB_C_SRC))
WEB_C_COMPILE = emcc -c $< -o $@ $(LUA_55_INCLUDE) $(RAYLIB_INCLUDE) $(CFLAGS) -DPLATFORM_WEB

libs/web/c-build/web/%.o: %.c
	@mkdir -p $(dir $@)
	$(WEB_C_COMPILE) -DDEBUG_MODE $(OPTIMIZATION)

libs/web/c-build/production/%.o: %.c
	@mkdir -p $(dir $@)
	$(WEB_C_COMPILE) -DPRODUCTION -O3

libs/web/c-build/webgame/%.o: %.c
	@mkdir -p $(dir $@)
	$(WEB_C_COMPILE) -DAUTO_LOAD_GAME $(OPTIMIZATION)

# Web (Lua 5.5.0 + vendored raylib, compiled with emcc)
$(RAYLIB_WEB_OBJDIR)/%.o: $(RAYLIB_SRC)/%.c
	@mkdir -p $(RAYLIB_WEB_OBJDIR)
	$(CC) -c $< -o $@ $(RAYLIB_WEB_CFLAGS)

# Web Target (development with debug). em++ links the C++ Furnace player.
web: $(WEB_C_OBJS_web) $(RAYLIB_WEB_OBJS) music/furnace-build-web/libfurengine.a music/fur_player-web.o
	@mkdir -p ../dist
	em++ $(WEB_C_OBJS_web) $(RAYLIB_WEB_OBJS) $(FUR_PLAYER_OBJ) -o $(OUTPUT) \
		$(EMFLAGS) $(OPTIMIZATION) $(DEBUG_FLAGS) \
		$(FUR_EXTRA_LIBS) -fexceptions -s DISABLE_EXCEPTION_CATCHING=0

# Production Target (optimized, no debug)
production: $(WEB_C_OBJS_production) $(RAYLIB_WEB_OBJS) music/furnace-build-web/libfurengine.a music/fur_player-web.o
	@mkdir -p ../dist
	em++ $(WEB_C_OBJS_production) $(RAYLIB_WEB_OBJS) $(FUR_PLAYER_OBJ) -o $(OUTPUT) \
		$(EMFLAGS) $(PROD_FLAGS) \
		$(FUR_EXTRA_LIBS) -fexceptions -s DISABLE_EXCEPTION_CATCHING=0

# Web Game Target (embed .lupi file)
web-game: $(WEB_C_OBJS_webgame) $(RAYLIB_WEB_OBJS) music/furnace-build-web/libfurengine.a music/fur_player-web.o
	@if [ -z "$(GAME_PATH)" ]; then echo "Error: GAME_PATH is required. Usage: make web-game GAME_PATH=game.lupi"; exit 1; fi
	@echo "Extracting $(GAME_PATH)..."
	@rm -rf /tmp/lupi-web-game
	@mkdir -p /tmp/lupi-web-game
	@unzip -q "$(GAME_PATH)" -d /tmp/lupi-web-game -x "*.git*" "*.lupi"
	@echo "Building with embedded game..."
	@mkdir -p ../dist/webgame
	em++ $(WEB_C_OBJS_webgame) $(RAYLIB_WEB_OBJS) $(FUR_PLAYER_OBJ) -o ../dist/webgame/game.html \
		$(EMFLAGS) $(OPTIMIZATION) -DAUTO_LOAD_GAME \
		$(FUR_EXTRA_LIBS) -fexceptions -s DISABLE_EXCEPTION_CATCHING=0 \
		--preload-file /tmp/lupi-web-game@/loaded_game
	@echo "Build complete! Output in ../dist/webgame/"

# Raylib objects for Linux (compiled into a dedicated dir so they stay ELF)
$(RAYLIB_LINUX_DIR)/%.o: $(RAYLIB_SRC)/%.c
	@mkdir -p $(RAYLIB_LINUX_DIR)
	$(CC_NATIVE) -c $< -o $@ $(RAYLIB_CFLAGS) -D_GLFW_X11 -fPIC

$(RAYLIB_LINUX_DIR)/rglfw.o: $(RAYLIB_SRC)/rglfw.c
	@mkdir -p $(RAYLIB_LINUX_DIR)
	$(CC_NATIVE) -c $< -o $@ $(RAYLIB_CFLAGS) -D_GLFW_X11 -fPIC -U_GNU_SOURCE

# Linux Target (native desktop)
linux: FURNACE_TAG=linux
linux: FUR_CXX=g++
linux: FUR_WHOLE=-Wl,--whole-archive $(FURNACE_LIB) -Wl,--no-whole-archive
linux: FUR_STDLIB=-lstdc++
linux: $(SRC_NATIVE) $(LUA_55_SRC) $(RAYLIB_LINUX_OBJS) music/furnace-build-linux/libfurengine.a music/fur_player-linux.o
	@mkdir -p ../dist
	$(CC_NATIVE) $(SRC_NATIVE) $(LUA_55_SRC) $(FUR_PLAYER_OBJ) -o ../dist/lupinho \
		$(NATIVE_INCLUDE) $(CFLAGS) -DLUA_USE_POSIX -DLUA_USE_DLOPEN \
		$(NATIVE_LIBS) $(FUR_EXTRA_LIBS) $(OPTIMIZATION)

# macOS (osxcross o64/oa64-clang, or native clang -arch)
$(RAYLIB_MACOS_ARCH_DIR)/%.o: $(RAYLIB_SRC)/%.c
	@mkdir -p $(RAYLIB_MACOS_ARCH_DIR)
	$(CC_OSX) -c $< -o $@ $(RAYLIB_CFLAGS) -U_GNU_SOURCE

$(RAYLIB_MACOS_ARCH_DIR)/rglfw.o: $(RAYLIB_SRC)/rglfw.c
	@mkdir -p $(RAYLIB_MACOS_ARCH_DIR)
	$(CC_OSX) -x objective-c -c $< -o $@ $(RAYLIB_CFLAGS) -U_GNU_SOURCE

macos-arch: FUR_WHOLE=-Wl,-force_load,$(FURNACE_LIB)
macos-arch: FUR_STDLIB=-lc++
macos-arch: $(SRC_NATIVE) $(LUA_55_SRC) $(RAYLIB_MACOS_OBJS) $(FURNACE_LIB) $(FUR_PLAYER_OBJ)
	@mkdir -p ../dist
	$(CC_OSX) $(SRC_NATIVE) $(LUA_55_SRC) $(FUR_PLAYER_OBJ) -o ../dist/lupinho-$(RAYLIB_MACOS_ARCH) \
		$(MACOS_INCLUDE) $(CFLAGS) -DLUA_USE_MACOSX \
		$(MACOS_LIBS) $(FUR_EXTRA_LIBS) $(OPTIMIZATION)

macos-x86_64:
	$(MAKE) macos-arch RAYLIB_MACOS_ARCH=x86_64 CC_OSX="$(CC_OSX_X86)" \
		FURNACE_TAG=x86_64 FUR_CXX="$(subst clang,clang++,$(CC_OSX_X86))" \
		FURNACE_CMAKE_EXTRA="$(FURNACE_OSX_CMAKE_X86)"

macos-arm64:
	$(MAKE) macos-arch RAYLIB_MACOS_ARCH=arm64 CC_OSX="$(CC_OSX_ARM)" \
		FURNACE_TAG=$(FURNACE_TAG_ARM) FUR_CXX="$(subst clang,clang++,$(CC_OSX_ARM))" \
		FURNACE_CMAKE_EXTRA="$(FURNACE_OSX_CMAKE_ARM)"

macos-universal:
	@mkdir -p ../dist
	@lipo_bin=`command -v llvm-lipo || command -v llvm-lipo-18 || command -v llvm-lipo-19 || command -v lipo || ls /osxcross/bin/*-apple-darwin*-lipo 2>/dev/null | head -1`; \
	if [ -n "$$lipo_bin" ]; then \
		$$lipo_bin -create ../dist/lupinho-x86_64 ../dist/lupinho-arm64 -output ../dist/lupinho; \
		echo "Universal binary -> ../dist/lupinho"; \
	else \
		echo "Skipping universal binary (install llvm-lipo or Xcode lipo)"; \
	fi

macos: macos-x86_64 macos-arm64 macos-universal

# Linux Static Target (portable binary with static raylib)
static: linux
	@cp ../dist/lupinho ../dist/lupinho-static
	strip ../dist/lupinho-static

# Raylib objects for Windows (MinGW COFF, kept out of the Linux object dir)
$(RAYLIB_WIN_DIR)/%.o: $(RAYLIB_SRC)/%.c
	@mkdir -p $(RAYLIB_WIN_DIR)
	$(CC_MINGW) -c $< -o $@ $(RAYLIB_CFLAGS) -DUNICODE -D_WIN32_WINNT=0x0601

$(RAYLIB_WIN_DIR)/rglfw.o: $(RAYLIB_SRC)/rglfw.c
	@mkdir -p $(RAYLIB_WIN_DIR)
	$(CC_MINGW) -c $< -o $@ $(RAYLIB_CFLAGS) -DUNICODE -D_WIN32_WINNT=0x0601 -U_GNU_SOURCE

# Windows Target (MinGW desktop, cross-compile from Linux/macOS or native MinGW)
ifeq ($(OS),Windows_NT)
CXX_MINGW ?= g++
else
CXX_MINGW ?= x86_64-w64-mingw32-g++
endif
windows: FURNACE_TAG=windows
windows: FUR_CXX=$(CXX_MINGW)
windows: FUR_WHOLE=-Wl,--whole-archive $(FURNACE_LIB) -Wl,--no-whole-archive
windows: FUR_STDLIB=-static-libstdc++ -lstdc++
windows: FURNACE_CMAKE_EXTRA=-DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER=$(CC_MINGW) -DCMAKE_CXX_COMPILER=$(CXX_MINGW) -DSYSTEM_ZLIB=OFF
windows: $(SRC_NATIVE) $(LUA_55_SRC) $(RAYLIB_WIN_OBJS) music/furnace-build-windows/libfurengine.a music/fur_player-windows.o
	@mkdir -p ../dist
	$(CC_MINGW) $(SRC_NATIVE) $(LUA_55_SRC) $(FUR_PLAYER_OBJ) -o ../dist/lupinho.exe \
		$(WIN_INCLUDE) $(WIN_CFLAGS) $(OPTIMIZATION) \
		$(WIN_LIBS) $(FUR_EXTRA_LIBS)

# Docker SDK: `make docker.linux`, `make docker.macos`, `make docker.windows`
# Explicit names: BSD make (macOS /usr/bin/make) has no `%` pattern rules.
docker.linux docker.macos docker.windows docker.wasm:
	@set -e; \
	platform=`echo $@ | sed 's/^docker.//'`; \
	test -f ../Dockerfile.$$platform || { echo "Unknown platform: $$platform (missing Dockerfile.$$platform)"; exit 1; }; \
	mkdir -p ../dist; \
	docker build -f ../Dockerfile.$$platform -t lupinho-$$platform ..; \
	workdir=`awk '/^WORKDIR/{print $$2}' ../Dockerfile.$$platform | tail -1`; \
	cid=`docker create lupinho-$$platform`; \
	docker cp "$$cid:$${workdir}/dist/." ../dist/; \
	docker rm "$$cid"; \
	echo "Built $$platform -> ../dist/"

# Clean generated files
clean:
	rm -rf ../dist/webgame/ $(RAYLIB_MACOS_DIR) $(RAYLIB_WEB_OBJDIR) libs/web/c-build
	rm -f $(OUTPUT) ../dist/game.js ../dist/game.wasm ../dist/game.data \
		lupi_emulator ../dist/lupinho ../dist/lupinho-static ../dist/lupinho.exe \
		../dist/lupinho-x86_64 ../dist/lupinho-arm64
	rm -f $(RAYLIB_LINUX_OBJS) $(RAYLIB_WIN_OBJS) \
		$(RAYLIB_LINUX_DIR)/libraylib.a $(RAYLIB_WIN_DIR)/libraylib.a

# Help
help:
	@echo "Lupi Emulator Makefile"
	@echo "======================"
	@echo ""
	@echo "Targets:"
	@echo "  make web         - Build for WebAssembly (development mode with debug)"
	@echo "  make production  - Build for WebAssembly (optimized, no debug)"
	@echo "  make web-game GAME_PATH=game.lupi - Build for Web with embedded .lupi game (output: dist/webgame/)"
	@echo "  make linux       - Build for Linux desktop (vendored raylib + Lua)"
	@echo "  make macos       - Build macOS x86_64 + arm64 (and universal lupinho if lipo exists)"
	@echo "  make windows     - Build lupinho.exe with MinGW (cross or native)"
	@echo "  make docker.PLATFORM - Build via Dockerfile.PLATFORM (linux, macos, windows, wasm)"
	@echo "  make static      - Alias of linux with stripped binary"
	@echo "  make clean       - Remove generated files"
	@echo ""
	@echo "Dependencies:"
	@echo "  make web         - Emscripten SDK"
	@echo "  make linux       - gcc, zlib, X11, OpenGL"
	@echo "  make macos       - osxcross o64-clang + oa64-clang, or Xcode clang -arch; vendored raylib"
	@echo "  make windows     - MinGW-w64 gcc (x86_64-w64-mingw32-gcc) and zlib"
	@echo "  make docker.*    - Docker Engine (Dockerfile.linux / .macos / .windows / .wasm)"
	@echo "  make static      - same as linux"
	@echo ""
	@echo "Note: For WebAssembly builds, Lua must be compiled with Emscripten."
	@echo "      If you get linking errors, you may need to compile Lua with emcc."

.PHONY: web production linux macos macos-arch macos-x86_64 macos-arm64 macos-universal windows docker.linux docker.macos docker.windows docker.wasm static clean help
