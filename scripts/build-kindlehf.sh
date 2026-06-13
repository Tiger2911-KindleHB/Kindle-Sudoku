#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TARGET="kindlehf"
TOOLCHAIN_DIR="$HOME/x-tools/arm-kindlehf-linux-gnueabihf"
TOOLCHAIN_BIN="$TOOLCHAIN_DIR/bin/arm-kindlehf-linux-gnueabihf-g++"
CROSS_FILE="$TOOLCHAIN_DIR/meson-crosscompile.txt"
SDK_DIR="$HOME/kindle-sdk"
SDK_MARKER="$TOOLCHAIN_DIR/.kindle-sdk-installed"
TOOLCHAIN_URL="https://github.com/koreader/koxtoolchain/releases/download/2025.05/kindlehf.tar.gz"

cd "$ROOT_DIR"

echo "Using prebuilt kindlehf toolchain only. This script intentionally does not run koxtoolchain/gen-tc.sh."

if [[ ! -x "$TOOLCHAIN_BIN" ]]; then
  echo "Installing prebuilt KOReader kindlehf toolchain..."
  rm -rf "$TOOLCHAIN_DIR"
  mkdir -p "$HOME/x-tools"
  curl -fL \
    --retry 5 \
    --retry-delay 5 \
    --connect-timeout 30 \
    --max-time 600 \
    -o /tmp/kindlehf.tar.gz \
    "$TOOLCHAIN_URL"
  tar -xzf /tmp/kindlehf.tar.gz -C "$HOME"
fi

if [[ ! -x "$TOOLCHAIN_BIN" ]]; then
  echo "Missing Kindle compiler after installing prebuilt toolchain: $TOOLCHAIN_BIN" >&2
  exit 1
fi

export PATH="$TOOLCHAIN_DIR/bin:$PATH"
"$TOOLCHAIN_BIN" --version | head -n 1

if [[ ! -d "$SDK_DIR" ]]; then
  git clone --recursive --depth=1 https://github.com/KindleModding/kindle-sdk.git "$SDK_DIR"
fi

# Kindle SDK adds Kindle libraries/pkg-config data on top of the existing prebuilt toolchain.
# It should not build the cross-toolchain from source.
if [[ ! -f "$SDK_MARKER" || ! -f "$CROSS_FILE" ]]; then
  chmod +x "$SDK_DIR/gen-sdk.sh"
  (cd "$SDK_DIR" && ./gen-sdk.sh "$TARGET")
  touch "$SDK_MARKER"
fi

if [[ ! -f "$CROSS_FILE" ]]; then
  echo "Missing Meson cross file: $CROSS_FILE" >&2
  exit 1
fi

meson setup --wipe --cross-file "$CROSS_FILE" builddir_kindlehf
meson compile -C builddir_kindlehf
bash ./scripts/package-kual.sh ./builddir_kindlehf/kindlesudoku
