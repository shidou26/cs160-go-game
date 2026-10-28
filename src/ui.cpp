#include "ui.h"
#include "../include/helper.h"

UI::UI() {
    start_x = START_X;
    start_y = START_Y;
    padding = PADDING;
    grid_width = DEFAULT_GRID_WIDTH;

    bg_color = (Color){ 20, 24, 35, 255 };
    board_wood = (Color){ 219, 169, 99, 255 };
    line_color = (Color){ 60, 40, 20, 255 };
    board_border = (Color){ 180, 133, 78, 255 };

    // Khởi tạo vị trí các nút Option trên giao diện
    btn_size9  = { { 260, 25, 75, 36 }, "9x9" };
    btn_size13 = { { 345, 25, 85, 36 }, "13x13" };
    btn_size19 = { { 440, 25, 85, 36 }, "19x19" };
    btn_undo   = { { 555, 25, 95, 36 }, "Undo" };
    btn_reset  = { { 660, 25, 100, 36 }, "Reset" };
}

Color UI::get_background_color() const {
    return bg_color;
}

int UI::get_cell_size(const Board &board) const {
    int board_size = board.get_size();
    return (board_size > 1) ? (grid_width / (board_size - 1)) : grid_width;
}

void UI::draw_button(const OptionButton &btn, Vector2 mouse_pos, bool is_active) const {
    bool is_hovered = CheckCollisionPointRec(mouse_pos, btn.bounds);

    Color fill = is_active
        ? (Color){ 219, 169, 99, 255 }
        : (is_hovered ? (Color){ 60, 70, 95, 255 } : (Color){ 35, 42, 60, 255 });
    Color text_col = is_active ? (Color){ 20, 24, 35, 255 } : RAYWHITE;
    Color border_col = is_active ? RAYWHITE : (Color){ 100, 115, 140, 255 };

    DrawRectangleRounded(btn.bounds, 0.25f, 6, fill);
    DrawRectangleRoundedLines(btn.bounds, 0.25f, 6, border_col);

    int font_size = 18;
    int text_weight = MeasureText(btn.label, font_size);
    DrawText(
        btn.label,
        (int)(btn.bounds.x + (btn.bounds.width - text_weight) / 2),
        (int)(btn.bounds.y + (btn.bounds.height - font_size) / 2),
        font_size,
        text_col
    );
}

void UI::draw(const Board &board, int current_player) const {
    int board_size = board.get_size();
    int cell_size = get_cell_size(board);
    int actual_grid_width = (board_size - 1) * cell_size;
    Vector2 mouse_pos = GetMousePosition();

    // 1. Vẽ thông tin lượt chơi và các nút Option
    if (current_player == 1) {
        DrawText("Turn: BLACK", 30, 33, 22, RAYWHITE);
    } else {
        DrawText("Turn: WHITE", 30, 33, 22, RAYWHITE);
    }

    draw_button(btn_size9,  mouse_pos, board_size == 9);
    draw_button(btn_size13, mouse_pos, board_size == 13);
    draw_button(btn_size19, mouse_pos, board_size == 19);
    draw_button(btn_undo,   mouse_pos, false);
    draw_button(btn_reset,  mouse_pos, false);

    // 2. Vẽ mặt bàn cờ gỗ
    Rectangle board_rect = { 
        (float)(start_x - padding), 
        (float)(start_y - padding), 
        (float)(actual_grid_width + padding * 2), 
        (float)(actual_grid_width + padding * 2) 
    };

    DrawRectangleRounded(board_rect, 0.05f, 4, board_wood); 

    // 3. Vẽ các đường kẻ lưới
    for (int i = 0; i < board_size; i++) {
        DrawLine(start_x + i * cell_size, start_y, start_x + i * cell_size, start_y + actual_grid_width, line_color);
        DrawLine(start_x, start_y + i * cell_size, start_x + actual_grid_width, start_y + i * cell_size, line_color);
    }

    // 4. Vẽ các quân cờ dựa trên dữ liệu đọc từ Board
    int radius = (int)(cell_size * 0.4f);
    Color black_stone_color = { 0, 0, 0, 255 };

    for (int r = 0; r < board_size; r++) {
        for (int c = 0; c < board_size; c++) {
            int cell_state = board.get_cell(r, c);
            if (cell_state != 0) {
                int stone_x = start_x + c * cell_size;
                int stone_y = start_y + r * cell_size;

                if (cell_state == 1) {
                    // Quân Đen
                    DrawCircle(stone_x, stone_y, (float)radius, black_stone_color);
                } else if (cell_state == 2) {
                    // Quân Trắng + viền xám nhẹ để nổi bật trên mặt gỗ
                    DrawCircle(stone_x, stone_y, (float)radius, RAYWHITE);
                    DrawCircleLines(stone_x, stone_y, (float)radius, DARKGRAY);
                }
            }
        }
    }
}

bool UI::screen_to_grid(Vector2 mouse_pos, const Board &board, int &row, int &col) const {
    int board_size = board.get_size();
    int cell_size = get_cell_size(board);

    col = (int)((mouse_pos.x - start_x + cell_size / 2.0f) / cell_size);
    row = (int)((mouse_pos.y - start_y + cell_size / 2.0f) / cell_size);

    return (row >= 0 && row < board_size && col >= 0 && col < board_size);
}

bool UI::handle_option_click(Vector2 mouse_pos, Board &board, int &current_player) {
    if (CheckCollisionPointRec(mouse_pos, btn_size9.bounds)) {
        board.reset(9);
        current_player = 1;
        return true;
    }
    if (CheckCollisionPointRec(mouse_pos, btn_size13.bounds)) {
        board.reset(13);
        current_player = 1;
        return true;
    }
    if (CheckCollisionPointRec(mouse_pos, btn_size19.bounds)) {
        board.reset(19);
        current_player = 1;
        return true;
    }
    if (CheckCollisionPointRec(mouse_pos, btn_undo.bounds)) {
        board.undo_move(true);
        current_player = board.get_turn() ? 2 : 1;
        return true;
    }
    if (CheckCollisionPointRec(mouse_pos, btn_reset.bounds)) {
        board.reset();
        current_player = 1;
        return true;
    }
    return false;
}
