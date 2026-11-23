// AI-assisted: see AI24

#pragma once

#include <raylib.h>
#include <string>
#include <vector>

#include "../include/board.h"
#include "../include/helper.h"

struct OptionButton {
    Rectangle bounds;
    const char* label;
};

struct ThemeOption {
    std::string name;
    Texture2D bg_tex{};
    Texture2D btn_normal_tex{};
    Texture2D btn_hover_tex{};
    Texture2D btn_active_tex{};
    Color bg_fallback       = { 20, 24, 35, 255 };
    Color text_color        = RAYWHITE;
    Color active_text_color = { 20, 24, 35, 255 };
    Color accent_color      = { 219, 169, 99, 255 };
    Color line_color        = { 55, 36, 18, 255 };
    Color board_border      = { 170, 124, 70, 255 };
};

struct BoardOption {
    std::string name;
    Texture2D tex{};
};

struct StoneOption {
    std::string name;
    Texture2D black_tex{};
    Texture2D white_tex{};
};

class UI {
private:
    int start_x;
    int start_y;
    int padding;
    int grid_width;

    Texture2D black_stone_tex;
    Texture2D white_stone_tex;
    Texture2D wood_board_tex;
    Texture2D trophy_tex;

    //Font
    Font title_font, heading_font, button_font, label_font, body_font;

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
    OptionButton btn_music_prev;
    OptionButton btn_music_next;
    OptionButton btn_tab_theme;
    OptionButton btn_tab_board;
    OptionButton btn_tab_stones;
    OptionButton btn_back_config;
    GamePhase previous_phase = GamePhase::START;

    // Flexible auto-scanned PNG customization lists & scroll state
    int active_select_tab = 0; // 0: Game Theme, 1: Board, 2: Stones
    float select_scroll_offset = 0.0f;
    std::vector<ThemeOption> theme_options;
    std::vector<BoardOption> board_options;
    std::vector<StoneOption> stone_options;
    int selected_theme_idx = 0;
    int selected_board_idx = 0;
    int selected_stone_idx = 0;

    //Option button during choosing mode
    OptionButton btn_pvp;
    OptionButton btn_pve;

    //Option button during choose difficulty
    OptionButton btn_easy;
    OptionButton btn_medium;
    OptionButton btn_hard;
    double capture_notify_until = 0.0;

    int get_cell_size(const Board &board) const;
    const ThemeOption &active_theme() const;
    void scan_custom_assets();
    void apply_selected_assets();
    void draw_background() const;
    void draw_select_preview(Rectangle preview_box) const;
    void draw_button(const OptionButton &btn, Vector2 mouse_pos, bool is_active = false, int font_size = 18, bool interactive = true) const;

public:
    int sound_volume;
    int music_volume;
    int master_volume;
    bool is_muted_sound;
    bool is_muted_music;
    bool is_muted_master;
    int selected_music;
    std::string selected_theme = "Classic";
    std::string selected_board = "Kaya Wood";
    std::string selected_stones = "3D Glossy";

    UI();

    Color get_background_color() const;

    /**
     * @brief Drawing
     * 
     * @param board 
     */
    void apply_theme(int id);
    void unload();
    void draw(Board &board, bool interactive = true);
    void draw_starting();
    void draw_ending(const Board &board);
    void draw_config();
    void draw_choosing_board_size(const Board &board);
    void draw_loading();
    void draw_saving();
    void draw_mode();
    void draw_difficulty();
    void draw_shop();
    void notify_stone_captured();

    //Convert screen mouse coordinates to Board (row, col) position
    bool screen_to_grid(Vector2 mouse_pos, const Board &board, int &row, int &col) const;

    //Handle mouse clicks on UI option buttons
    bool handle_option_click(Vector2 mouse_pos, GamePhase &phase, Board &board);

    //Save settings
    bool save_setting(const std::string &path);

    //Load settings
    bool load_setting(const std::string &path);
};
