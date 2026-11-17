#include <raylib.h>
#include <iostream>

#include "../include/board.h"
#include "../include/ui.h"
#include "../include/helper.h"
#include "../include/cpu.h"

int main() {
    //Setup
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280, 960, TITLE);
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
                Vector2 mouse_pos = user_interface.get_virtual_mouse();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }

            user_interface.begin_render();

            if (phase == GamePhase::SAVING) {
                user_interface.draw_saving();
            } else {
                user_interface.draw_loading();
            }
            user_interface.end_render();
        }
        else if (phase == GamePhase::CONFIG) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = user_interface.get_virtual_mouse();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }
            user_interface.begin_render();

            user_interface.draw_config();
            user_interface.end_render();
        } 
        else if (phase == GamePhase::START) {
            if(IsKeyPressed(KEY_H)) {
                break;
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = user_interface.get_virtual_mouse();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }
            user_interface.begin_render();

            user_interface.draw_starting(board);
            user_interface.end_render();
        } 
        else if (phase == GamePhase::MODE) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = user_interface.get_virtual_mouse();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }
            user_interface.begin_render();

            user_interface.draw_mode();
            user_interface.end_render();
        }
        else if(phase == GamePhase::DIFFICULTY) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = user_interface.get_virtual_mouse();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }
            user_interface.begin_render();

            user_interface.draw_difficulty();
            user_interface.end_render();
        }
        else if(phase == GamePhase::PLAYING) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (board.is_human_turn()) {
                    Vector2 mouse_pos = user_interface.get_virtual_mouse();
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
            
            user_interface.begin_render();

            user_interface.draw(board);
            user_interface.end_render();
        } 
        else {
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = user_interface.get_virtual_mouse();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }

            user_interface.begin_render();

            user_interface.draw_ending(board);
            user_interface.end_render();
        }
    }

    user_interface.unload();
    CloseWindow();
    
    return 0;
}
