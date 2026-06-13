#include "game_state.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

namespace ksudoku {

namespace {

std::string encode_values(const std::vector<int>& values) {
    std::string out;
    out.reserve(values.size());
    for (int value : values) {
        if (value <= 0) out.push_back('.');
        else if (value < 10) out.push_back(static_cast<char>('0' + value));
        else out.push_back(static_cast<char>('A' + (value - 10)));
    }
    return out;
}

bool decode_values(const std::string& text, int expected_size, int max_value, std::vector<int>& values) {
    if (static_cast<int>(text.size()) != expected_size) return false;
    values.assign(expected_size, 0);
    for (int i = 0; i < expected_size; ++i) {
        const char ch = text[i];
        int value = 0;
        if (ch == '.' || ch == '0') value = 0;
        else if (ch >= '1' && ch <= '9') value = ch - '0';
        else if (ch >= 'A' && ch <= 'Z') value = 10 + ch - 'A';
        else if (ch >= 'a' && ch <= 'z') value = 10 + ch - 'a';
        else return false;
        if (value < 0 || value > max_value) return false;
        values[i] = value;
    }
    return true;
}

void mkdir_parent_best_effort(const std::string& path) {
    const std::string::size_type slash = path.find_last_of('/');
    if (slash == std::string::npos) return;
    const std::string parent = path.substr(0, slash);
    if (parent.empty()) return;

    std::string current;
    if (parent[0] == '/') current = "/";
    std::stringstream ss(parent);
    std::string part;
    while (std::getline(ss, part, '/')) {
        if (part.empty()) continue;
        if (current.size() > 1) current += "/";
        current += part;
        mkdir(current.c_str(), 0755);
    }
}

} // namespace

void GameState::reset_from_puzzle(const GeneratedPuzzle& puzzle) {
    spec = puzzle.spec;
    difficulty = puzzle.difficulty;
    solution = puzzle.solution;
    givens = puzzle.givens;
    entries.assign(cell_count(), 0);
    pencils.assign(cell_count(), 0);
    incorrect.assign(cell_count(), false);
    mode = EntryMode::Normal;
    selected_number = 1;
    selected_cell = -1;
    highlight_number = 0;
    elapsed_seconds = 0;
    start_session_clock();
}

bool GameState::is_given(int index) const {
    return index >= 0 && index < cell_count() && givens[index] != 0;
}

int GameState::visible_value(int index) const {
    if (index < 0 || index >= cell_count()) return 0;
    if (givens[index] != 0) return givens[index];
    return entries[index];
}

bool GameState::is_correct_visible_value(int index) const {
    if (index < 0 || index >= cell_count()) return false;
    const int value = visible_value(index);
    return value != 0 && value == solution[index];
}

void GameState::refresh_incorrect_flags() {
    incorrect.assign(cell_count(), false);
    for (int i = 0; i < cell_count(); ++i) {
        if (givens[i] == 0 && entries[i] != 0 && entries[i] != solution[i]) {
            incorrect[i] = true;
        }
    }
}

int GameState::correct_count_for_number(int number) const {
    int count = 0;
    for (int i = 0; i < cell_count(); ++i) {
        if (visible_value(i) == number && is_correct_visible_value(i)) {
            ++count;
        }
    }
    return count;
}

bool GameState::number_is_complete(int number) const {
    return number >= 1 && number <= spec.size && correct_count_for_number(number) >= spec.size;
}

bool GameState::is_complete() const {
    for (int i = 0; i < cell_count(); ++i) {
        if (!is_correct_visible_value(i)) return false;
    }
    return true;
}

void GameState::start_session_clock() {
    session_started = std::time(nullptr);
}

void GameState::fold_session_time() {
    const std::time_t now = std::time(nullptr);
    if (session_started > 0 && now >= session_started) {
        elapsed_seconds += static_cast<long>(now - session_started);
    }
    session_started = now;
}

SaveManager::SaveManager(std::string path) : path_(std::move(path)) {}

bool SaveManager::load(GameState& state) {
    std::ifstream in(path_.c_str());
    if (!in) return false;

    std::map<std::string, std::string> kv;
    std::string line;
    while (std::getline(in, line)) {
        const std::string::size_type eq = line.find('=');
        if (eq == std::string::npos) continue;
        kv[line.substr(0, eq)] = line.substr(eq + 1);
    }

    if (kv["version"] != "1") return false;
    const int size = std::atoi(kv["size"].c_str());
    SizeSpec spec;
    try {
        spec = spec_for_size(size);
    } catch (...) {
        return false;
    }

    const int cells = spec.size * spec.size;
    std::vector<int> solution;
    std::vector<int> givens;
    std::vector<int> entries;
    std::vector<int> pencils;

    if (!decode_values(kv["solution"], cells, spec.size, solution)) return false;
    if (!decode_values(kv["givens"], cells, spec.size, givens)) return false;
    if (!decode_values(kv["entries"], cells, spec.size, entries)) return false;
    if (!decode_values(kv["pencils"], cells, spec.size, pencils)) return false;

    state.spec = spec;
    state.difficulty = difficulty_from_string(kv["difficulty"]);
    state.solution = solution;
    state.givens = givens;
    state.entries = entries;
    state.pencils = pencils;
    state.incorrect.assign(cells, false);
    state.mode = kv["mode"] == "Pencil" ? EntryMode::Pencil : EntryMode::Normal;
    state.selected_number = std::atoi(kv["selected_number"].c_str());
    if (state.selected_number < 0 || state.selected_number > spec.size) state.selected_number = 1;
    state.selected_cell = -1;
    state.highlight_number = 0;
    state.elapsed_seconds = std::max(0, std::atoi(kv["elapsed_seconds"].c_str()));
    state.refresh_incorrect_flags();
    state.start_session_clock();
    return true;
}

bool SaveManager::save(GameState state) {
    state.fold_session_time();
    mkdir_parent_best_effort(path_);

    const std::string tmp = path_ + ".tmp";
    std::ofstream out(tmp.c_str(), std::ios::trunc);
    if (!out) return false;

    out << "version=1\n";
    out << "size=" << state.spec.size << "\n";
    out << "difficulty=" << difficulty_to_string(state.difficulty) << "\n";
    out << "elapsed_seconds=" << state.elapsed_seconds << "\n";
    out << "mode=" << (state.mode == EntryMode::Pencil ? "Pencil" : "Normal") << "\n";
    out << "selected_number=" << state.selected_number << "\n";
    out << "solution=" << encode_values(state.solution) << "\n";
    out << "givens=" << encode_values(state.givens) << "\n";
    out << "entries=" << encode_values(state.entries) << "\n";
    out << "pencils=" << encode_values(state.pencils) << "\n";
    out.close();

    if (!out) return false;
    if (std::rename(tmp.c_str(), path_.c_str()) != 0) {
        std::remove(tmp.c_str());
        return false;
    }
    return true;
}

} // namespace ksudoku
