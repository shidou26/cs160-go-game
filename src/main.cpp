#include <raylib.h>
#include "board.h"
#include "ui.h"

int currentPlayer = 1;

int main() {
    InitWindow(1280, 960, "Go Game");
    SetTargetFPS(60);

    // 1. Tạo đối tượng bàn cờ (quản lý grid[i][j] & logic) và đối tượng giao diện (UI)
    Board board(9);
    UI ui;

    while (!WindowShouldClose()) {
        // --- CẬP NHẬT LOGIC & BẮT CLICK CHUỘT ---
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            int row, col;

            // 1. Kiểm tra xem người chơi có nháy chuột vào các nút Option trên UI không
            if (!ui.HandleOptionClick(mousePos, board, currentPlayer)) {
                // 2. Kiểm tra xem click chuột có trúng bàn cờ không
                if (ui.ScreenToGrid(mousePos, board, row, col)) {
                    // 3. Thử đặt quân cờ vào vị trí đó trong Board
                    if (board.PlaceStone(row, col, currentPlayer)) {
                        // 4. Nếu đặt thành công, đổi lượt chơi cho người kia
                        currentPlayer = (currentPlayer == 1) ? 2 : 1;
                    }
                }
            }
        }

        // --- VẼ GIAO DIỆN ---
        BeginDrawing();
        ClearBackground(ui.GetBackgroundColor());

        // Gọi hàm vẽ của class UI (đọc trạng thái từ đối tượng board)
        ui.Draw(board, currentPlayer);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
