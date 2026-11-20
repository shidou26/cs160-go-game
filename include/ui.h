// AI-assisted: see AI24

#pragma once

#include <raylib.h>
#include <string>

#include "../include/board.h"
#include "../include/helper.h"

struct OptionButton {
    Rectangle bounds;
    const char* label;
};

class UI {
private:
    int start_x;
    int start_y;
    int padding;
    int grid_width;

    // [HD UPGRADE]: High-resolution Textures for stones, wood board, and crisp custom Font
    Texture2D black_stone_tex;
    Texture2D white_stone_tex;
    Texture2D wood_board_tex;
    RenderTexture2D render_target;
    Font ui_font;
    bool has_custom_font = false;

    Color bg_color;
    Color board_wood;
    Color line_color;
    Color board_border;

    //Option button during start menu
    OptionButton btn_new_game;
    OptionButton btn_load_game;
    OptionButton btn_config;

    //Option button during choosing board size
    OptionButton btn_size9;
    OptionButton btn_size13;
    OptionButton btn_size19;

    //Option button during game 
    OptionButton btn_undo;
    OptionButton btn_redo;
    OptionButton btn_reset;
    OptionButton btn_pass;
    OptionButton btn_save;
    OptionButton btn_setting_play;
    OptionButton btn_back_from_play;

    //Option buttons for saving and loading
    OptionButton btn_save_slots[3];
    OptionButton btn_load_slots[3];
    OptionButton btn_back_save;
    OptionButton btn_back_load;
    OptionButton btn_confirm_overwrite;
    OptionButton btn_cancel_overwrite;
    int overwrite_slot = -1;

    //Option button during gameover menu
    OptionButton btn_back_from_end; // go back to start screen

    //Option button during config (Settings)
    OptionButton btn_master_down;
    OptionButton btn_master_up;
    OptionButton btn_master_mute;
    OptionButton btn_music_down;
    OptionButton btn_music_up;
    OptionButton btn_music_mute;
    OptionButton btn_sound_down;
    OptionButton btn_sound_up;
    OptionButton btn_sound_mute;
    OptionButton btn_change_music;
    OptionButton opt_music[3];
    OptionButton btn_windowed;
    OptionButton btn_fullscreen;
    OptionButton btn_back_config;
    GamePhase previous_phase = GamePhase::START;
    bool music_dropdown_open = false;

    //Option button during choosing mode
    OptionButton btn_pvp;
    OptionButton btn_pve;

    //Option button during choose difficulty
    OptionButton btn_easy;
    OptionButton btn_medium;
    OptionButton btn_hard;


    int get_cell_size(const Board &board) const;
    void draw_button(const OptionButton &btn, Vector2 mouse_pos, bool is_active = false, int font_size = 18) const;

    // [HD UPGRADE]: Helper to render crisp text using high-res custom font instead of default DrawText
    void draw_text_hd(const char* text, float x, float y, float font_size, Color color) const;

public:
    int sound_volume;
    int music_volume;
    int master_volume;
    bool is_muted_sound;
    bool is_muted_music;
    bool is_muted_master;
    int selected_music;

    UI();

    Color get_background_color() const;

    /**
     * @brief Drawing
     * 
     * @param board 
     */
    void unload();
    void begin_render() const;
    void end_render() const;
    Vector2 get_virtual_mouse() const;
    void draw(Board &board);
    void draw_starting(const Board &board);
    void draw_ending(const Board &board);
    void draw_config();
    void draw_choosing_board_size(const Board &board);
    void draw_loading();
    void draw_saving();
    void draw_mode();
    void draw_difficulty();

    //Convert screen mouse coordinates to Board (row, col) position
    bool screen_to_grid(Vector2 mouse_pos, const Board &board, int &row, int &col) const;

    //Handle mouse clicks on UI option buttons
    bool handle_option_click(Vector2 mouse_pos, GamePhase &phase, Board &board);

    //Save settings
    bool save_setting(const std::string &path);

    //Load settings
    bool load_setting(const std::string &path);
};
