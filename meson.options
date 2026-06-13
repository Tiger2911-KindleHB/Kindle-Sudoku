#include "sudoku_engine.h"

#include <algorithm>
#include <chrono>
#include <stdexcept>

namespace ksudoku {

SizeSpec spec_for_size(int size) {
    switch (size) {
        case 4: return {4, 2, 2, "4x4 Mini"};
        case 6: return {6, 2, 3, "6x6 Small"};
        case 9: return {9, 3, 3, "9x9 Classic"};
        default: throw std::runtime_error("Unsupported Sudoku size");
    }
}

const char* difficulty_to_string(Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Easy: return "Easy";
        case Difficulty::Normal: return "Normal";
        case Difficulty::Hard: return "Hard";
        case Difficulty::Expert: return "Expert";
    }
    return "Normal";
}

Difficulty difficulty_from_string(const std::string& value) {
    if (value == "Easy") return Difficulty::Easy;
    if (value == "Hard") return Difficulty::Hard;
    if (value == "Expert") return Difficulty::Expert;
    return Difficulty::Normal;
}

int target_clues_for(SizeSpec spec, Difficulty difficulty) {
    if (spec.size == 4) {
        switch (difficulty) {
            case Difficulty::Easy: return 12;
            case Difficulty::Normal: return 10;
            case Difficulty::Hard: return 8;
            case Difficulty::Expert: return 6;
        }
    }
    if (spec.size == 6) {
        switch (difficulty) {
            case Difficulty::Easy: return 25;
            case Difficulty::Normal: return 21;
            case Difficulty::Hard: return 17;
            case Difficulty::Expert: return 14;
        }
    }
    switch (difficulty) {
        case Difficulty::Easy: return 45;
        case Difficulty::Normal: return 38;
        case Difficulty::Hard: return 32;
        case Difficulty::Expert: return 27;
    }
    return 38;
}

SudokuEngine::SudokuEngine() {
    const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::random_device rd;
    rng_.seed(static_cast<unsigned int>(now ^ rd()));
}

GeneratedPuzzle SudokuEngine::generate(int size, Difficulty difficulty) {
    const SizeSpec spec = spec_for_size(size);

    for (int attempt = 0; attempt < 12; ++attempt) {
        GeneratedPuzzle puzzle;
        puzzle.spec = spec;
        puzzle.difficulty = difficulty;
        puzzle.solution = make_solution(spec);
        puzzle.givens = puzzle.solution;

        std::vector<int> positions(spec.size * spec.size);
        for (int i = 0; i < static_cast<int>(positions.size()); ++i) positions[i] = i;
        std::shuffle(positions.begin(), positions.end(), rng_);

        int clues = spec.size * spec.size;
        const int target = target_clues_for(spec, difficulty);

        for (int pos : positions) {
            if (clues <= target) break;
            const int backup = puzzle.givens[pos];
            puzzle.givens[pos] = 0;

            if (has_unique_solution(spec, puzzle.givens)) {
                --clues;
            } else {
                puzzle.givens[pos] = backup;
            }
        }

        if (clues <= target + 3 && has_unique_solution(spec, puzzle.givens)) {
            return puzzle;
        }
    }

    GeneratedPuzzle fallback;
    fallback.spec = spec;
    fallback.difficulty = difficulty;
    fallback.solution = make_solution(spec);
    fallback.givens = fallback.solution;

    std::vector<int> positions(spec.size * spec.size);
    for (int i = 0; i < static_cast<int>(positions.size()); ++i) positions[i] = i;
    std::shuffle(positions.begin(), positions.end(), rng_);
    const int fallback_clues = std::max(target_clues_for(spec, Difficulty::Easy), target_clues_for(spec, difficulty));
    int clues = spec.size * spec.size;
    for (int pos : positions) {
        if (clues <= fallback_clues) break;
        int backup = fallback.givens[pos];
        fallback.givens[pos] = 0;
        if (has_unique_solution(spec, fallback.givens)) --clues;
        else fallback.givens[pos] = backup;
    }
    return fallback;
}

std::vector<int> SudokuEngine::make_solution(const SizeSpec& spec) {
    const int n = spec.size;
    std::vector<int> rows;
    rows.reserve(n);
    std::vector<int> band_order;
    for (int b = 0; b < n / spec.box_rows; ++b) band_order.push_back(b);
    std::shuffle(band_order.begin(), band_order.end(), rng_);
    for (int band : band_order) {
        std::vector<int> inner;
        for (int r = 0; r < spec.box_rows; ++r) inner.push_back(r);
        std::shuffle(inner.begin(), inner.end(), rng_);
        for (int r : inner) rows.push_back(band * spec.box_rows + r);
    }

    std::vector<int> cols;
    cols.reserve(n);
    std::vector<int> stack_order;
    for (int s = 0; s < n / spec.box_cols; ++s) stack_order.push_back(s);
    std::shuffle(stack_order.begin(), stack_order.end(), rng_);
    for (int stack : stack_order) {
        std::vector<int> inner;
        for (int c = 0; c < spec.box_cols; ++c) inner.push_back(c);
        std::shuffle(inner.begin(), inner.end(), rng_);
        for (int c : inner) cols.push_back(stack * spec.box_cols + c);
    }

    std::vector<int> digits;
    for (int d = 1; d <= n; ++d) digits.push_back(d);
    std::shuffle(digits.begin(), digits.end(), rng_);

    std::vector<int> board(n * n, 0);
    for (int rr = 0; rr < n; ++rr) {
        for (int cc = 0; cc < n; ++cc) {
            const int r = rows[rr];
            const int c = cols[cc];
            const int pattern = (spec.box_cols * (r % spec.box_rows) + r / spec.box_rows + c) % n;
            board[rr * n + cc] = digits[pattern];
        }
    }
    return board;
}

bool SudokuEngine::has_unique_solution(const SizeSpec& spec, const std::vector<int>& board) const {
    std::vector<int> work = board;
    int count = 0;
    count_solutions(spec, work, count, 2);
    return count == 1;
}

bool SudokuEngine::count_solutions(const SizeSpec& spec,
                                   std::vector<int>& board,
                                   int& count,
                                   int limit) const {
    if (count >= limit) return true;

    const int n = spec.size;
    int best_index = -1;
    std::vector<int> best_values;

    for (int idx = 0; idx < n * n; ++idx) {
        if (board[idx] != 0) continue;
        const int row = idx / n;
        const int col = idx % n;
        std::vector<int> values;
        for (int value = 1; value <= n; ++value) {
            if (can_place(spec, board, row, col, value)) values.push_back(value);
        }
        if (values.empty()) return false;
        if (best_index < 0 || values.size() < best_values.size()) {
            best_index = idx;
            best_values = values;
            if (values.size() == 1) break;
        }
    }

    if (best_index < 0) {
        ++count;
        return count >= limit;
    }

    std::shuffle(best_values.begin(), best_values.end(), rng_);
    const int row = best_index / n;
    const int col = best_index % n;
    for (int value : best_values) {
        if (!can_place(spec, board, row, col, value)) continue;
        board[best_index] = value;
        if (count_solutions(spec, board, count, limit)) {
            board[best_index] = 0;
            return true;
        }
        board[best_index] = 0;
    }
    return false;
}

bool SudokuEngine::can_place(const SizeSpec& spec,
                             const std::vector<int>& board,
                             int row,
                             int col,
                             int value) const {
    const int n = spec.size;
    for (int c = 0; c < n; ++c) {
        if (board[row * n + c] == value) return false;
    }
    for (int r = 0; r < n; ++r) {
        if (board[r * n + col] == value) return false;
    }
    const int box_r = (row / spec.box_rows) * spec.box_rows;
    const int box_c = (col / spec.box_cols) * spec.box_cols;
    for (int r = 0; r < spec.box_rows; ++r) {
        for (int c = 0; c < spec.box_cols; ++c) {
            if (board[(box_r + r) * n + box_c + c] == value) return false;
        }
    }
    return true;
}

bool SudokuEngine::is_valid_solution(const SizeSpec& spec, const std::vector<int>& solution) const {
    if (static_cast<int>(solution.size()) != spec.size * spec.size) return false;
    for (int idx = 0; idx < spec.size * spec.size; ++idx) {
        const int value = solution[idx];
        if (value < 1 || value > spec.size) return false;
        std::vector<int> copy = solution;
        copy[idx] = 0;
        if (!can_place(spec, copy, idx / spec.size, idx % spec.size, value)) return false;
    }
    return true;
}

} // namespace ksudoku
