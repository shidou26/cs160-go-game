#pragma once

#include <raylib.h>

#include "../include/board.h"
#include "../include/helper.h"

struct OptionButton {
    Rectangle bounds;
    const char* label;
};

class UI {
private:
    int start_x;
    int start_y;
    int padding;
    int grid_width;

    Color bg_color;
    Color board_wood;
    Color line_color;
    Color board_border;

    //Option button during start menu
    OptionButton btn_new_game;
    OptionButton btn_load_game;
    OptionButton btn_config;

    //Option button during game 
    OptionButton btn_size9;
    OptionButton btn_size13;
    OptionButton btn_size19;
    OptionButton btn_undo;
    OptionButton btn_redo;
    OptionButton btn_reset;
    OptionButton btn_pass;
    OptionButton btn_save;
    OptionButton btn_back_from_play;

    //Option buttons for saving and loading
    OptionButton btn_save_slots[3];
    OptionButton btn_load_slots[3];
    OptionButton btn_back_save;
    OptionButton btn_back_load;
    OptionButton btn_confirm_overwrite;
    OptionButton btn_cancel_overwrite;
    int overwrite_slot = -1;

    //Option button during gameover menu
    OptionButton btn_back_from_end; // go back to start screen

    //Option button during config
    OptionButton btn_check;

    //Option button during choosing mode
    OptionButton btn_pvp;
    OptionButton btn_pve;

    //Option button during choose difficulty
    OptionButton btn_easy;
    OptionButton btn_medium;
    OptionButton btn_hard;


    int get_cell_size(const Board &board) const;
    void draw_button(const OptionButton &btn, Vector2 mouse_pos, bool is_active = false, int font_size = 18) const;

public:
    UI();

    Color get_background_color() const;

    /**
     * @brief Drawing
     * 
     * @param board 
     */
    void draw(Board &board);
    void draw_starting(const Board &board);
    void draw_ending(const Board &board);
    void draw_config();
    void draw_loading();
    void draw_saving();
    void draw_mode();
    void draw_difficulty();

    // Chuyển đổi tọa độ click chuột trên màn hình sang vị trí (row, col) của Board
    bool screen_to_grid(Vector2 mouse_pos, const Board &board, int &row, int &col) const;

    // Xử lý khi người chơi nháy chuột vào các nút Option trên giao diện
    bool handle_option_click(Vector2 mouse_pos, GamePhase &phase, Board &board);
};
