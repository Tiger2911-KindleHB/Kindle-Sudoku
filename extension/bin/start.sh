#!/bin/sh
APP_DIR="/mnt/us/extensions/kindlesudoku"
BIN="$APP_DIR/bin/kindlesudoku"

cd "$APP_DIR" || exit 1
mkdir -p "$APP_DIR/data"

# Keep the device from sleeping while actively playing, then restore normal behavior on exit.
lipc-set-prop com.lab126.powerd preventScreenSaver 1 >/dev/null 2>&1 || true
export KINDLESUDOKU_HOME="$APP_DIR"
export GTK_IM_MODULE=xim
export GDK_USE_XFT=1

"$BIN"
STATUS=$?

lipc-set-prop com.lab126.powerd preventScreenSaver 0 >/dev/null 2>&1 || true
exit "$STATUS"
