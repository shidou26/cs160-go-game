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
    Image icon = LoadImage("assets/images/icon.png");
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
    double last_timer = GetTime();
    const double CPU_DELAY = 1.5; // seconds

    while (!WindowShouldClose()) {
        //Timer handler
        double now_timer = GetTime();
        if (phase == GamePhase::PLAYING) {
            board.update_timer(now_timer - last_timer);
        }
        last_timer = now_timer;
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
                Vector2 mouse_pos = GetMousePosition();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }

            BeginDrawing();
            ClearBackground(user_interface.get_background_color());
            user_interface.draw_saving();
            EndDrawing();
        }
        else if(phase == GamePhase::LOADING) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = GetMousePosition();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }

            BeginDrawing();
            ClearBackground(user_interface.get_background_color());
            user_interface.draw_loading();
            EndDrawing();
        }
        else if (phase == GamePhase::CONFIG) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = GetMousePosition();
                if (user_interface.handle_option_click(mouse_pos, phase, board)) {
                    user_interface.save_setting("saves/config.json");
                }
            }

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
            user_interface.draw_starting();
            EndDrawing();
        } 
        else if (phase == GamePhase::CHOOSING_SIZE) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse_pos = GetMousePosition();
                user_interface.handle_option_click(mouse_pos, phase, board);
            }

            BeginDrawing();
            ClearBackground(user_interface.get_background_color());
            user_interface.draw_choosing_board_size(board);
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
                            int cap_before = board.get_captured(0) + board.get_captured(1);
                            if (board.place_stone(row, col)) {
                                Audio::play(SFX::STONE);
                                int cap_after = board.get_captured(0) + board.get_captured(1);
                                if (cap_after > cap_before) {
                                    user_interface.notify_stone_captured();
                                }
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
                if (cpu_turn_started == -1.0) cpu_turn_started = GetTime();
                if (GetTime() - cpu_turn_started >= CPU_DELAY) {
                    Move seol = cpu_play(board);
                    // Count CPU thinking time before placing the stone changes turns.
                    double finished = GetTime();
                    board.update_timer(finished - last_timer);
                    last_timer = finished;
                    
                    int cap_before = board.get_captured(0) + board.get_captured(1);
                    if (board.place_stone(seol.pos_x, seol.pos_y)) {
                        Audio::play(SFX::STONE);
                        int cap_after = board.get_captured(0) + board.get_captured(1);
                        if (cap_after > cap_before) {
                            user_interface.notify_stone_captured();
                        }
                    }
                    
                    cpu_turn_started = -1.0;
                }
            } else {
                cpu_turn_started = -1.0;
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
            user_interface.draw(board, false);
            user_interface.draw_ending(board);
            EndDrawing();
        }

    }

    user_interface.unload();
    Audio::shutdown();
    CloseWindow();
    
    return 0;
}
