#include "ui.h"

Board::Board() {
    cellSize = CELL_SIZE;
    startX = START_X;
    startY = START_Y;
    padding = PADDING;
    gridWidth = (BOARD_SIZE - 1) * cellSize;

    boardWood = (Color){ 219, 169, 99, 255 };
    lineColor = (Color){ 60, 40, 20, 255 };
    boardBorder = (Color){ 180, 133, 78, 255 };

    Reset();
}

void Board::Reset() {
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            grid[r][c] = 0;
        }
    }
}

void Board::Draw() {
    // Giữ nguyên đúng đoạn vẽ bàn cờ của bạn
    Rectangle boardRect = { 
        (float)(startX - padding), 
        (float)(startY - padding), 
        (float)(gridWidth + padding * 2), 
        (float)(gridWidth + padding * 2) 
    };

    DrawRectangleRounded(boardRect, 0.05f, 4, boardWood); 

    for (int i = 0; i < BOARD_SIZE; i++) {
        DrawLine(startX + i * cellSize, startY, startX + i * cellSize, startY + gridWidth, lineColor);
        DrawLine(startX, startY + i * cellSize, startX + gridWidth, startY + i * cellSize, lineColor);
    }

        // --- VẼ CÁC QUÂN CỜ ---
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (grid[r][c] != 0) {
                // 1. Tính tọa độ tâm của quân cờ tại giao điểm (r, c)
                int stoneX = startX + c * cellSize;
                int stoneY = startY + r * cellSize;
                int radius = 24; // Bán kính quân cờ (khoảng 24-25 pixel là vừa khít ô 60)

                // 2. Vẽ quân cờ tương ứng
                if (grid[r][c] == 1) {
                    // Quân Đen
                    DrawCircle(stoneX, stoneY, radius, BLACK);
                } else if (grid[r][c] == 2) {
                    // Quân Trắng + viền xám nhẹ để nổi bật trên mặt gỗ
                    DrawCircle(stoneX, stoneY, radius, RAYWHITE);
                    DrawCircleLines(stoneX, stoneY, radius, DARKGRAY);
                }
            }
        }
    }
}

bool Board::PlaceStone(int row, int col, int player) {
    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE) {
        if (grid[row][col] == 0) {
            grid[row][col] = player;
            return true;
        }
    }
    return false;
}

bool Board::ScreenToGrid(Vector2 mousePos, int &row, int &col) {
    col = (int)((mousePos.x - startX + cellSize / 2.0f) / cellSize);
    row = (int)((mousePos.y - startY + cellSize / 2.0f) / cellSize);

    return (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE);
}

int Board::GetCell(int row, int col) const {
    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE) {
        return grid[row][col];
    }
    return 0;
}