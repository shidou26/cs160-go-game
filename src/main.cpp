#include <raylib.h>
#include "board.h"
#include "ui.h"

int main() {
    InitWindow(1280, 960, "Go Game");
    SetTargetFPS(60);

    // 1. Tạo đối tượng bàn cờ (quản lý grid[i][j] & logic) và đối tượng giao diện (UI)
    Board board(9);
    UI ui;
    int current_player = 1;

    while (!WindowShouldClose()) {
        // --- CẬP NHẬT LOGIC & BẮT CLICK CHUỘT ---
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            int row, col;

            // 1. Kiểm tra xem người chơi có nháy chuột vào các nút Option trên UI không
            if (!ui.handle_option_click(mousePos, board, current_player)) {
                // 2. Kiểm tra xem click chuột có trúng bàn cờ không
                if (ui.screen_to_grid(mousePos, board, row, col)) {
                    // 3. Thử đặt quân cờ vào vị trí đó trong Board
                    if (board.place_stone(row, col, current_player)) {
                        // 4. Nếu đặt thành công, đổi lượt chơi cho người kia
                        current_player = (current_player == 1) ? 2 : 1;
                    }
                }
            }
        }

        // --- VẼ GIAO DIỆN ---
        BeginDrawing();
        ClearBackground(ui.get_background_color());

        // Gọi hàm vẽ của class UI (đọc trạng thái từ đối tượng board)
        ui.draw(board, current_player);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
