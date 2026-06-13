#pragma once

#include "sudoku_engine.h"

#include <ctime>
#include <string>
#include <vector>

namespace ksudoku {

enum class EntryMode {
    Normal,
    Pencil
};

struct GameState {
    SizeSpec spec = spec_for_size(9);
    Difficulty difficulty = Difficulty::Normal;
    std::vector<int> solution;
    std::vector<int> givens;
    std::vector<int> entries;
    std::vector<int> pencils;
    std::vector<bool> incorrect;

    EntryMode mode = EntryMode::Normal;
    int selected_number = 1; // 0 means erase tool.
    int selected_cell = -1;
    int highlight_number = 0;
    long elapsed_seconds = 0;
    std::time_t session_started = 0;

    void reset_from_puzzle(const GeneratedPuzzle& puzzle);
    int size() const { return spec.size; }
    int cell_count() const { return spec.size * spec.size; }
    bool is_given(int index) const;
    int visible_value(int index) const;
    bool is_correct_visible_value(int index) const;
    void refresh_incorrect_flags();
    int correct_count_for_number(int number) const;
    bool number_is_complete(int number) const;
    bool is_complete() const;
    void start_session_clock();
    void fold_session_time();
};

class SaveManager {
public:
    explicit SaveManager(std::string path);

    bool load(GameState& state);
    bool save(GameState state);
    const std::string& path() const { return path_; }

private:
    std::string path_;
};

} // namespace ksudoku
