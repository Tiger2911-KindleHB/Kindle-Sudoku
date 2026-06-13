#pragma once

#include "game_state.h"
#include "sudoku_engine.h"

#include <gtk/gtk.h>

#include <string>
#include <vector>

namespace ksudoku {

struct Rect {
    double x = 0;
    double y = 0;
    double w = 0;
    double h = 0;

    bool contains(double px, double py) const {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    }
};

class SudokuApp {
public:
    SudokuApp(int argc, char** argv, std::string app_dir);
    int run();

private:
    enum class ModalMode {
        None,
        ChooseSize,
        ChooseDifficulty
    };

    GtkWidget* window_ = nullptr;
    GtkWidget* canvas_ = nullptr;

    std::string app_dir_;
    SaveManager save_;
    SudokuEngine engine_;
    GameState state_;

    ModalMode modal_ = ModalMode::None;
    int pending_size_ = 9;
    Rect new_button_;
    Rect exit_button_;
    Rect board_rect_;
    Rect normal_button_;
    Rect pencil_button_;
    std::vector<Rect> number_buttons_;
    std::vector<int> number_button_values_;
    std::vector<Rect> modal_buttons_;
    std::vector<int> modal_values_;

    void initialize_state();
    void create_window();
    void save_now();
    void new_game(int size, Difficulty difficulty);

    void draw(cairo_t* cr, int width, int height);
    void draw_top_bar(cairo_t* cr, int width, int height);
    void draw_board(cairo_t* cr, int width, int height);
    void draw_controls(cairo_t* cr, int width, int height);
    void draw_modal(cairo_t* cr, int width, int height);
    void draw_completion_banner(cairo_t* cr, int width, int height);

    guint32 last_tap_time_ = 0;
    double last_tap_x_ = -10000.0;
    double last_tap_y_ = -10000.0;

    void handle_tap(double x, double y);
    void handle_modal_tap(double x, double y);
    void handle_board_tap(double x, double y);
    bool should_process_button_event(GdkEventButton* event);
    void queue_redraw();

    static gboolean on_expose(GtkWidget* widget, GdkEventExpose* event, gpointer data);
    static gboolean on_button_press(GtkWidget* widget, GdkEventButton* event, gpointer data);
    static gboolean on_button_release(GtkWidget* widget, GdkEventButton* event, gpointer data);
    static gboolean on_delete(GtkWidget* widget, GdkEvent* event, gpointer data);
};

} // namespace ksudoku
