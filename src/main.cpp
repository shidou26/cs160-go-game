#include <raylib.h>

#include "../include/board.h"
#include "../include/ui.h"
#include "../include/helper.h"

int main() {
    InitWindow(1280, 960, "The Game of Go");
    SetTargetFPS(60);

    Board board(BOARD_SIZE);
    UI ui;

    while (!WindowShouldClose()) {
        // --- CẬP NHẬT LOGIC & BẮT CLICK CHUỘT ---
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse_pos = GetMousePosition();
            int row, col;

            // 1. Kiểm tra xem người chơi có nháy chuột vào các nút Option trên UI không
            if (!ui.handle_option_click(mouse_pos, board)) {
                // 2. Kiểm tra xem click chuột có trúng bàn cờ không
                if (ui.screen_to_grid(mouse_pos, board, row, col)) {
                    // 3. Thử đặt quân cờ vào vị trí đó trong Board
                    if (board.place_stone(row, col)) {

                    }
                }
            }
        }

        BeginDrawing();
        ClearBackground(ui.get_background_color());
        ui.draw(board);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
