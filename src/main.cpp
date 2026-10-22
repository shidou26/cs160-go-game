#include <raylib.h>
#include "ui.h"

int currentPlayer = 1;

int main() {
    InitWindow(800, 800, "Go Game - 9x9");
    SetTargetFPS(60);

    // 1. Tạo đối tượng bàn cờ
    Board board;

    Color BG_COLOR = { 20, 24, 35, 255 };

    while (!WindowShouldClose()) {
        // --- CẬP NHẬT LOGIC (Sau này chúng ta sẽ bắt click chuột ở đây) ---

        // --- VẼ GIAO DIỆN ---

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            int row, col;

            // 1. Kiểm tra xem click chuột có trúng bàn cờ không
            if (board.ScreenToGrid(mousePos, row, col)) {
                // 2. Thử đặt quân cờ vào vị trí đó
                if (board.PlaceStone(row, col, currentPlayer)) {
                    // 3. Nếu đặt thành công, đổi lượt chơi cho người kia
                    currentPlayer = (currentPlayer == 1) ? 2 : 1;
                }
            }

            if (currentPlayer == 1) {
                DrawText("Turn: BLACK", 20, 20, 20, RAYWHITE);
            } else {
                DrawText("Turn: WHITE", 20, 20, 20, RAYWHITE);
            }
        }

        BeginDrawing();
        ClearBackground(BG_COLOR);

        // Gọi hàm vẽ của class Board
        board.Draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}