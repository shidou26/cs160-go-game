#include <raylib.h>
#include <iostream>

#include "../include/board.h"
#include "../include/ui.h"
#include "../include/helper.h"
#include "../include/cpu.h"

int main() {
    //Setup
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_ALWAYS_RUN);
    InitWindow(1280, 960, TITLE);
    SetTargetFPS(60);

    //Set game icon
    Image icon = LoadImage("assets/icon.png");
    SetWindowIcon(icon);
    UnloadImage(icon);

    //Set game soundeffect and soundtrack
    InitAudioDevice();
    Sound stone_sound = LoadSound("assets/sfx/go_stone_1.wav");
    Music music = LoadMusicStream("assets/soundtrack/song_1.mp3");
    PlayMusicStream(music);

    //Game
    GamePhase phase = GamePhase::START;
    Board board;
    UI user_interface;

    while (!WindowShouldClose()) {
        //Sound handler
        UpdateMusicStream(music);

        if (phase == GamePhase::SAVING) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = user_interface.get_virtual_mouse();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }

            user_interface.begin_render();
            user_interface.draw_saving();
            user_interface.end_render();
        }
        else if(phase == GamePhase::LOADING) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = user_interface.get_virtual_mouse();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }

            user_interface.begin_render();
            user_interface.draw_loading();
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
        else if (phase == GamePhase::CHOOSING_SIZE) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = user_interface.get_virtual_mouse();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }

            user_interface.begin_render();
            user_interface.draw_choosing_board_size(board);
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
                            if (board.place_stone(row, col)) {
                                PlaySound(stone_sound);
                            }
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
                if (board.place_stone(seol.pos_x, seol.pos_y)) {
                    PlaySound(stone_sound);
                }
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
    UnloadSound(stone_sound);
    UnloadMusicStream(music);

    CloseAudioDevice();
    CloseWindow();
    
    return 0;
}
