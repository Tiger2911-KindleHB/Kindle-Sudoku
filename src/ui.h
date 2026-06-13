#pragma once

#include "game_state.h"
#include "sudoku_engine.h"

#include <gtk/gtk.h>

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
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
    ~SudokuApp();
    int run();

private:
    enum class ModalMode {
        None,
        ChooseSize,
        ChooseDifficulty,
        Complete,
        Stats
    };

    struct StatsRecord {
        int size = 9;
        Difficulty difficulty = Difficulty::Normal;
        long seconds = 0;
        std::time_t completed_at = 0;
    };

    GtkWidget* window_ = nullptr;
    GtkWidget* event_box_ = nullptr;
    GtkWidget* canvas_ = nullptr;
    int screen_width_ = 0;
    int screen_height_ = 0;

    std::string app_dir_;
    SaveManager save_;
    SudokuEngine engine_;
    GameState state_;

    ModalMode modal_ = ModalMode::None;
    int pending_size_ = 9;
    Rect new_button_;
    Rect stats_button_;
    Rect exit_button_;
    Rect board_rect_;
    Rect normal_button_;
    Rect pencil_button_;
    std::vector<Rect> number_buttons_;
    std::vector<int> number_button_values_;
    std::vector<Rect> modal_buttons_;
    std::vector<int> modal_values_;
    std::vector<StatsRecord> stats_;
    int stats_selected_size_ = 9;
    bool completion_recorded_for_current_puzzle_ = false;

    void initialize_state();
    void load_stats();
    void save_stats();
    void record_completion_if_needed();
    long current_elapsed_seconds() const;
    void create_window();
    void configure_input_widgets();
    void start_evdev_reader();
    void stop_evdev_reader();
    void evdev_loop();
    void save_now();
    void new_game(int size, Difficulty difficulty);

    void draw(cairo_t* cr, int width, int height);
    void draw_top_bar(cairo_t* cr, int width, int height);
    void draw_board(cairo_t* cr, int width, int height);
    void draw_controls(cairo_t* cr, int width, int height);
    void draw_modal(cairo_t* cr, int width, int height);
    void draw_completion_popup(cairo_t* cr, int width, int height);
    void draw_stats_modal(cairo_t* cr, int width, int height);
    void draw_completion_banner(cairo_t* cr, int width, int height);

    guint32 last_tap_time_ = 0;
    double last_tap_x_ = -10000.0;
    double last_tap_y_ = -10000.0;

    std::atomic<bool> evdev_running_{false};
    std::thread evdev_thread_;
    std::mutex evdev_schedule_mutex_;
    long evdev_last_tap_ms_ = 0;

    void handle_tap(double x, double y);
    void handle_tap_from_evdev(double x, double y);
    void handle_modal_tap(double x, double y);
    void handle_board_tap(double x, double y);
    bool should_process_button_event(GdkEventButton* event);
    void log_button_event(const char* source, const char* phase, GdkEventButton* event);
    void queue_redraw();

    static gboolean on_evdev_tap_idle(gpointer data);
    static gboolean on_expose(GtkWidget* widget, GdkEventExpose* event, gpointer data);
    static gboolean on_button_press(GtkWidget* widget, GdkEventButton* event, gpointer data);
    static gboolean on_button_release(GtkWidget* widget, GdkEventButton* event, gpointer data);
    static gboolean on_root_button_press(GtkWidget* widget, GdkEventButton* event, gpointer data);
    static gboolean on_root_button_release(GtkWidget* widget, GdkEventButton* event, gpointer data);
    static gboolean on_delete(GtkWidget* widget, GdkEvent* event, gpointer data);
};

} // namespace ksudoku
