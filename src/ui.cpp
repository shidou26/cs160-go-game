#include "ui.h"

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
    int bSize = board.get_size();
    return (bSize > 1) ? (grid_width / (bSize - 1)) : grid_width;
}

void UI::draw_button(const OptionButton &btn, Vector2 mousePos, bool isActive) const {
    bool isHovered = CheckCollisionPointRec(mousePos, btn.bounds);

    Color fill = isActive
        ? (Color){ 219, 169, 99, 255 }
        : (isHovered ? (Color){ 60, 70, 95, 255 } : (Color){ 35, 42, 60, 255 });
    Color textCol = isActive ? (Color){ 20, 24, 35, 255 } : RAYWHITE;
    Color borderCol = isActive ? RAYWHITE : (Color){ 100, 115, 140, 255 };

    DrawRectangleRounded(btn.bounds, 0.25f, 6, fill);
    DrawRectangleRoundedLines(btn.bounds, 0.25f, 6, borderCol);

    int fontSize = 18;
    int textW = MeasureText(btn.label, fontSize);
    DrawText(
        btn.label,
        (int)(btn.bounds.x + (btn.bounds.width - textW) / 2),
        (int)(btn.bounds.y + (btn.bounds.height - fontSize) / 2),
        fontSize,
        textCol
    );
}

void UI::draw(const Board &board, int currentPlayer) const {
    int board_size = board.get_size();
    int cell_size = get_cell_size(board);
    int actual_grid_width = (board_size - 1) * cell_size;
    Vector2 mousePos = GetMousePosition();

    // 1. Vẽ thông tin lượt chơi và các nút Option
    if (currentPlayer == 1) {
        DrawText("Turn: BLACK", 30, 33, 22, RAYWHITE);
    } else {
        DrawText("Turn: WHITE", 30, 33, 22, RAYWHITE);
    }

    draw_button(btn_size9,  mousePos, board_size == 9);
    draw_button(btn_size13, mousePos, board_size == 13);
    draw_button(btn_size19, mousePos, board_size == 19);
    draw_button(btn_undo,   mousePos, false);
    draw_button(btn_reset,  mousePos, false);

    // 2. Vẽ mặt bàn cờ gỗ
    Rectangle boardRect = { 
        (float)(start_x - padding), 
        (float)(start_y - padding), 
        (float)(actual_grid_width + padding * 2), 
        (float)(actual_grid_width + padding * 2) 
    };

    DrawRectangleRounded(boardRect, 0.05f, 4, board_wood); 

    // 3. Vẽ các đường kẻ lưới
    for (int i = 0; i < board_size; i++) {
        DrawLine(start_x + i * cell_size, start_y, start_x + i * cell_size, start_y + actual_grid_width, line_color);
        DrawLine(start_x, start_y + i * cell_size, start_x + actual_grid_width, start_y + i * cell_size, line_color);
    }

    // 4. Vẽ các quân cờ dựa trên dữ liệu đọc từ Board
    int radius = (int)(cell_size * 0.4f);
    Color blackStoneColor = { 0, 0, 0, 255 };

    for (int r = 0; r < board_size; r++) {
        for (int c = 0; c < board_size; c++) {
            int cellState = board.get_cell(r, c);
            if (cellState != 0) {
                int stoneX = start_x + c * cell_size;
                int stoneY = start_y + r * cell_size;

                if (cellState == 1) {
                    // Quân Đen
                    DrawCircle(stoneX, stoneY, (float)radius, blackStoneColor);
                } else if (cellState == 2) {
                    // Quân Trắng + viền xám nhẹ để nổi bật trên mặt gỗ
                    DrawCircle(stoneX, stoneY, (float)radius, RAYWHITE);
                    DrawCircleLines(stoneX, stoneY, (float)radius, DARKGRAY);
                }
            }
        }
    }
}

bool UI::screen_to_grid(Vector2 mousePos, const Board &board, int &row, int &col) const {
    int bSize = board.get_size();
    int cellSize = get_cell_size(board);

    col = (int)((mousePos.x - start_x + cellSize / 2.0f) / cellSize);
    row = (int)((mousePos.y - start_y + cellSize / 2.0f) / cellSize);

    return (row >= 0 && row < bSize && col >= 0 && col < bSize);
}

bool UI::handle_option_click(Vector2 mousePos, Board &board, int &currentPlayer) {
    if (CheckCollisionPointRec(mousePos, btn_size9.bounds)) {
        board.reset(9);
        currentPlayer = 1;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, btn_size13.bounds)) {
        board.reset(13);
        currentPlayer = 1;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, btn_size19.bounds)) {
        board.reset(19);
        currentPlayer = 1;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, btn_undo.bounds)) {
        board.undo_move(true);
        currentPlayer = board.get_turn() ? 2 : 1;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, btn_reset.bounds)) {
        board.reset();
        currentPlayer = 1;
        return true;
    }
    return false;
}
