#!/bin/sh
# Build a headless Furnace static library for the Lupinho music player.
# Usage: build_furnace.sh <tag> <furnace-src> [extra cmake args...]
set -e
cd "$(dirname "$0")/.."

tag="$1"
src="$2"
shift 2

if [ -z "$tag" ] || [ -z "$src" ]; then
  echo "usage: build_furnace.sh <tag> <furnace-src> [cmake args...]" >&2
  exit 1
fi
if [ ! -f "$src/src/engine/engine.h" ]; then
  echo "Furnace sources not found at $src" >&2
  echo "Clone https://github.com/tildearrow/furnace (with submodules) or set FURNACE_SRC." >&2
  echo "Docker images run: sh music/fetch_furnace.sh /furnace" >&2
  exit 1
fi
if ! command -v ninja >/dev/null 2>&1; then
  echo "ninja is required (apt install ninja-build)" >&2
  exit 1
fi

build="music/furnace-build-$tag"
cmake_bin="${CMAKE:-cmake}"
zlib_flag="-DSYSTEM_ZLIB=ON"
# libstdc++ on GCC 12+ does not pull these in transitively. Apple clang still does.
cxx_flags="-include cstring -include climits"
if [ "$tag" = "web" ]; then
  zlib_flag="-DSYSTEM_ZLIB=OFF"
  cxx_flags="$cxx_flags -fexceptions"
fi

invoke_cmake() {
  if [ "$tag" = "web" ]; then
    emcmake "$cmake_bin" "$@"
  else
    "$cmake_bin" "$@"
  fi
}

# Extra cmake args stay last so they can override the defaults above.
invoke_cmake -S "$src" -B "$build" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DBUILD_GUI=OFF \
  -DUSE_SDL2=OFF \
  -DUSE_SNDFILE=ON \
  -DWITH_JACK=OFF \
  -DWITH_PORTAUDIO=OFF \
  -DUSE_RTMIDI=OFF \
  -DWITH_LOCALE=OFF \
  -DUSE_BACKWARD=OFF \
  -DWITH_JSON=OFF \
  -DUSE_FREETYPE=OFF \
  "$zlib_flag" \
  -DSYSTEM_LIBSNDFILE=OFF \
  -DWARNINGS_ARE_ERRORS=OFF \
  "-DCMAKE_CXX_FLAGS=$cxx_flags" \
  "$@"

"$cmake_bin" --build "$build" --target furnace

rm -f "$build/libfurengine.a"
python3 - "$build" << 'PY'
import os, shutil, subprocess, sys
build = sys.argv[1]
root = os.path.join(build, "CMakeFiles", "furnace.dir")
objs = []
for dirpath, _, files in os.walk(root):
    for name in files:
        if not (name.endswith(".o") or name.endswith(".obj")):
            continue
        if name.startswith("main.cpp."):
            continue
        objs.append(os.path.join(dirpath, name))
if not objs:
    sys.stderr.write("no furnace objects to archive\n")
    sys.exit(1)
staging = os.path.join(build, "ar-names")
if os.path.isdir(staging):
    shutil.rmtree(staging)
os.makedirs(staging)
names = []
for i, obj in enumerate(objs):
    dest = os.path.join(staging, "%04d-%s" % (i, os.path.basename(obj)))
    try:
        os.link(obj, dest)
    except OSError:
        shutil.copy2(obj, dest)
    names.append(dest)
out = os.path.join(build, "libfurengine.a")
ar = "/osxcross/bin/apple-ar" if os.path.exists("/osxcross/bin/apple-ar") else "ar"
subprocess.check_call([ar, "rcs", out] + names)
shutil.rmtree(staging)
print("archived", len(objs), "objects")
PY

python3 - "$build" << 'PY'
import sys
build = sys.argv[1]
text = open(build + "/build.ninja", encoding="utf-8", errors="replace").read()
idx = text.find("CXX_EXECUTABLE_LINKER__furnace")
if idx < 0:
    sys.stderr.write("furnace ninja link target not found\n")
    sys.exit(1)
chunk = text[idx:idx + 2000000]
key = "\n  LINK_LIBRARIES = "
at = chunk.find(key)
if at < 0:
    sys.stderr.write("furnace link libraries not found\n")
    sys.exit(1)
rest = chunk[at + len(key):]
parts = []
for line in rest.splitlines():
    parts.append(line.strip().rstrip("$").strip())
    if not line.rstrip().endswith("$"):
        break
blob = " ".join(parts).replace("$$", "$").replace("$ ", " ").replace("\\", "/")
libs = []
for lib in blob.split():
    if (lib.endswith(".a") or lib.endswith(".lib")) and not lib.startswith("/") and not lib.startswith("-"):
        libs.append(build + "/" + lib)
    else:
        libs.append(lib)
open(build + "/furengine-libs.txt", "w").write(" ".join(libs) + "\n")
print("link libs:", " ".join(libs))
PY

echo "furnace library: $build/libfurengine.a"
