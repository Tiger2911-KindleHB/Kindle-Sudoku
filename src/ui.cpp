#include "ui.h"

#include <cairo.h>
#include <pango/pangocairo.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <stdexcept>

namespace ksudoku {

namespace {

void set_gray(cairo_t* cr, double value) {
    cairo_set_source_rgb(cr, value, value, value);
}

void fill_rect(cairo_t* cr, const Rect& r, double gray) {
    set_gray(cr, gray);
    cairo_rectangle(cr, r.x, r.y, r.w, r.h);
    cairo_fill(cr);
}

void stroke_rect(cairo_t* cr, const Rect& r, double gray, double width) {
    set_gray(cr, gray);
    cairo_set_line_width(cr, width);
    cairo_rectangle(cr, r.x, r.y, r.w, r.h);
    cairo_stroke(cr);
}

void draw_text(cairo_t* cr,
               const std::string& text,
               const Rect& bounds,
               double size,
               bool bold,
               double gray,
               PangoAlignment alignment = PANGO_ALIGN_CENTER) {
    PangoLayout* layout = pango_cairo_create_layout(cr);
    PangoFontDescription* desc = pango_font_description_new();
    pango_font_description_set_family(desc, "Sans");
    pango_font_description_set_absolute_size(desc, size * PANGO_SCALE);
    pango_font_description_set_weight(desc, bold ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL);
    pango_layout_set_font_description(layout, desc);
    pango_layout_set_text(layout, text.c_str(), -1);
    pango_layout_set_alignment(layout, alignment);
    pango_layout_set_width(layout, static_cast<int>(bounds.w * PANGO_SCALE));
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);

    int text_w = 0;
    int text_h = 0;
    pango_layout_get_pixel_size(layout, &text_w, &text_h);

    // Pango performs the horizontal alignment inside the layout width we set
    // above.  Do not also offset the Cairo origin for centered/right text, or
    // the text is effectively aligned twice and appears pushed toward the
    // right edge of buttons/cells on the Kindle.
    const double x = bounds.x;
    const double y = bounds.y + (bounds.h - text_h) / 2.0;

    set_gray(cr, gray);
    cairo_move_to(cr, std::floor(x), std::floor(y));
    pango_cairo_show_layout(cr, layout);

    pango_font_description_free(desc);
    g_object_unref(layout);
}

std::string digit_label(int value) {
    if (value == 0) return "\u25A1"; // White square erase tool.
    if (value < 10) return std::string(1, static_cast<char>('0' + value));
    return std::string(1, static_cast<char>('A' + value - 10));
}

std::string time_label(long seconds) {
    if (seconds < 0) seconds = 0;
    const long minutes = seconds / 60;
    const long hours = minutes / 60;
    std::ostringstream out;
    if (hours > 0) out << hours << "h " << (minutes % 60) << "m";
    else out << minutes << "m";
    return out.str();
}

void append_app_log(const std::string& app_dir, const std::string& message) {
    const std::string path = app_dir + "/data/app.log";
    FILE* fp = std::fopen(path.c_str(), "a");
    if (!fp) return;
    const std::time_t now = std::time(nullptr);
    std::fprintf(fp, "%ld %s\n", static_cast<long>(now), message.c_str());
    std::fclose(fp);
}

int kindle_input_event_mask() {
    return GDK_BUTTON_PRESS_MASK |
           GDK_BUTTON_RELEASE_MASK |
           GDK_POINTER_MOTION_MASK |
           GDK_ENTER_NOTIFY_MASK |
           GDK_LEAVE_NOTIFY_MASK |
           GDK_FOCUS_CHANGE_MASK |
           GDK_KEY_PRESS_MASK |
           GDK_KEY_RELEASE_MASK;
}

gboolean raise_window_once(gpointer data) {
    GtkWidget* window = GTK_WIDGET(data);
    if (!window || !GTK_IS_WINDOW(window)) return FALSE;

    gtk_window_fullscreen(GTK_WINDOW(window));
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
    gtk_window_present(GTK_WINDOW(window));

    GdkWindow* gdk_window = gtk_widget_get_window(window);
    if (gdk_window) {
        gdk_window_raise(gdk_window);
        gdk_window_focus(gdk_window, GDK_CURRENT_TIME);
    }
    return FALSE;
}

} // namespace

SudokuApp::SudokuApp(int argc, char** argv, std::string app_dir)
    : app_dir_(std::move(app_dir)),
      save_(app_dir_ + "/data/save.dat") {
    append_app_log(app_dir_, "constructor: before gtk_init");
    gtk_init(&argc, &argv);
    append_app_log(app_dir_, "constructor: after gtk_init");
    initialize_state();
    create_window();
    append_app_log(app_dir_, "constructor: window created");
}

int SudokuApp::run() {
    append_app_log(app_dir_, "run: show window");
    gtk_widget_show_all(window_);
    gtk_window_fullscreen(GTK_WINDOW(window_));
    gtk_window_set_keep_above(GTK_WINDOW(window_), TRUE);
    gtk_window_present(GTK_WINDOW(window_));
    raise_window_once(window_);

    // Kindle's home framework can steal focus immediately after KUAL starts a
    // native process. Raise the window a few times after mapping so the game is
    // visible instead of running behind the Kindle home screen.
    g_timeout_add(250, raise_window_once, window_);
    g_timeout_add(750, raise_window_once, window_);
    g_timeout_add(1500, raise_window_once, window_);

    configure_input_widgets();

    append_app_log(app_dir_, "run: enter gtk_main");
    gtk_main();
    append_app_log(app_dir_, "run: exit gtk_main");
    return 0;
}

void SudokuApp::initialize_state() {
    if (!save_.load(state_)) {
        new_game(9, Difficulty::Normal);
    }
    state_.refresh_incorrect_flags();
}

void SudokuApp::create_window() {
    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_), "Kindle Sudoku");
    gtk_window_set_decorated(GTK_WINDOW(window_), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(window_), TRUE);
    gtk_window_set_position(GTK_WINDOW(window_), GTK_WIN_POS_CENTER);
    gtk_window_set_keep_above(GTK_WINDOW(window_), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(window_), gdk_screen_width(), gdk_screen_height());

    gtk_widget_set_can_focus(window_, TRUE);
    gtk_widget_add_events(window_, kindle_input_event_mask());

    event_box_ = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(event_box_), FALSE);
    gtk_widget_set_can_focus(event_box_, TRUE);
    gtk_widget_add_events(event_box_, kindle_input_event_mask());

    canvas_ = gtk_drawing_area_new();
    gtk_widget_set_size_request(canvas_, gdk_screen_width(), gdk_screen_height());
    gtk_widget_set_app_paintable(canvas_, TRUE);
    gtk_widget_set_can_focus(canvas_, TRUE);
    gtk_widget_add_events(canvas_, kindle_input_event_mask());

    gtk_container_add(GTK_CONTAINER(window_), event_box_);
    gtk_container_add(GTK_CONTAINER(event_box_), canvas_);

    g_signal_connect(G_OBJECT(canvas_), "expose-event", G_CALLBACK(SudokuApp::on_expose), this);

    // Kindle touch can arrive at either the drawing surface or its parent
    // container depending on firmware/input routing. Listen at all three
    // levels and collapse duplicates in should_process_button_event().
    g_signal_connect(G_OBJECT(canvas_), "button-press-event", G_CALLBACK(SudokuApp::on_button_press), this);
    g_signal_connect(G_OBJECT(canvas_), "button-release-event", G_CALLBACK(SudokuApp::on_button_release), this);
    g_signal_connect(G_OBJECT(event_box_), "button-press-event", G_CALLBACK(SudokuApp::on_root_button_press), this);
    g_signal_connect(G_OBJECT(event_box_), "button-release-event", G_CALLBACK(SudokuApp::on_root_button_release), this);
    g_signal_connect(G_OBJECT(window_), "button-press-event", G_CALLBACK(SudokuApp::on_root_button_press), this);
    g_signal_connect(G_OBJECT(window_), "button-release-event", G_CALLBACK(SudokuApp::on_root_button_release), this);

    g_signal_connect(G_OBJECT(window_), "delete-event", G_CALLBACK(SudokuApp::on_delete), this);
    g_signal_connect(G_OBJECT(window_), "destroy", G_CALLBACK(gtk_main_quit), nullptr);
}

void SudokuApp::configure_input_widgets() {
    GtkWidget* widgets[] = {window_, event_box_, canvas_};
    for (GtkWidget* widget : widgets) {
        if (!widget) continue;
        gtk_widget_add_events(widget, kindle_input_event_mask());
        GdkWindow* gdk_window = gtk_widget_get_window(widget);
        if (gdk_window) {
            gdk_window_set_events(gdk_window, gdk_window_get_events(gdk_window) | kindle_input_event_mask());
        }
    }
    if (event_box_) gtk_widget_grab_focus(event_box_);
    if (canvas_) gtk_widget_grab_focus(canvas_);
    append_app_log(app_dir_, "input: configured window/eventbox/canvas event masks");
}

void SudokuApp::save_now() {
    state_.refresh_incorrect_flags();
    save_.save(state_);
}

void SudokuApp::new_game(int size, Difficulty difficulty) {
    GeneratedPuzzle puzzle = engine_.generate(size, difficulty);
    state_.reset_from_puzzle(puzzle);
    save_now();
}

void SudokuApp::draw(cairo_t* cr, int width, int height) {
    static bool logged_first_draw = false;
    if (!logged_first_draw) {
        logged_first_draw = true;
        append_app_log(app_dir_, "draw: first expose");
    }
    fill_rect(cr, Rect{0, 0, static_cast<double>(width), static_cast<double>(height)}, 1.0);
    draw_top_bar(cr, width, height);
    draw_board(cr, width, height);
    draw_controls(cr, width, height);
    if (state_.is_complete()) {
        draw_completion_banner(cr, width, height);
    }
    if (modal_ != ModalMode::None) {
        draw_modal(cr, width, height);
    }
}

void SudokuApp::draw_top_bar(cairo_t* cr, int width, int height) {
    const double button_h = std::max(46.0, std::min(68.0, height * 0.075));
    const double button_w = std::max(88.0, std::min(140.0, width * 0.16));
    const double margin = std::max(10.0, width * 0.018);

    new_button_ = Rect{margin, margin, button_w, button_h};
    exit_button_ = Rect{width - margin - button_w, margin, button_w, button_h};

    stroke_rect(cr, new_button_, 0.0, 3.0);
    stroke_rect(cr, exit_button_, 0.0, 3.0);
    draw_text(cr, "New", new_button_, button_h * 0.43, true, 0.0);
    draw_text(cr, "Exit", exit_button_, button_h * 0.43, true, 0.0);

    std::ostringstream title;
    title << state_.spec.label << "  " << difficulty_to_string(state_.difficulty)
          << "  " << time_label(state_.elapsed_seconds + (std::time(nullptr) - state_.session_started));
    Rect title_rect{new_button_.x + new_button_.w + 8, margin, exit_button_.x - new_button_.x - new_button_.w - 16, button_h};
    draw_text(cr, title.str(), title_rect, std::max(20.0, button_h * 0.34), true, 0.0);
}

void SudokuApp::draw_board(cairo_t* cr, int width, int height) {
    const int n = state_.spec.size;
    const double top_reserved = std::max(74.0, height * 0.095);
    const double bottom_reserved = std::max(158.0, height * 0.22);
    double max_board = std::min(width * 0.94, height - top_reserved - bottom_reserved);
    max_board = std::max(220.0, max_board);
    double cell = std::floor(max_board / n);
    if (cell < 28.0) cell = max_board / n;
    const double board_size = cell * n;
    board_rect_ = Rect{std::floor((width - board_size) / 2.0), std::floor(top_reserved), board_size, board_size};

    stroke_rect(cr, board_rect_, 0.0, 4.0);

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            const int idx = r * n + c;
            Rect cell_rect{board_rect_.x + c * cell, board_rect_.y + r * cell, cell, cell};
            const int visible = state_.visible_value(idx);
            const bool highlighted = state_.highlight_number != 0 && visible == state_.highlight_number;
            if (highlighted) {
                fill_rect(cr, cell_rect, 0.0);
            }

            if (visible != 0) {
                const bool given = state_.is_given(idx);
                const double text_gray = highlighted ? 1.0 : (given ? 0.0 : 0.18);
                draw_text(cr, digit_label(visible), cell_rect, cell * 0.58, given, text_gray);

                if (idx < static_cast<int>(state_.incorrect.size()) && state_.incorrect[idx]) {
                    set_gray(cr, highlighted ? 1.0 : 0.0);
                    cairo_set_line_width(cr, std::max(3.0, cell * 0.055));
                    const double pad = cell * 0.22;
                    cairo_move_to(cr, cell_rect.x + pad, cell_rect.y + pad);
                    cairo_line_to(cr, cell_rect.x + cell - pad, cell_rect.y + cell - pad);
                    cairo_move_to(cr, cell_rect.x + cell - pad, cell_rect.y + pad);
                    cairo_line_to(cr, cell_rect.x + pad, cell_rect.y + cell - pad);
                    cairo_stroke(cr);
                }
            } else if (state_.pencils[idx] != 0) {
                Rect pencil_rect{cell_rect.x + cell * 0.52, cell_rect.y + cell * 0.06, cell * 0.42, cell * 0.34};
                draw_text(cr, digit_label(state_.pencils[idx]), pencil_rect, cell * 0.25, false, 0.42, PANGO_ALIGN_RIGHT);
            }
        }
    }

    for (int i = 1; i < n; ++i) {
        const bool major_col = (i % state_.spec.box_cols == 0);
        const bool major_row = (i % state_.spec.box_rows == 0);
        cairo_set_line_width(cr, major_col ? 4.0 : 1.4);
        set_gray(cr, 0.0);
        cairo_move_to(cr, board_rect_.x + i * cell, board_rect_.y);
        cairo_line_to(cr, board_rect_.x + i * cell, board_rect_.y + board_rect_.h);
        cairo_stroke(cr);

        cairo_set_line_width(cr, major_row ? 4.0 : 1.4);
        cairo_move_to(cr, board_rect_.x, board_rect_.y + i * cell);
        cairo_line_to(cr, board_rect_.x + board_rect_.w, board_rect_.y + i * cell);
        cairo_stroke(cr);
    }
    stroke_rect(cr, board_rect_, 0.0, 4.0);
}

void SudokuApp::draw_controls(cairo_t* cr, int width, int height) {
    const double bottom_margin = std::max(12.0, height * 0.02);
    const double number_h = std::max(48.0, std::min(72.0, height * 0.075));
    const double mode_h = std::max(46.0, std::min(66.0, height * 0.07));
    const double gap = std::max(8.0, width * 0.012);

    normal_button_ = Rect{std::max(10.0, width * 0.05), height - bottom_margin - number_h - gap - mode_h,
                          width * 0.40, mode_h};
    pencil_button_ = Rect{width - normal_button_.x - normal_button_.w, normal_button_.y,
                          normal_button_.w, mode_h};

    auto draw_mode_button = [&](const Rect& rect, const std::string& label, bool active) {
        if (active) fill_rect(cr, rect, 0.0);
        stroke_rect(cr, rect, 0.0, 3.0);
        draw_text(cr, label, rect, mode_h * 0.38, true, active ? 1.0 : 0.0);
    };
    draw_mode_button(normal_button_, "Normal", state_.mode == EntryMode::Normal);
    draw_mode_button(pencil_button_, "Pencil", state_.mode == EntryMode::Pencil);

    number_buttons_.clear();
    number_button_values_.clear();

    std::vector<int> visible_numbers;
    visible_numbers.push_back(0);
    for (int value = 1; value <= state_.spec.size; ++value) {
        if (!state_.number_is_complete(value)) visible_numbers.push_back(value);
    }

    const int count = static_cast<int>(visible_numbers.size());
    const double candidate_w = std::min(70.0, (width * 0.92 - gap * (count - 1)) / count);
    const double button_w = std::max(42.0, candidate_w);
    const double total_w = count * button_w + (count - 1) * gap;
    double x = std::floor((width - total_w) / 2.0);
    const double y = height - bottom_margin - number_h;

    for (int value : visible_numbers) {
        Rect r{x, y, button_w, number_h};
        const bool active = value == state_.selected_number;
        if (active) fill_rect(cr, r, 0.0);
        stroke_rect(cr, r, 0.0, 3.0);
        draw_text(cr, digit_label(value), r, number_h * 0.48, true, active ? 1.0 : 0.0);
        number_buttons_.push_back(r);
        number_button_values_.push_back(value);
        x += button_w + gap;
    }
}

void SudokuApp::draw_modal(cairo_t* cr, int width, int height) {
    modal_buttons_.clear();
    modal_values_.clear();

    fill_rect(cr, Rect{0, 0, static_cast<double>(width), static_cast<double>(height)}, 0.78);
    const double box_w = std::min(width * 0.86, 620.0);
    const double box_h = modal_ == ModalMode::ChooseSize ? std::min(height * 0.64, 430.0) : std::min(height * 0.72, 510.0);
    Rect box{std::floor((width - box_w) / 2.0), std::floor((height - box_h) / 2.0), box_w, box_h};
    fill_rect(cr, box, 1.0);
    stroke_rect(cr, box, 0.0, 5.0);

    Rect title{box.x + 20, box.y + 18, box.w - 40, 56};
    draw_text(cr, modal_ == ModalMode::ChooseSize ? "New Puzzle: Size" : "New Puzzle: Difficulty",
              title, 32.0, true, 0.0);

    const double button_h = 62.0;
    const double button_gap = 16.0;
    const double button_w = box.w - 80.0;
    double y = title.y + title.h + 16.0;

    if (modal_ == ModalMode::ChooseSize) {
        const char* labels[] = {"4x4 Mini", "6x6 Small", "9x9 Classic"};
        const int values[] = {4, 6, 9};
        for (int i = 0; i < 3; ++i) {
            Rect r{box.x + 40, y, button_w, button_h};
            stroke_rect(cr, r, 0.0, 3.0);
            draw_text(cr, labels[i], r, 30.0, true, 0.0);
            modal_buttons_.push_back(r);
            modal_values_.push_back(values[i]);
            y += button_h + button_gap;
        }
    } else {
        const char* labels[] = {"Easy", "Normal", "Hard", "Expert"};
        const int values[] = {0, 1, 2, 3};
        for (int i = 0; i < 4; ++i) {
            Rect r{box.x + 40, y, button_w, button_h};
            stroke_rect(cr, r, 0.0, 3.0);
            draw_text(cr, labels[i], r, 30.0, true, 0.0);
            modal_buttons_.push_back(r);
            modal_values_.push_back(values[i]);
            y += button_h + button_gap;
        }
    }

    Rect cancel{box.x + 40, box.y + box.h - 76, button_w, 52};
    stroke_rect(cr, cancel, 0.0, 2.0);
    draw_text(cr, "Cancel", cancel, 24.0, false, 0.0);
    modal_buttons_.push_back(cancel);
    modal_values_.push_back(-1);
}

void SudokuApp::draw_completion_banner(cairo_t* cr, int width, int height) {
    const double banner_w = std::min(width * 0.70, 520.0);
    const double banner_h = 62.0;
    Rect r{std::floor((width - banner_w) / 2.0), board_rect_.y + board_rect_.h + 8, banner_w, banner_h};
    fill_rect(cr, r, 1.0);
    stroke_rect(cr, r, 0.0, 4.0);
    draw_text(cr, "Puzzle Complete", r, 30.0, true, 0.0);
}

void SudokuApp::handle_tap(double x, double y) {
    if (modal_ != ModalMode::None) {
        handle_modal_tap(x, y);
        return;
    }

    if (new_button_.contains(x, y)) {
        modal_ = ModalMode::ChooseSize;
        state_.highlight_number = 0;
        queue_redraw();
        return;
    }

    if (exit_button_.contains(x, y)) {
        save_now();
        gtk_main_quit();
        return;
    }

    if (normal_button_.contains(x, y)) {
        state_.mode = EntryMode::Normal;
        state_.highlight_number = 0;
        save_now();
        queue_redraw();
        return;
    }

    if (pencil_button_.contains(x, y)) {
        state_.mode = EntryMode::Pencil;
        state_.highlight_number = 0;
        save_now();
        queue_redraw();
        return;
    }

    for (std::size_t i = 0; i < number_buttons_.size(); ++i) {
        if (number_buttons_[i].contains(x, y)) {
            state_.selected_number = number_button_values_[i];
            state_.highlight_number = 0;
            save_now();
            queue_redraw();
            return;
        }
    }

    if (board_rect_.contains(x, y)) {
        handle_board_tap(x, y);
        return;
    }

    state_.highlight_number = 0;
    state_.selected_cell = -1;
    queue_redraw();
}

void SudokuApp::handle_modal_tap(double x, double y) {
    for (std::size_t i = 0; i < modal_buttons_.size(); ++i) {
        if (!modal_buttons_[i].contains(x, y)) continue;
        const int value = modal_values_[i];
        if (value < 0) {
            modal_ = ModalMode::None;
            queue_redraw();
            return;
        }

        if (modal_ == ModalMode::ChooseSize) {
            pending_size_ = value;
            modal_ = ModalMode::ChooseDifficulty;
            queue_redraw();
            return;
        }

        Difficulty difficulty = Difficulty::Normal;
        if (value == 0) difficulty = Difficulty::Easy;
        else if (value == 1) difficulty = Difficulty::Normal;
        else if (value == 2) difficulty = Difficulty::Hard;
        else if (value == 3) difficulty = Difficulty::Expert;
        new_game(pending_size_, difficulty);
        modal_ = ModalMode::None;
        queue_redraw();
        return;
    }
}

void SudokuApp::handle_board_tap(double x, double y) {
    const int n = state_.spec.size;
    const double cell = board_rect_.w / n;
    const int col = std::min(n - 1, std::max(0, static_cast<int>((x - board_rect_.x) / cell)));
    const int row = std::min(n - 1, std::max(0, static_cast<int>((y - board_rect_.y) / cell)));
    const int idx = row * n + col;
    state_.selected_cell = idx;

    if (state_.is_given(idx)) {
        state_.highlight_number = state_.givens[idx];
        queue_redraw();
        return;
    }

    if (state_.selected_number == 0) {
        state_.entries[idx] = 0;
        state_.pencils[idx] = 0;
        state_.highlight_number = 0;
        state_.refresh_incorrect_flags();
        save_now();
        queue_redraw();
        return;
    }

    const int visible = state_.visible_value(idx);
    if (visible != 0) {
        state_.highlight_number = visible;
        queue_redraw();
        return;
    }

    if (state_.mode == EntryMode::Pencil) {
        state_.pencils[idx] = state_.selected_number;
        state_.highlight_number = 0;
    } else {
        state_.entries[idx] = state_.selected_number;
        state_.pencils[idx] = 0;
        state_.refresh_incorrect_flags();
        if (!state_.incorrect[idx]) {
            state_.highlight_number = state_.selected_number;
        } else {
            state_.highlight_number = 0;
        }
    }

    if (state_.selected_number > 0 && state_.number_is_complete(state_.selected_number)) {
        state_.selected_number = 0;
    }
    save_now();
    queue_redraw();
}

void SudokuApp::log_button_event(const char* source, const char* phase, GdkEventButton* event) {
    if (!event) {
        append_app_log(app_dir_, std::string("input: ") + source + " " + phase + " null-event");
        return;
    }
    char buffer[256];
    std::snprintf(buffer, sizeof(buffer),
                  "input: %s %s type=%d button=%u x=%.1f y=%.1f xroot=%.1f yroot=%.1f time=%u",
                  source, phase, static_cast<int>(event->type), event->button,
                  event->x, event->y, event->x_root, event->y_root, event->time);
    append_app_log(app_dir_, buffer);
}

bool SudokuApp::should_process_button_event(GdkEventButton* event) {
    if (!event) return false;

    // Some Kindle touch stacks report the primary tap as button 0 instead of
    // desktop mouse button 1. Accept both; ignore only clearly secondary
    // buttons.
    if (event->button > 1) return false;

    const double dx = event->x - last_tap_x_;
    const double dy = event->y - last_tap_y_;
    const double dist2 = dx * dx + dy * dy;

    // Some Kindle/X input stacks emit both press and release for the same tap,
    // while others only deliver one of them to GTK.  Accept either event, but
    // collapse a press/release pair into one logical tap so modal buttons and
    // board cells do not process twice.
    if (last_tap_time_ != 0 && event->time >= last_tap_time_ &&
        event->time - last_tap_time_ < 700 && dist2 < 144.0) {
        return false;
    }

    last_tap_time_ = event->time;
    last_tap_x_ = event->x;
    last_tap_y_ = event->y;
    return true;
}

void SudokuApp::queue_redraw() {
    if (canvas_) gtk_widget_queue_draw(canvas_);
}

gboolean SudokuApp::on_expose(GtkWidget* widget, GdkEventExpose*, gpointer data) {
    SudokuApp* app = static_cast<SudokuApp*>(data);
    cairo_t* cr = gdk_cairo_create(widget->window);
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    app->draw(cr, allocation.width, allocation.height);
    cairo_destroy(cr);
    return TRUE;
}

gboolean SudokuApp::on_button_press(GtkWidget*, GdkEventButton* event, gpointer data) {
    SudokuApp* app = static_cast<SudokuApp*>(data);
    app->log_button_event("canvas", "press", event);
    if (!app->should_process_button_event(event)) return FALSE;
    app->handle_tap(event->x, event->y);
    return TRUE;
}

gboolean SudokuApp::on_button_release(GtkWidget*, GdkEventButton* event, gpointer data) {
    SudokuApp* app = static_cast<SudokuApp*>(data);
    app->log_button_event("canvas", "release", event);
    if (!app->should_process_button_event(event)) return FALSE;
    app->handle_tap(event->x, event->y);
    return TRUE;
}

gboolean SudokuApp::on_root_button_press(GtkWidget*, GdkEventButton* event, gpointer data) {
    SudokuApp* app = static_cast<SudokuApp*>(data);
    app->log_button_event("root", "press", event);
    if (!app->should_process_button_event(event)) return FALSE;
    app->handle_tap(event->x, event->y);
    return TRUE;
}

gboolean SudokuApp::on_root_button_release(GtkWidget*, GdkEventButton* event, gpointer data) {
    SudokuApp* app = static_cast<SudokuApp*>(data);
    app->log_button_event("root", "release", event);
    if (!app->should_process_button_event(event)) return FALSE;
    app->handle_tap(event->x, event->y);
    return TRUE;
}

gboolean SudokuApp::on_delete(GtkWidget*, GdkEvent*, gpointer data) {
    SudokuApp* app = static_cast<SudokuApp*>(data);
    append_app_log(app->app_dir_, "window delete event");
    app->save_now();
    return FALSE;
}

} // namespace ksudoku
