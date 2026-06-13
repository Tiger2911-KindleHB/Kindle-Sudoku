name: Build KindleSudoku for Kindle

'on':
  push:
  pull_request:
  workflow_dispatch:

env:
  FORCE_JAVASCRIPT_ACTIONS_TO_NODE24: true

jobs:
  desktop-smoke:
    name: Desktop smoke build
    runs-on: ubuntu-22.04
    steps:
      - name: Checkout
        uses: actions/checkout@v4

      - name: Install desktop build dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y \
            build-essential \
            meson \
            ninja-build \
            pkg-config \
            zip \
            libgtk2.0-dev

      - name: Normalize script permissions
        run: |
          chmod +x scripts/*.sh || true
          chmod +x extension/bin/start.sh || true

      - name: Configure desktop build
        run: meson setup builddir

      - name: Compile desktop build
        run: meson compile -C builddir

      - name: Run Sudoku engine smoke test
        run: meson test -C builddir --print-errorlogs

      - name: Package desktop debug KUAL zip
        run: bash ./scripts/package-kual.sh ./builddir/kindlesudoku

      - name: Upload desktop debug artifact
        uses: actions/upload-artifact@v4
        with:
          name: kindlesudoku-desktop-debug-kual
          path: dist/kindlesudoku-kual.zip

  kindlehf:
    name: Kindle PW12 kindlehf build
    runs-on: ubuntu-22.04
    steps:
      - name: Checkout
        uses: actions/checkout@v4

      - name: Install host build dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y \
            build-essential \
            autoconf \
            automake \
            bison \
            flex \
            gawk \
            libtool \
            libtool-bin \
            libncurses-dev \
            curl \
            file \
            git \
            gperf \
            help2man \
            texinfo \
            unzip \
            wget \
            sed \
            libarchive-dev \
            nettle-dev \
            meson \
            ninja-build \
            pkg-config \
            zip \
            python3 \
            make \
            binutils

      - name: Cache Kindle toolchain and SDK
        uses: actions/cache@v4
        with:
          path: |
            ~/x-tools/arm-kindlehf-linux-gnueabihf
            ~/kindle-sdk
          key: kindlesudoku-kindlehf-toolchain-sdk-v1

      - name: Normalize script permissions
        run: |
          chmod +x scripts/*.sh || true
          chmod +x extension/bin/start.sh || true

      - name: Build Kindle binary and KUAL package
        run: bash ./scripts/build-kindlehf.sh

      - name: Upload Kindle KUAL artifact
        uses: actions/upload-artifact@v4
        with:
          name: kindlesudoku-kual
          path: dist/kindlesudoku-kual.zip
