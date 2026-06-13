#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 /path/to/kindlesudoku-binary" >&2
  exit 1
fi

BIN_PATH="$1"
if [[ ! -f "$BIN_PATH" ]]; then
  echo "Binary not found: $BIN_PATH" >&2
  exit 1
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DIST_DIR="$ROOT_DIR/dist"
STAGE_DIR="$ROOT_DIR/build/package/kindlesudoku"

rm -rf "$ROOT_DIR/build/package"
mkdir -p "$STAGE_DIR/bin" "$STAGE_DIR/data" "$DIST_DIR"

cp -R "$ROOT_DIR/extension/"* "$STAGE_DIR/"
cp "$BIN_PATH" "$STAGE_DIR/bin/kindlesudoku"
chmod +x "$STAGE_DIR/bin/kindlesudoku"
chmod +x "$STAGE_DIR/bin/start.sh"

if command -v arm-kindlehf-linux-gnueabihf-strip >/dev/null 2>&1; then
  arm-kindlehf-linux-gnueabihf-strip "$STAGE_DIR/bin/kindlesudoku" || true
elif command -v strip >/dev/null 2>&1; then
  strip "$STAGE_DIR/bin/kindlesudoku" || true
fi

(
  cd "$ROOT_DIR/build/package"
  zip -qr "$DIST_DIR/kindlesudoku-kual.zip" kindlesudoku
)

echo "Created $DIST_DIR/kindlesudoku-kual.zip"
