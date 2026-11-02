#!/bin/sh
# Fetch the Furnace revision Lupinho was tested against.
# Usage: fetch_furnace.sh [destination]
set -eu

dest="${1:-/furnace}"
rev=bcba71de409d519f9053e50b9d2b26412206936f
url=https://github.com/tildearrow/furnace.git

if [ -f "$dest/src/engine/engine.h" ] && [ -f "$dest/extern/fmt/include/fmt/format.h" ]; then
  echo "Furnace sources already at $dest"
  exit 0
fi

export GIT_TERMINAL_PROMPT=0
rm -rf "$dest"
git init "$dest"
git -C "$dest" remote add origin "$url"
git -C "$dest" fetch --depth 1 origin "$rev"
git -C "$dest" checkout --detach FETCH_HEAD
git -C "$dest" submodule update --init --recursive --depth 1

test -f "$dest/src/engine/engine.h"
test -f "$dest/extern/fmt/include/fmt/format.h"
echo "Furnace $rev -> $dest"
