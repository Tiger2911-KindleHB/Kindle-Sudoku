# Kindle Sudoku

A native Sudoku application designed specifically for jailbroken Kindle devices and E-Ink displays.

Kindle Sudoku is built to feel like a clean, focused, Kindle-native puzzle app rather than a generic mobile Sudoku port. It uses a high-contrast black-and-white interface, large touch targets, persistent input modes, automatic save/restore, procedural puzzle generation, and a KUAL-friendly install package.

## Features

### Kindle-Optimized Interface

- Native Kindle application packaged as a KUAL extension
- High-contrast black-and-white UI for E-Ink readability
- Large board numbers, buttons, and touch targets
- Centered Sudoku board with clean grid rendering
- Enlarged bottom number bar for easier touch input
- Minimal visual clutter and distraction-free gameplay
- Designed for Kindle Paperwhite 6 / modern jailbroken Kindle devices

### Puzzle Sizes

Version 1 supports three Sudoku formats:

- **4x4 Mini** — digits 1–4
- **6x6 Small** — digits 1–6
- **9x9 Classic** — digits 1–9

The interface automatically scales the board and controls for the selected puzzle size.

### Difficulty Levels

Each puzzle size supports four difficulty levels:

- Easy
- Normal
- Hard
- Expert

Puzzles are procedurally generated and validated to ensure they have a valid solution.

### Gameplay Controls

- **New** — start a new puzzle by selecting size and difficulty
- **Check** — scan the current puzzle for invalid entries
- **Stats** — view fastest completion times by puzzle type and difficulty
- **Exit** — save progress and close the application
- **Ink** — enter final numbers into cells
- **Pencil** — place small pencil-style candidate marks
- **Erase** — clear user-entered numbers or pencil marks

Original puzzle givens cannot be edited or erased.

### Persistent Input Behavior

Kindle Sudoku is designed to reduce unnecessary taps on E-Ink hardware.

- The selected number remains active until changed
- The selected input mode remains active until changed
- Multiple cells can be filled using the same selected number
- Number-bar selections do not clear active board highlights

### Pencil Marks

The **Pencil** mode allows a single small pencil-style mark per cell. Pencil marks are drawn smaller and lighter than final entries so they remain visually distinct from confirmed answers.

### Mistake Detection

Incorrect final entries are left visible but marked clearly with an X. Invalid entries do not count toward puzzle completion.

The **Check** button scans the current board and reports how many invalid entries are currently present.

### Number Highlighting

Tapping a filled cell highlights every matching visible number on the board using an inverted black-and-white style:

- Cell background becomes black
- Number becomes white

Tapping empty board space or an empty cell clears the highlight. Tapping number-bar controls does not clear the current highlight.

### Auto-Hide Completed Numbers

When every correct instance of a number has been completed, that number is automatically removed from the number bar.

Examples:

- 4x4: hide a number after 4 correct instances
- 6x6: hide a number after 6 correct instances
- 9x9: hide a number after 9 correct instances

The count includes original givens and correct user entries. It does not include pencil marks or incorrect entries.

### Completion Popup

When a puzzle is completed, Kindle Sudoku displays a completion popup with:

- Congratulations message
- Puzzle size
- Difficulty
- Completion time
- New Puzzle button
- Close button

### Stats and Leaderboards

The Stats screen tracks fastest completion times for each puzzle size and difficulty.

Stats are saved locally on the Kindle at:

```text
/mnt/us/extensions/kindlesudoku/data/stats.dat
```

### Save and Restore

The current puzzle is automatically saved and restored.

Saved state includes:

- Puzzle size
- Difficulty
- Original givens
- User-entered numbers
- Pencil marks
- Incorrect entries
- Current board state
- Elapsed time

Save data is stored locally inside the extension data folder.

## Installation

Builds are distributed as a KUAL extension package.

After downloading the GitHub Actions artifact, extract the final package until you have a folder named:

```text
kindlesudoku/
```

Copy that folder to your Kindle at:

```text
/mnt/us/extensions/kindlesudoku/
```

Expected Kindle folder layout:

```text
/mnt/us/extensions/kindlesudoku/config.xml
/mnt/us/extensions/kindlesudoku/menu.json
/mnt/us/extensions/kindlesudoku/bin/start.sh
/mnt/us/extensions/kindlesudoku/bin/kindlesudoku
```

Launch the app from KUAL.

## Build System

Kindle Sudoku uses the same overall workflow as the Kindle Chess project:

1. Source files are committed to GitHub
2. GitHub Actions runs the Kindle build
3. A KUAL-compatible artifact is produced
4. The artifact is downloaded and extracted
5. The resulting extension folder is copied to the Kindle
6. Hardware testing is performed on the device
7. Fixes are applied through small file patches

The project is built with:

- C++17
- GTK2
- Meson
- Ninja
- KOReader/Kindle `kindlehf` toolchain
- GitHub Actions

## GitHub Actions Artifact

The workflow produces a Kindle/KUAL artifact named:

```text
kindlesudoku-kual
```

Inside the artifact is:

```text
kindlesudoku-kual.zip
```

That zip extracts to the installable KUAL extension folder:

```text
kindlesudoku/
```

## Repository Structure

```text
.github/workflows/     GitHub Actions workflow
extension/             KUAL extension files and launcher scripts
src/                   Application source code
tests/                 Engine smoke tests
scripts/               Build and packaging scripts
meson.build            Meson build configuration
README.md              Project documentation
```

## Runtime Logs

For debugging on Kindle hardware, logs are written to:

```text
/mnt/us/extensions/kindlesudoku/data/launch.log
/mnt/us/extensions/kindlesudoku/data/app.log
```

These logs are useful for diagnosing launch, display, input, and runtime issues.

## Design Goals

Kindle Sudoku prioritizes:

- E-Ink readability
- Low CPU usage
- Low memory usage
- Fast interaction
- Large touch targets
- Reliable save/restore
- Simple code organization
- Patch-friendly development
- KUAL-compatible installation

## Current Status

The current version includes:

- 4x4, 6x6, and 9x9 puzzle generation
- Difficulty selection
- Save/load support
- Mark and Test input modes
- Erase support
- Mistake marking
- Check button
- Number highlighting
- Auto-hide completed numbers
- Completion popup
- Local stats and leaderboards
- Kindle touch fallback handling
- KUAL packaging
- GitHub Actions cloud build

## Notes

This project is intended for jailbroken Kindle devices capable of running KUAL extensions. It is not an official Amazon application.
