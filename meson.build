#pragma once

#include <random>
#include <string>
#include <vector>

namespace ksudoku {

enum class Difficulty {
    Easy,
    Normal,
    Hard,
    Expert
};

struct SizeSpec {
    int size;
    int box_rows;
    int box_cols;
    const char* label;
};

struct GeneratedPuzzle {
    SizeSpec spec;
    Difficulty difficulty;
    std::vector<int> solution;
    std::vector<int> givens;
};

SizeSpec spec_for_size(int size);
const char* difficulty_to_string(Difficulty difficulty);
Difficulty difficulty_from_string(const std::string& value);
int target_clues_for(SizeSpec spec, Difficulty difficulty);

class SudokuEngine {
public:
    SudokuEngine();

    GeneratedPuzzle generate(int size, Difficulty difficulty);
    bool has_unique_solution(const SizeSpec& spec, const std::vector<int>& board) const;
    bool is_valid_solution(const SizeSpec& spec, const std::vector<int>& solution) const;

private:
    mutable std::mt19937 rng_;

    std::vector<int> make_solution(const SizeSpec& spec);
    bool count_solutions(const SizeSpec& spec,
                         std::vector<int>& board,
                         int& count,
                         int limit) const;
    bool can_place(const SizeSpec& spec,
                   const std::vector<int>& board,
                   int row,
                   int col,
                   int value) const;
};

} // namespace ksudoku
