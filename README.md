# Kindle Sudoku

Native C++ / GTK2 Sudoku for jailbroken Kindle devices, packaged as a KUAL extension and tuned for E Ink readability.

Primary target: **Kindle Paperwhite 12th Generation, firmware 5.17.x, kindlehf**.

## Current V1 Feature Set

- Native Kindle/KUAL application layout
- High-contrast black-and-white UI
- Full-screen GTK2 drawing surface
- Procedural Sudoku generation
- Unique-solution enforcement
- 4x4 Mini puzzles
- 6x6 Small puzzles
- 9x9 Classic puzzles
- Easy / Normal / Hard / Expert difficulty selection
- Top-left **New** button
- Top-right **Exit** button
- Automatic save and restore
- Persistent Normal mode
- Persistent Pencil mode
- Persistent number selection
- Erase tool using `□`
- One pencil mark per cell, drawn in the upper-right corner
- Incorrect entries remain visible and receive an X marker
- Tapping a filled cell highlights matching visible numbers with black/white inversion
- Completed numbers auto-hide from the bottom number bar
- Large touch targets and automatic scaling across screen sizes

## Repository Layout

```text
.github/workflows/main.yml      GitHub Actions cloud builds
cross/kindlehf.txt              Meson cross-file for Kindle PW12-class devices
docs/                           Notes for testing and iteration
extension/                      KUAL extension skeleton
scripts/package-kual.sh         Builds the release zip from a compiled binary
src/                            Application source code
tests/engine_smoke.cpp          Generator/solver smoke test
meson.build                     Meson build definition
```

## Runtime Install Layout

The packaged release installs to:

```text
/mnt/us/extensions/kindlesudoku/
├── config.xml
├── menu.json
├── bin/
│   ├── kindlesudoku
│   └── start.sh
└── data/
    └── save.dat
```

Launch path:

```text
KUAL → Kindle Sudoku → Start
```

## GitHub Actions Build

The workflow builds on push, pull request, and manual dispatch.

Artifacts:

- `kindlesudoku-kual`: Kindle KUAL release zip
- `kindlesudoku-desktop-debug-kual`: desktop smoke-build package, useful only for UI/debug validation on Linux

The Kindle job uses the `kindlehf` KOReader toolchain target, matching modern Kindle firmware 5.16.3+ devices.

## Local Smoke Build

On Ubuntu 22.04 or a matching environment:

```bash
sudo apt-get install -y build-essential meson ninja-build pkg-config libgtk2.0-dev zip
./scripts/build-local.sh
```

This produces:

```text
dist/kindlesudoku-kual.zip
```

For the actual Kindle binary, use the GitHub Actions `kindlehf` build artifact.

## Testing Checklist

1. Download `kindlesudoku-kual.zip` from GitHub Actions.
2. Extract it.
3. Copy the `kindlesudoku` folder to `/mnt/us/extensions/` on the Kindle.
4. Launch from KUAL.
5. Confirm the last puzzle restores after closing and reopening.
6. Test New → 4x4 / 6x6 / 9x9 → each difficulty.
7. Test Normal mode persistence.
8. Test Pencil mode persistence.
9. Test selected number persistence.
10. Test erase behavior on user entries, pencil marks, and givens.
11. Confirm incorrect entries draw an X and do not count as completed numbers.
12. Confirm completed numbers disappear from the bottom bar.
13. Tap filled cells and confirm all matching numbers invert black/white.

## Design Notes

The UI uses one custom GTK drawing surface instead of many widgets. That reduces redraw noise, keeps memory use low, and gives precise control over E Ink-friendly contrast and touch target sizing.

The save format is intentionally simple line-based text so it can be inspected and recovered easily on the Kindle:

```text
/mnt/us/extensions/kindlesudoku/data/save.dat
```

