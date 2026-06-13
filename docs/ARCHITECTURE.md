# Architecture

## Modules

- `sudoku_engine`: puzzle generation, solution validation, uniqueness testing.
- `game_state`: saved puzzle data, editable entries, pencil marks, incorrect flags, completed-number tracking.
- `ui`: GTK2 full-screen drawing surface, touch handling, modal flow, E Ink-friendly layout.
- `main`: app directory detection and startup.

## Puzzle Generation

The generator creates a randomized solved grid using Sudoku-preserving row, column, band, stack, and digit permutations. It then removes clues while checking that the puzzle still has exactly one solution.

Supported box layouts:

- 4x4: 2x2 boxes
- 6x6: 2x3 boxes
- 9x9: 3x3 boxes

## Save / Restore

State is written to `data/save.dat` after meaningful actions and on exit. The save includes:

- puzzle size
- difficulty
- solution
- original givens
- user entries
- pencil marks
- selected mode
- selected number
- elapsed time

Incorrect flags are recalculated from entries and solution on load.

## E Ink Strategy

- One custom drawing surface.
- Black/white UI with limited gray only for pencil marks and modal dimming.
- Large controls.
- No animations.
- Redraw only after touch actions.
- No background puzzle generation.
