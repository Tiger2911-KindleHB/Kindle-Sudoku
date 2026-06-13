#include "../src/sudoku_engine.h"

#include <iostream>
#include <stdexcept>

int main() {
    ksudoku::SudokuEngine engine;
    const int sizes[] = {4, 6, 9};
    const ksudoku::Difficulty difficulties[] = {
        ksudoku::Difficulty::Easy,
        ksudoku::Difficulty::Normal,
        ksudoku::Difficulty::Hard,
        ksudoku::Difficulty::Expert
    };

    for (int size : sizes) {
        for (auto difficulty : difficulties) {
            auto puzzle = engine.generate(size, difficulty);
            if (!engine.is_valid_solution(puzzle.spec, puzzle.solution)) {
                throw std::runtime_error("invalid generated solution");
            }
            if (!engine.has_unique_solution(puzzle.spec, puzzle.givens)) {
                throw std::runtime_error("generated puzzle is not unique");
            }
            std::cout << puzzle.spec.label << " " << ksudoku::difficulty_to_string(difficulty) << " OK\n";
        }
    }
    return 0;
}
