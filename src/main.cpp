#include <raylib.h>
#include <iostream>

#include "../include/board.h"
#include "../include/ui.h"
#include "../include/helper.h"

int main() {
    //Setup
    InitWindow(1280, 960, "Anhs's Game of Go");
    SetTargetFPS(60);

    //Set game icon
    Image icon = LoadImage("assets/icon.png");
    SetWindowIcon(icon);
    UnloadImage(icon);

    //Game
    GamePhase phase = GamePhase::START;
    GameMode mode;
    Board board(BOARD_SIZE);
    UI user_interface;

    while (!WindowShouldClose()) {
        if (phase == GamePhase::START) {
            if(IsKeyPressed(KEY_H)) {
                break;
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = GetMousePosition();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }

            BeginDrawing();
            ClearBackground(user_interface.get_background_color());
            user_interface.draw_starting(board);
            EndDrawing();
        } 
        else if(phase == GamePhase::PLAYING) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = GetMousePosition();
                int row, col;

                // 1. Kiểm tra xem người chơi có nháy chuột vào các nút Option trên UI không
                if (!user_interface.handle_option_click(mouse_pos, phase, board)) {
                    // 2. Kiểm tra xem click chuột có trúng bàn cờ không
                    if (user_interface.screen_to_grid(mouse_pos, board, row, col)) {
                        // 3. Thử đặt quân cờ vào vị trí đó trong Board
                        if (board.place_stone(row, col)) {

                        }
                    }
                }
            }

            if (board.get_pass() == 2) {
                phase = GamePhase::GAMEOVER;
            } else {
                BeginDrawing();
                ClearBackground(user_interface.get_background_color());
                user_interface.draw(board);
                EndDrawing();
            }
        } 
        else {
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = GetMousePosition();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }

            BeginDrawing();
            ClearBackground(user_interface.get_background_color());
            user_interface.draw_ending(board);
            EndDrawing();
        }
    }

    CloseWindow();
    return 0;
}
