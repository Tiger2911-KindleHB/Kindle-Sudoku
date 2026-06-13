#!/bin/sh
APP_DIR="/mnt/us/extensions/kindlesudoku"
BIN="$APP_DIR/bin/kindlesudoku"
DATA_DIR="$APP_DIR/data"
LOG="$DATA_DIR/launch.log"

mkdir -p "$DATA_DIR"

# Keep a small, persistent launch log so Kindle/KUAL failures are not silent.
# This is intentionally plain sh for Kindle compatibility.
{
  echo "============================================================"
  echo "Kindle Sudoku launch: $(date 2>/dev/null || echo unknown-date)"
  echo "APP_DIR=$APP_DIR"
  echo "BIN=$BIN"
  echo "PWD=$(pwd 2>/dev/null || echo unknown)"
  echo "USER=$(id 2>/dev/null || echo unknown)"

  cd "$APP_DIR" || {
    echo "ERROR: cannot cd to $APP_DIR"
    exit 1
  }

  if [ ! -f "$BIN" ]; then
    echo "ERROR: binary missing: $BIN"
    exit 1
  fi

  if [ ! -x "$BIN" ]; then
    echo "WARN: binary was not executable; attempting chmod +x"
    chmod +x "$BIN" 2>/dev/null || true
  fi

  if [ ! -x "$BIN" ]; then
    echo "ERROR: binary is still not executable: $BIN"
    ls -l "$BIN" 2>/dev/null || true
    exit 1
  fi

  ls -l "$BIN" 2>/dev/null || true

  # KUAL launches from the Kindle framework environment, but DISPLAY is not
  # guaranteed to be exported on every device/firmware combination.
  export DISPLAY="${DISPLAY:-:0}"
  export KINDLESUDOKU_HOME="$APP_DIR"
  export HOME="$APP_DIR"
  export GTK_IM_MODULE=xim
  export GDK_USE_XFT=1
  export GDK_CORE_DEVICE_EVENTS=1

  # Do not let the screensaver trigger while the game is active.
  lipc-set-prop com.lab126.powerd preventScreenSaver 1 >/dev/null 2>&1 || true

  echo "DISPLAY=$DISPLAY"
  echo "Starting binary..."
  "$BIN"
  STATUS=$?
  echo "Binary exited with status: $STATUS"

  lipc-set-prop com.lab126.powerd preventScreenSaver 0 >/dev/null 2>&1 || true
  exit "$STATUS"
} >> "$LOG" 2>&1
