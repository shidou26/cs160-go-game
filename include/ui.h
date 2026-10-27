#pragma once
#include <raylib.h>
#include "board.h"

// Các hằng số kích thước giao diện mặc định
const int DEFAULT_GRID_WIDTH = 480;
const int START_X = 160;
const int START_Y = 160;
const int PADDING = 50;

struct OptionButton {
    Rectangle bounds;
    const char* label;
};

class UI {
private:
    int startX;
    int startY;
    int padding;
    int gridWidth;

    Color bgColor;
    Color boardWood;
    Color lineColor;
    Color boardBorder;

    // Các nút chọn Option trên giao diện
    OptionButton btnSize9;
    OptionButton btnSize13;
    OptionButton btnSize19;
    OptionButton btnUndo;
    OptionButton btnReset;

    int GetCellSize(const Board &board) const;
    void DrawButton(const OptionButton &btn, Vector2 mousePos, bool isActive = false) const;

public:
    UI();

    Color GetBackgroundColor() const;

    // Chỉ làm nhiệm vụ vẽ bàn cờ, quân cờ và các nút Option dựa trên dữ liệu từ Board
    void Draw(const Board &board, int currentPlayer) const;

    // Chuyển đổi tọa độ click chuột trên màn hình sang vị trí (row, col) của Board
    bool ScreenToGrid(Vector2 mousePos, const Board &board, int &row, int &col) const;

    // Xử lý khi người chơi nháy chuột vào các nút Option trên giao diện
    bool HandleOptionClick(Vector2 mousePos, Board &board, int &currentPlayer);
};
