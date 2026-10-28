#pragma once

#include <raylib.h>

#include "board.h"

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

    // Các nút chọn Option trên giao diện
    OptionButton btn_size9;
    OptionButton btn_size13;
    OptionButton btn_size19;
    OptionButton btn_undo;
    OptionButton btn_reset;

    int get_cell_size(const Board &board) const;
    void draw_button(const OptionButton &btn, Vector2 mouse_pos, bool is_active = false) const;

public:
    UI();

    Color get_background_color() const;

    // Chỉ làm nhiệm vụ vẽ bàn cờ, quân cờ và các nút Option dựa trên dữ liệu từ Board
    void draw(const Board &board, int current_player) const;

    // Chuyển đổi tọa độ click chuột trên màn hình sang vị trí (row, col) của Board
    bool screen_to_grid(Vector2 mouse_pos, const Board &board, int &row, int &col) const;

    // Xử lý khi người chơi nháy chuột vào các nút Option trên giao diện
    bool handle_option_click(Vector2 mouse_pos, Board &board, int &current_player);
};
