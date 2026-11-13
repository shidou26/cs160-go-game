#include <raylib.h>
#include <iostream>

#include "../include/board.h"
#include "../include/ui.h"
#include "../include/helper.h"
#include "../include/cpu.h"

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
    Board board;
    UI user_interface;

    while (!WindowShouldClose()) {
        if (phase == GamePhase::SAVING || phase == GamePhase::LOADING) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = GetMousePosition();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }
            if (phase != GamePhase::SAVING && phase != GamePhase::LOADING) continue;

            BeginDrawing();
            ClearBackground(user_interface.get_background_color());
            if (phase == GamePhase::SAVING) {
                user_interface.draw_saving();
            } else {
                user_interface.draw_loading();
            }
            EndDrawing();
        }
        else if (phase == GamePhase::CONFIG) {
            BeginDrawing();
            ClearBackground(user_interface.get_background_color());
            user_interface.draw_config();
            EndDrawing();
        } 
        else if (phase == GamePhase::START) {
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
        else if (phase == GamePhase::MODE) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = GetMousePosition();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }
            BeginDrawing();
            ClearBackground(user_interface.get_background_color());
            user_interface.draw_mode();
            EndDrawing();
        }
        else if(phase == GamePhase::DIFFICULTY) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = GetMousePosition();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }
            BeginDrawing();
            ClearBackground(user_interface.get_background_color());
            user_interface.draw_difficulty();
            EndDrawing();
        }
        else if(phase == GamePhase::PLAYING) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (board.is_human_turn()) {
                    Vector2 mouse_pos = GetMousePosition();
                    int row, col;

                    if (!user_interface.handle_option_click(mouse_pos, phase, board)) {
                        if (user_interface.screen_to_grid(mouse_pos, board, row, col)) {
                            board.place_stone(row, col);
                        }
                    }
                }
            }

            if (board.get_pass() == 2) {
                phase = GamePhase::GAMEOVER;
                continue;
            }

            if (!board.is_human_turn()) {
                Move seol = cpu_play(board);
                board.place_stone(seol.pos_x, seol.pos_y);
            }
            
            BeginDrawing();
            ClearBackground(user_interface.get_background_color());
            user_interface.draw(board);
            EndDrawing();
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
