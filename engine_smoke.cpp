#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TARGET="kindlehf"
TOOLCHAIN_DIR="$HOME/x-tools/arm-kindlehf-linux-gnueabihf"
CROSS_FILE="$TOOLCHAIN_DIR/meson-crosscompile.txt"

cd "$ROOT_DIR"
export PATH="$TOOLCHAIN_DIR/bin:$PATH"

if [[ ! -x "$TOOLCHAIN_DIR/bin/arm-kindlehf-linux-gnueabihf-g++" || ! -f "$CROSS_FILE" ]]; then
  rm -rf /tmp/koxtoolchain
  git clone --recursive --depth=1 https://github.com/koreader/koxtoolchain.git /tmp/koxtoolchain
  chmod +x /tmp/koxtoolchain/gen-tc.sh
  (cd /tmp/koxtoolchain && ./gen-tc.sh "$TARGET")
fi

if [[ ! -d "$HOME/kindle-sdk" ]]; then
  git clone --recursive --depth=1 https://github.com/KindleModding/kindle-sdk.git "$HOME/kindle-sdk"
fi

chmod +x "$HOME/kindle-sdk/gen-sdk.sh"
(cd "$HOME/kindle-sdk" && ./gen-sdk.sh "$TARGET")

if [[ ! -f "$CROSS_FILE" ]]; then
  echo "Missing Meson cross file: $CROSS_FILE" >&2
  exit 1
fi

meson setup --wipe --cross-file "$CROSS_FILE" builddir_kindlehf
meson compile -C builddir_kindlehf
bash ./scripts/package-kual.sh ./builddir_kindlehf/kindlesudoku
