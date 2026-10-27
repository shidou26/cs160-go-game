#include "ui.h"

UI::UI() {
    startX = START_X;
    startY = START_Y;
    padding = PADDING;
    gridWidth = DEFAULT_GRID_WIDTH;

    bgColor = (Color){ 20, 24, 35, 255 };
    boardWood = (Color){ 219, 169, 99, 255 };
    lineColor = (Color){ 60, 40, 20, 255 };
    boardBorder = (Color){ 180, 133, 78, 255 };

    // Khởi tạo vị trí các nút Option trên giao diện
    btnSize9  = { { 260, 25, 75, 36 }, "9x9" };
    btnSize13 = { { 345, 25, 85, 36 }, "13x13" };
    btnSize19 = { { 440, 25, 85, 36 }, "19x19" };
    btnUndo   = { { 555, 25, 95, 36 }, "Undo" };
    btnReset  = { { 660, 25, 100, 36 }, "Reset" };
}

Color UI::GetBackgroundColor() const {
    return bgColor;
}

int UI::GetCellSize(const Board &board) const {
    int bSize = board.GetSize();
    return (bSize > 1) ? (gridWidth / (bSize - 1)) : gridWidth;
}

void UI::DrawButton(const OptionButton &btn, Vector2 mousePos, bool isActive) const {
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

void UI::Draw(const Board &board, int currentPlayer) const {
    int bSize = board.GetSize();
    int cellSize = GetCellSize(board);
    int actualGridWidth = (bSize - 1) * cellSize;
    Vector2 mousePos = GetMousePosition();

    // 1. Vẽ thông tin lượt chơi và các nút Option
    if (currentPlayer == 1) {
        DrawText("Turn: BLACK", 30, 33, 22, RAYWHITE);
    } else {
        DrawText("Turn: WHITE", 30, 33, 22, RAYWHITE);
    }

    DrawButton(btnSize9,  mousePos, bSize == 9);
    DrawButton(btnSize13, mousePos, bSize == 13);
    DrawButton(btnSize19, mousePos, bSize == 19);
    DrawButton(btnUndo,   mousePos, false);
    DrawButton(btnReset,  mousePos, false);

    // 2. Vẽ mặt bàn cờ gỗ
    Rectangle boardRect = { 
        (float)(startX - padding), 
        (float)(startY - padding), 
        (float)(actualGridWidth + padding * 2), 
        (float)(actualGridWidth + padding * 2) 
    };

    DrawRectangleRounded(boardRect, 0.05f, 4, boardWood); 

    // 3. Vẽ các đường kẻ lưới
    for (int i = 0; i < bSize; i++) {
        DrawLine(startX + i * cellSize, startY, startX + i * cellSize, startY + actualGridWidth, lineColor);
        DrawLine(startX, startY + i * cellSize, startX + actualGridWidth, startY + i * cellSize, lineColor);
    }

    // 4. Vẽ các quân cờ dựa trên dữ liệu đọc từ Board
    int radius = (int)(cellSize * 0.4f);
    Color blackStoneColor = { 0, 0, 0, 255 };

    for (int r = 0; r < bSize; r++) {
        for (int c = 0; c < bSize; c++) {
            int cellState = board.GetCell(r, c);
            if (cellState != 0) {
                int stoneX = startX + c * cellSize;
                int stoneY = startY + r * cellSize;

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

bool UI::ScreenToGrid(Vector2 mousePos, const Board &board, int &row, int &col) const {
    int bSize = board.GetSize();
    int cellSize = GetCellSize(board);

    col = (int)((mousePos.x - startX + cellSize / 2.0f) / cellSize);
    row = (int)((mousePos.y - startY + cellSize / 2.0f) / cellSize);

    return (row >= 0 && row < bSize && col >= 0 && col < bSize);
}

bool UI::HandleOptionClick(Vector2 mousePos, Board &board, int &currentPlayer) {
    if (CheckCollisionPointRec(mousePos, btnSize9.bounds)) {
        board.Reset(9);
        currentPlayer = 1;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, btnSize13.bounds)) {
        board.Reset(13);
        currentPlayer = 1;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, btnSize19.bounds)) {
        board.Reset(19);
        currentPlayer = 1;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, btnUndo.bounds)) {
        board.undo_move(true);
        currentPlayer = board.get_turn() ? 2 : 1;
        return true;
    }
    if (CheckCollisionPointRec(mousePos, btnReset.bounds)) {
        board.Reset();
        currentPlayer = 1;
        return true;
    }
    return false;
}
