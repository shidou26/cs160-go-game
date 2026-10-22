#pragma once
#include <raylib.h>

// Các hằng số kích thước giữ nguyên từ code gốc của bạn
const int BOARD_SIZE = 9;
const int CELL_SIZE = 60;
const int START_X = 160;
const int START_Y = 160;
const int PADDING = 50;

class Board {
private:
    int grid[BOARD_SIZE][BOARD_SIZE];
    
    int cellSize;
    int startX;
    int startY;
    int padding;
    int gridWidth;

    Color boardWood;
    Color lineColor;
    Color boardBorder;

public:
    Board();

    void Reset();
    void Draw();
    bool PlaceStone(int row, int col, int player);
    bool ScreenToGrid(Vector2 mousePos, int &row, int &col);
    int GetCell(int row, int col) const;
};