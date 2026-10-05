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
SRC = main.c ui.c lua_api.c

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

# Lua 5.5.1 (submodule src/libs/lua; compiled from source on each target)
LUA_55_INCLUDE = -Ilibs/lua
LUA_55_DIR = libs/lua
# Interpreter, compiler, amalgamation, and the debug test module are not the library.
LUA_55_SRC = $(filter-out $(LUA_55_DIR)/lua.c $(LUA_55_DIR)/luac.c $(LUA_55_DIR)/onelua.c $(LUA_55_DIR)/ltests.c,$(wildcard $(LUA_55_DIR)/*.c))

# Native (Linux) Libraries — raylib objects built from vendored sources
NATIVE_INCLUDE = $(LUA_55_INCLUDE) $(RAYLIB_INCLUDE)
NATIVE_LIBS = $(RAYLIB_LINUX_OBJS) -lGL -lm -lpthread -ldl -lrt -lX11 -lXcursor -lXinerama -lXrandr -lXi -lz

# Native (Linux / macOS / Windows) Source Files
SRC_NATIVE = main.c ui.c lua_api.c zip.c

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

# Web (Lua 5.5.1 + vendored raylib, compiled with emcc)
$(RAYLIB_WEB_OBJDIR)/%.o: $(RAYLIB_SRC)/%.c
	@mkdir -p $(RAYLIB_WEB_OBJDIR)
	$(CC) -c $< -o $@ $(RAYLIB_WEB_CFLAGS)

# Web Target (development with debug)
web: $(SRC) $(LUA_55_SRC) $(RAYLIB_WEB_OBJS)
	@mkdir -p ../dist
	$(CC) $(SRC) $(LUA_55_SRC) $(RAYLIB_WEB_OBJS) -o $(OUTPUT) \
		$(LUA_55_INCLUDE) $(RAYLIB_INCLUDE) $(CFLAGS) $(EMFLAGS) $(OPTIMIZATION) $(DEBUG_FLAGS)

# Production Target (optimized, no debug)
production: $(SRC) $(LUA_55_SRC) $(RAYLIB_WEB_OBJS)
	@mkdir -p ../dist
	$(CC) $(SRC) $(LUA_55_SRC) $(RAYLIB_WEB_OBJS) -o $(OUTPUT) \
		$(LUA_55_INCLUDE) $(RAYLIB_INCLUDE) $(CFLAGS) $(EMFLAGS) $(PROD_FLAGS)

# Web Game Target (embed .lupi file)
web-game: $(SRC) $(LUA_55_SRC) $(RAYLIB_WEB_OBJS)
	@if [ -z "$(GAME_PATH)" ]; then echo "Error: GAME_PATH is required. Usage: make web-game GAME_PATH=game.lupi"; exit 1; fi
	@echo "Extracting $(GAME_PATH)..."
	@rm -rf /tmp/lupi-web-game
	@mkdir -p /tmp/lupi-web-game
	@unzip -q "$(GAME_PATH)" -d /tmp/lupi-web-game -x "*.git*" "*.lupi"
	@echo "Building with embedded game..."
	@mkdir -p ../dist/webgame
	$(CC) $(SRC) $(LUA_55_SRC) $(RAYLIB_WEB_OBJS) -o ../dist/webgame/game.html \
		$(LUA_55_INCLUDE) $(RAYLIB_INCLUDE) $(CFLAGS) $(EMFLAGS) $(OPTIMIZATION) -DAUTO_LOAD_GAME \
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
linux: $(SRC_NATIVE) $(LUA_55_SRC) $(RAYLIB_LINUX_OBJS)
	@mkdir -p ../dist
	$(CC_NATIVE) $(SRC_NATIVE) $(LUA_55_SRC) -o ../dist/lupinho \
		$(NATIVE_INCLUDE) $(CFLAGS) -DLUA_USE_POSIX -DLUA_USE_DLOPEN \
		$(NATIVE_LIBS) $(OPTIMIZATION)

# macOS (osxcross o64/oa64-clang, or native clang -arch)
$(RAYLIB_MACOS_ARCH_DIR)/%.o: $(RAYLIB_SRC)/%.c
	@mkdir -p $(RAYLIB_MACOS_ARCH_DIR)
	$(CC_OSX) -c $< -o $@ $(RAYLIB_CFLAGS) -U_GNU_SOURCE

$(RAYLIB_MACOS_ARCH_DIR)/rglfw.o: $(RAYLIB_SRC)/rglfw.c
	@mkdir -p $(RAYLIB_MACOS_ARCH_DIR)
	$(CC_OSX) -x objective-c -c $< -o $@ $(RAYLIB_CFLAGS) -U_GNU_SOURCE

macos-arch: $(SRC_NATIVE) $(LUA_55_SRC) $(RAYLIB_MACOS_OBJS)
	@mkdir -p ../dist
	$(CC_OSX) $(SRC_NATIVE) $(LUA_55_SRC) -o ../dist/lupinho-$(RAYLIB_MACOS_ARCH) \
		$(MACOS_INCLUDE) $(CFLAGS) -DLUA_USE_MACOSX \
		$(MACOS_LIBS) $(OPTIMIZATION)

macos-x86_64:
	$(MAKE) macos-arch RAYLIB_MACOS_ARCH=x86_64 CC_OSX="$(CC_OSX_X86)"

macos-arm64:
	$(MAKE) macos-arch RAYLIB_MACOS_ARCH=arm64 CC_OSX="$(CC_OSX_ARM)"

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
windows: $(SRC_NATIVE) $(LUA_55_SRC) $(RAYLIB_WIN_OBJS)
	@mkdir -p ../dist
	$(CC_MINGW) $(SRC_NATIVE) $(LUA_55_SRC) -o ../dist/lupinho.exe \
		$(WIN_INCLUDE) $(WIN_CFLAGS) $(OPTIMIZATION) \
		$(WIN_LIBS)

# Docker SDK: `make docker.linux`, `make docker.macos`, `make docker.windows`
# Explicit names: BSD make (macOS /usr/bin/make) has no `%` pattern rules.
docker.linux docker.macos docker.windows docker.wasm:
	@set -e; \
	platform=`echo $@ | sed 's/^docker.//'`; \
	test -f ../Dockerfile.$$platform || { echo "Unknown platform: $$platform (missing Dockerfile.$$platform)"; exit 1; }; \
	git -C .. submodule update --init src/libs/lua; \
	mkdir -p ../dist; \
	docker build -f ../Dockerfile.$$platform -t lupinho-$$platform ..; \
	workdir=`awk '/^WORKDIR/{print $$2}' ../Dockerfile.$$platform | tail -1`; \
	cid=`docker create lupinho-$$platform`; \
	docker cp "$$cid:$${workdir}/dist/." ../dist/; \
	docker rm "$$cid"; \
	echo "Built $$platform -> ../dist/"

# Clean generated files
clean:
	rm -rf ../dist/webgame/ $(RAYLIB_MACOS_DIR) $(RAYLIB_WEB_OBJDIR)
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
