// AI-assisted: see AI12

#include <raylib.h>
#include <iostream>

#include "../include/board.h"
#include "../include/ui.h"
#include "../include/helper.h"
#include "../include/cpu.h"
#include "../include/audio.h"

int main() {
    //Setup
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_ALWAYS_RUN);
    InitWindow(1280, 960, TITLE);
    SetTargetFPS(60);

    //Set game icon
    Image icon = LoadImage("assets/icon.png");
    SetWindowIcon(icon);
    UnloadImage(icon);

    //Interface
    UI user_interface;
    user_interface.load_setting("saves/config.json");

    //Set game soundeffect and soundtrack
    Audio::init();
    Audio::change_music(user_interface.selected_music);
    
    //Game
    GamePhase phase = GamePhase::START;
    Board board;
    double cpu_turn_started = -1.0;
    const double CPU_DELAY = 1.5; // seconds

    while (!WindowShouldClose()) {
        if (phase != GamePhase::PLAYING) {
            cpu_turn_started = -1.0;
        }

        //Sound handler
        Audio::update();
        Audio::set_volume(
            user_interface.is_muted_master ? 0 : user_interface.master_volume,
            user_interface.is_muted_sound ? 0 : user_interface.sound_volume,
            user_interface.is_muted_music ? 0 : user_interface.music_volume
        );

        //Phase handler
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
                if (user_interface.handle_option_click(mouse_pos, phase, board)) {
                    user_interface.save_setting("saves/config.json");
                }
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
                                Audio::play(SFX::STONE);
                            }
                        }
                    }
                }
            }

            if (board.get_pass() == 2) {
                phase = GamePhase::GAMEOVER;
                Audio::play(SFX::END);
                continue;
            }

            if (!board.is_human_turn()) {
                if (cpu_turn_started < 0.0) {
                    cpu_turn_started = GetTime();
                }
                if (GetTime() - cpu_turn_started >= CPU_DELAY) {
                    Move seol = cpu_play(board);
                    if (board.place_stone(seol.pos_x, seol.pos_y)) {
                        Audio::play(SFX::STONE);
                    }
                    cpu_turn_started = -1.0;
                }
            } else {
                cpu_turn_started = -1.0;
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
    Audio::shutdown();
    CloseWindow();
    
    return 0;
}
