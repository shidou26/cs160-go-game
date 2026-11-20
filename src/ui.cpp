// AI-assisted: see AI09, AI10, AI13, AI14

#include <iostream>
#include <utility>
#include <filesystem>
#include <cmath>
#include <algorithm>
#include <vector>

#include "../include/ui.h"
#include "../include/helper.h"
#include "../include/rule.h"
#include "../include/score.h"
#include "../include/board.h"
#include "../include/audio.h"

namespace {

Texture2D create_hd_stone_texture(bool is_black) {
    const int tex_size = 512;
    Image img = GenImageColor(tex_size, tex_size, BLANK);
    Color *pixels = (Color *)img.data;

    float center = (tex_size - 1) * 0.5f;
    float max_r = tex_size * 0.47f;

    for (int y = 0; y < tex_size; y++) {
        for (int x = 0; x < tex_size; x++) {
            float dx = (x - center) / max_r;
            float dy = (y - center) / max_r;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist > 1.0f) continue;

            // Sub-pixel anti-aliasing along the circle edge for crisp, smooth borders
            float edge_alpha = std::clamp((1.0f - dist) * (max_r * 0.5f), 0.0f, 1.0f);

            // Compute 3D sphere normal vector
            float nz = std::sqrt(std::max(0.0f, 1.0f - dist * dist));

            // Main light source from top-left (-0.42, -0.45, 0.78)
            float lx = -0.42f, ly = -0.45f, lz = 0.78f;
            float diff = std::max(0.0f, dx * lx + dy * ly + nz * lz);

            // Glossy specular highlight at top-left
            float hx = dx + 0.36f;
            float hy = dy + 0.38f;
            float h_dist = std::sqrt(hx * hx + hy * hy);
            float specular = std::pow(std::max(0.0f, 1.0f - h_dist / 0.48f), 2.8f) * 0.65f;

            // Bottom-right rim light for 3D depth
            float rx = dx - 0.35f;
            float ry = dy - 0.35f;
            float rim = std::pow(std::clamp(dist, 0.0f, 1.0f), 3.0f) * std::max(0.0f, (rx + ry) * 0.45f);

            float r_val, g_val, b_val;
            if (is_black) {
                // Black stone: glossy slate finish with crisp reflections
                float base = 14.0f + diff * 48.0f + rim * 55.0f;
                r_val = base + specular * 215.0f;
                g_val = base + specular * 218.0f;
                b_val = base + 4.0f + specular * 230.0f;
            } else {
                // White stone: ivory shell stone with soft 3D shading and highlight
                float shade = 195.0f + diff * 55.0f - std::pow(dist, 2.2f) * 32.0f + rim * 28.0f;
                // Subtle natural shell grain
                float grain = std::sin((dx * 6.0f + dy * 3.0f) * 3.14159f) * 2.5f;
                r_val = shade + grain + specular * 55.0f;
                g_val = shade + grain + specular * 55.0f;
                b_val = shade + 3.0f + grain + specular * 58.0f;
            }

            // Slightly darken the outer rim to separate the stone from the board
            float border_darken = std::clamp((1.0f - std::pow(dist, 6.0f) * 0.35f), 0.65f, 1.0f);
            r_val *= border_darken;
            g_val *= border_darken;
            b_val *= border_darken;

            pixels[y * tex_size + x] = (Color){
                (unsigned char)std::clamp((int)r_val, 0, 255),
                (unsigned char)std::clamp((int)g_val, 0, 255),
                (unsigned char)std::clamp((int)b_val, 0, 255),
                (unsigned char)std::clamp((int)(edge_alpha * 255.0f), 0, 255)
            };
        }
    }

    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);

    // [HD UPGRADE]: Generate Mipmaps and enable Trilinear filtering so 19x19 stones stay sharp when scaled down
    GenTextureMipmaps(&tex);
    SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
    return tex;
}

// [HD UPGRADE]: Generate high-resolution natural wood grain texture (1024x1024) for the board
Texture2D create_hd_wood_texture() {
    const int tex_size = 1024;
    Image img = GenImageColor(tex_size, tex_size, BLANK);
    Color *pixels = (Color *)img.data;

    for (int y = 0; y < tex_size; y++) {
        for (int x = 0; x < tex_size; x++) {
            float u = (float)x / tex_size;
            float v = (float)y / tex_size;

            // Vertical wood grain fibers resembling a Kaya Go board
            float wave = std::sin(v * 6.28318f * 1.5f + std::sin(u * 25.0f) * 0.35f) * 0.04f;
            float fiber1 = std::sin((u + wave) * 180.0f);
            float fiber2 = std::sin((u * 0.7f + wave * 1.3f) * 420.0f) * 0.45f;
            float fine_grain = std::sin((u * 950.0f) + v * 18.0f) * 0.2f;
            float plank_tone = std::sin(u * 18.0f) * 6.0f;

            float grain_val = (fiber1 + fiber2 + fine_grain) * 5.5f + plank_tone;

            // Warm golden wood tone
            int r = std::clamp((int)(226.0f + grain_val * 1.05f), 0, 255);
            int g = std::clamp((int)(178.0f + grain_val * 0.95f), 0, 255);
            int b = std::clamp((int)(108.0f + grain_val * 0.75f), 0, 255);

            pixels[y * tex_size + x] = (Color){ (unsigned char)r, (unsigned char)g, (unsigned char)b, 255 };
        }
    }

    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    GenTextureMipmaps(&tex);
    SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
    return tex;
}

} // namespace

UI::UI() {
    // [HD UPGRADE]: Initialize high-res 3D stone and wood board textures with Mipmaps + Trilinear filtering
    black_stone_tex = create_hd_stone_texture(true);
    white_stone_tex = create_hd_stone_texture(false);
    wood_board_tex  = create_hd_wood_texture();

    // [HD UPGRADE]: Load system TrueType font at large size (64px) for crisp, non-pixelated text
    const char* font_paths[] = {
        "C:/Windows/Fonts/segoeuib.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arialbd.ttf",
        "C:/Windows/Fonts/arial.ttf"
    };
    has_custom_font = false;
    for (const char* path : font_paths) {
        if (std::filesystem::exists(path)) {
            ui_font = LoadFontEx(path, 64, nullptr, 0);
            if (ui_font.texture.id != 0) {
                GenTextureMipmaps(&ui_font.texture);
                SetTextureFilter(ui_font.texture, TEXTURE_FILTER_TRILINEAR);
                has_custom_font = true;
                break;
            }
        }
    }

    start_x = START_X;
    start_y = START_Y;
    padding = PADDING;
    grid_width = DEFAULT_GRID_WIDTH;

    bg_color = (Color){ 20, 24, 35, 255 };
    board_wood = (Color){ 219, 169, 99, 255 };
    line_color = (Color){ 55, 36, 18, 255 };
    board_border = (Color){ 170, 124, 70, 255 };

    //Init option buttons for starting phase
    btn_new_game  = {{ 140, 590, 1000, 80 }, "New Game"};
    btn_load_game = {{ 140, 690, 1000, 80 }, "Load Game"};
    btn_config    = {{ 140, 790, 1000, 80 }, "Settings"};

    //Init option buttons for choosing mode
    btn_pvp = { { 390, 250, 500, 100 }, "Play against Friend" };
    btn_pve = { { 390, 400, 500, 100 }, "Play against Seol" };

    //Init option buttons for choosing difficulty
    btn_easy = { { 390, 250, 500, 100 }, "Easy" };
    btn_medium = { { 390, 400, 500, 100 }, "Medium" };
    btn_hard = { { 390, 550, 500, 100 }, "Hard" };

    //Init option buttons for playing phase
    btn_size9  = { { 390, 250, 500, 100 }, "9x9" };
    btn_size13 = { { 390, 400, 500, 100 }, "13x13" };
    btn_size19 = { { 390, 550, 500, 100 }, "19x19" };
    btn_undo   = { { 554, 25, 88, 40 }, "Undo" };
    btn_redo   = { { 652, 25, 88, 40 }, "Redo" };
    btn_reset  = { { 750, 25, 88, 40 }, "Reset" };
    btn_pass   = { { 848, 25, 88, 40 }, "Pass" };
    btn_save   = { { 946, 25, 88, 40 }, "Save" };
    btn_setting_play   = { { 1135, 110, 115, 40 }, "Settings" };
    btn_back_from_play = { { 1044, 25, 88, 40 }, "Back" };

    //Init option buttons for config (Settings) phase
    btn_master_down = { { 560, 240, 60, 50 }, "-" };
    btn_master_up   = { { 760, 240, 60, 50 }, "+" };
    btn_master_mute = { { 840, 240, 120, 50 }, "Mute" };
    btn_music_down  = { { 560, 330, 60, 50 }, "-" };
    btn_music_up    = { { 760, 330, 60, 50 }, "+" };
    btn_music_mute  = { { 840, 330, 120, 50 }, "Mute" };
    btn_sound_down  = { { 560, 420, 60, 50 }, "-" };
    btn_sound_up    = { { 760, 420, 60, 50 }, "+" };
    btn_sound_mute  = { { 840, 420, 120, 50 }, "Mute" };
    btn_change_music = { { 980, 330, 230, 50 }, "Change Music" };
    opt_music[0] = { { 980, 385, 230, 50 }, "Song 1" };
    opt_music[1] = { { 980, 435, 230, 50 }, "Song 2" };
    opt_music[2] = { { 980, 485, 230, 50 }, "Song 3" };
    btn_back_config = { { 390, 650, 500, 60 }, "Back" };

    //Init option buttons for saving phase
    btn_save_slots[0] = { { 390, 220, 500, 80 }, "Save to Slot 1" };
    btn_save_slots[1] = { { 390, 330, 500, 80 }, "Save to Slot 2" };
    btn_save_slots[2] = { { 390, 440, 500, 80 }, "Save to Slot 3" };
    btn_back_save = { { 390, 650, 500, 60 }, "Return to game" };
    btn_confirm_overwrite = { { 390, 400, 230, 60 }, "Overwrite" };
    btn_cancel_overwrite = { { 660, 400, 230, 60 }, "Cancel" };

    //Init option buttons for loading phase
    btn_load_slots[0] = { { 390, 220, 500, 80 }, "Load Slot 1" };
    btn_load_slots[1] = { { 390, 330, 500, 80 }, "Load Slot 2" };
    btn_load_slots[2] = { { 390, 440, 500, 80 }, "Load Slot 3" };
    btn_back_load = { { 390, 650, 500, 60 }, "Return to title screen" };

    //Init option buttons for gameover phase
    btn_back_from_end = { { 380, 250, 500, 200 }, "Return to title screen"};
}

Color UI::get_background_color() const {
    return bg_color;
}

void UI::unload() {
    UnloadTexture(black_stone_tex);
    UnloadTexture(white_stone_tex);
    UnloadTexture(wood_board_tex);
    if (has_custom_font) {
        UnloadFont(ui_font);
    }
}

// [HD UPGRADE]: Render high-resolution text using DrawTextEx
void UI::draw_text_hd(const char* text, float x, float y, float font_size, Color color) const {
    if (has_custom_font) {
        DrawTextEx(ui_font, text, { x, y }, font_size, 1.0f, color);
    } else {
        DrawText(text, (int)x, (int)y, (int)font_size, color);
    }
}

int UI::get_cell_size(const Board &board) const {
    int board_size = board.get_size();
    return (board_size > 1) ? (grid_width / (board_size - 1)) : grid_width;
}

void UI::draw_button(const OptionButton &btn, Vector2 mouse_pos, bool is_active, int font_size) const {
    bool is_hovered = CheckCollisionPointRec(mouse_pos, btn.bounds);

    Color fill = is_active
        ? (Color){ 219, 169, 99, 255 }
        : (is_hovered ? (Color){ 60, 70, 95, 255 } : (Color){ 35, 42, 60, 255 });
    Color text_col = is_active ? (Color){ 20, 24, 35, 255 } : RAYWHITE;
    Color border_col = is_active ? RAYWHITE : (Color){ 100, 115, 140, 255 };

    // [HD UPGRADE]: Increase corner roundness segments (16) for smoother button borders
    DrawRectangleRounded(btn.bounds, 0.28f, 16, fill);
    DrawRectangleRoundedLinesEx(btn.bounds, 0.28f, 16, 1.5f, border_col);

    // [HD UPGRADE]: Center and render button text using the HD font
    if (has_custom_font) {
        Vector2 text_size = MeasureTextEx(ui_font, btn.label, (float)font_size, 1.0f);
        draw_text_hd(
            btn.label,
            btn.bounds.x + (btn.bounds.width - text_size.x) * 0.5f,
            btn.bounds.y + (btn.bounds.height - text_size.y) * 0.5f,
            (float)font_size,
            text_col
        );
    } else {
        int text_weight = MeasureText(btn.label, font_size);
        DrawText(
            btn.label,
            (int)(btn.bounds.x + (btn.bounds.width - text_weight) / 2),
            (int)(btn.bounds.y + (btn.bounds.height - font_size) / 2),
            font_size,
            text_col
        );
    }
}

void UI::draw(Board &board) {
    int board_size = board.get_size();
    int cell_size = get_cell_size(board);
    int actual_grid_width = (board_size - 1) * cell_size;
    Vector2 mouse_pos = GetMousePosition();

    // 1. Draw current turn info and Option buttons using the HD font
    if (board.get_turn() == 1) {
        draw_text_hd("Turn: BLACK", 30, 33, 24, RAYWHITE);
    } else {
        draw_text_hd("Turn: WHITE", 30, 33, 24, RAYWHITE);
    }

    draw_button(btn_undo,   mouse_pos, false);
    draw_button(btn_redo,   mouse_pos, false);
    draw_button(btn_reset,  mouse_pos, false);
    draw_button(btn_pass,   mouse_pos, false);
    draw_button(btn_save,   mouse_pos, false);
    draw_button(btn_back_from_play, mouse_pos, false);
    draw_button(btn_setting_play, mouse_pos, false);

    draw_text_hd(TextFormat("Black: %i", board.get_captured(1)), 1145, 28, 24, RAYWHITE);
    draw_text_hd(TextFormat("White: %i", board.get_captured(0)), 1145, 62, 24, RAYWHITE);

    // 2. [HD UPGRADE]: Draw realistic wooden board (Drop shadow + 3D border + HD wood grain texture)
    Rectangle board_rect = { 
        (float)(start_x - padding), 
        (float)(start_y - padding), 
        (float)(actual_grid_width + padding * 2), 
        (float)(actual_grid_width + padding * 2) 
    };

    // Drop shadow underneath the board for depth
    Rectangle board_shadow = { board_rect.x + 8.0f, board_rect.y + 10.0f, board_rect.width, board_rect.height };
    DrawRectangleRounded(board_shadow, 0.05f, 24, (Color){ 0, 0, 0, 110 });

    // Darker outer wood border
    Rectangle outer_border = { board_rect.x - 4.0f, board_rect.y - 4.0f, board_rect.width + 8.0f, board_rect.height + 8.0f };
    DrawRectangleRounded(outer_border, 0.05f, 24, board_border);
    DrawRectangleRounded(board_rect, 0.05f, 24, board_wood);

    // Overlay natural HD wood grain texture onto the board surface
    Rectangle wood_src = { 0.0f, 0.0f, (float)wood_board_tex.width, (float)wood_board_tex.height };
    Rectangle wood_dst = { board_rect.x + 6.0f, board_rect.y + 6.0f, board_rect.width - 12.0f, board_rect.height - 12.0f };
    DrawTexturePro(wood_board_tex, wood_src, wood_dst, { 0.0f, 0.0f }, 0.0f, WHITE);
    DrawRectangleRoundedLinesEx(board_rect, 0.05f, 24, 4.0f, board_border);

    // 3. [HD UPGRADE]: Draw grid lines with engraved groove effect (1 highlight line + 1 dark line)
    Color groove_highlight = { 245, 208, 145, 160 };
    for (int i = 0; i < board_size; i++) {
        float x_pos = (float)(start_x + i * cell_size);
        float y_pos = (float)(start_y + i * cell_size);

        // Thin bright line beside the groove for lighting depth
        DrawLineEx({ x_pos + 1.2f, (float)start_y }, { x_pos + 1.2f, (float)(start_y + actual_grid_width) }, 1.2f, groove_highlight);
        DrawLineEx({ (float)start_x, y_pos + 1.2f }, { (float)(start_x + actual_grid_width), y_pos + 1.2f }, 1.2f, groove_highlight);

        // Crisp dark main groove line
        DrawLineEx({ x_pos, (float)start_y }, { x_pos, (float)(start_y + actual_grid_width) }, 1.8f, line_color);
        DrawLineEx({ (float)start_x, y_pos }, { (float)(start_x + actual_grid_width), y_pos }, 1.8f, line_color);
    }

    // 3b. [HD UPGRADE]: Draw standard Star Points (Hoshi) based on board size
    std::vector<int> star_coords;
    if (board_size == 19) star_coords = { 3, 9, 15 };
    else if (board_size == 13) star_coords = { 3, 6, 9 };
    else if (board_size == 9) star_coords = { 2, 4, 6 };

    for (int sr : star_coords) {
        for (int sc : star_coords) {
            // For 9x9 and 13x13 boards, only draw the 4 corner star points and the center point (Tengen)
            if (board_size != 19) {
                int mid = board_size / 2;
                bool is_center = (sr == mid && sc == mid);
                bool is_corner = (sr != mid && sc != mid);
                if (!is_center && !is_corner) continue;
            }
            Vector2 star_pos = { (float)(start_x + sc * cell_size), (float)(start_y + sr * cell_size) };
            DrawCircleV(star_pos, std::max(4.0f, cell_size * 0.1f), line_color);
        }
    }

    // 4. [HD UPGRADE]: Draw glossy 3D stones with soft gradient drop shadows
    float radius = cell_size * 0.475f;
    for (int r = 0; r < board_size; r++) {
        for (int c = 0; c < board_size; c++) {
            int cell_state = board.get_cell(r, c);
            if (cell_state != 0) {
                float stone_x = (float)(start_x + c * cell_size);
                float stone_y = (float)(start_y + r * cell_size);

                // Soft gradient drop shadow offset toward bottom-right for 3D elevation
                float shadow_offset_x = radius * 0.10f;
                float shadow_offset_y = radius * 0.14f;
                DrawCircleGradient(
                    { stone_x + shadow_offset_x, stone_y + shadow_offset_y },
                    radius * 1.06f,
                    (Color){ 0, 0, 0, 125 },
                    (Color){ 0, 0, 0, 0 }
                );

                // Draw smoothed HD 3D stone texture (Trilinear + Mipmaps)
                Texture2D tex = (cell_state == 1) ? black_stone_tex : white_stone_tex;
                Rectangle source = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
                Rectangle dest = {
                    stone_x - radius,
                    stone_y - radius,
                    radius * 2.0f,
                    radius * 2.0f
                };
                DrawTexturePro(tex, source, dest, { 0.0f, 0.0f }, 0.0f, WHITE);
            }
        }
    }

    std::pair<int, int> ko_cell = check_ko_rule(board);
    // Draw hover ghost stone preview
    int hover_r = -1, hover_c = -1;
    if (mouse_pos.x >= start_x - cell_size / 2.0f &&
        mouse_pos.y >= start_y - cell_size / 2.0f &&
        screen_to_grid(mouse_pos, board, hover_r, hover_c)) {
        bool is_ko = (hover_r == ko_cell.first && hover_c == ko_cell.second);
        bool is_human_turn = (board.get_mode() == GameMode::PVP || board.get_turn() == 1);
        if (is_human_turn && board.get_cell(hover_r, hover_c) == 0 && !is_ko) {
            float ghost_x = (float)(start_x + hover_c * cell_size);
            float ghost_y = (float)(start_y + hover_r * cell_size);
            Texture2D ghost_tex = (board.get_turn() == 1) ? black_stone_tex : white_stone_tex;
            Rectangle source = { 0.0f, 0.0f, (float)ghost_tex.width, (float)ghost_tex.height };
            Rectangle dest = {
                ghost_x - radius,
                ghost_y - radius,
                radius * 2.0f,
                radius * 2.0f
            };
            DrawTexturePro(ghost_tex, source, dest, { 0.0f, 0.0f }, 0.0f, Fade(WHITE, 0.45f));
        }
    }

    if (board.in_board(ko_cell.first, ko_cell.second)) {
        float center_x = (float)(start_x + ko_cell.second * cell_size);
        float center_y = (float)(start_y + ko_cell.first * cell_size);
        float square_size = cell_size * 0.45f;
        Rectangle rect = {
            center_x - square_size / 2.0f,
            center_y - square_size / 2.0f,
            square_size,
            square_size
        };
        DrawRectangleLinesEx(rect, 2.5f, RED);
    }
}

void UI::draw_starting(const Board &board) {
    Vector2 mouse_pos = GetMousePosition();

    draw_text_hd(TITLE, 310, 280, 150, RAYWHITE);
    draw_text_hd("Press H to exit", 1120, 920, 20, RAYWHITE);

    draw_button(btn_new_game, mouse_pos, false, 36);
    draw_button(btn_load_game, mouse_pos, false, 36);
    draw_button(btn_config, mouse_pos, false, 36);
}

void UI::draw_mode() {
    Vector2 mouse_pos = GetMousePosition();

    draw_text_hd("CHOOSE GAME MODE", 390, 120, 36, RAYWHITE);
    draw_button(btn_pvp, mouse_pos, CheckCollisionPointRec(mouse_pos, btn_pvp.bounds), 28);
    draw_button(btn_pve, mouse_pos, CheckCollisionPointRec(mouse_pos, btn_pve.bounds), 28);
}

void UI::draw_difficulty() {
    Vector2 mouse_pos = GetMousePosition();

    draw_text_hd("CHOOSE DIFFICULTY", 390, 120, 36, RAYWHITE);
    draw_button(btn_easy,   mouse_pos, CheckCollisionPointRec(mouse_pos, btn_easy.bounds), 28);
    draw_button(btn_medium, mouse_pos, CheckCollisionPointRec(mouse_pos, btn_medium.bounds), 28);
    draw_button(btn_hard,   mouse_pos, CheckCollisionPointRec(mouse_pos, btn_hard.bounds), 28);
}

void UI::draw_choosing_board_size(const Board &board) {
    Vector2 mouse_pos = GetMousePosition();

    draw_text_hd("CHOOSE BOARD SIZE", 390, 120, 36, RAYWHITE);
    draw_button(btn_size9,  mouse_pos, CheckCollisionPointRec(mouse_pos, btn_size9.bounds), 28);
    draw_button(btn_size13, mouse_pos, CheckCollisionPointRec(mouse_pos, btn_size13.bounds), 28);
    draw_button(btn_size19, mouse_pos, CheckCollisionPointRec(mouse_pos, btn_size19.bounds), 28);
}

void UI::draw_ending(const Board &board) {
    Vector2 mouse_pos = GetMousePosition();

    std::pair<double, double> final_score = scoring(board);
    std::cerr << final_score.first << " " << final_score.second << '\n';

    if (final_score.first == final_score.second) {
        draw_text_hd(TextFormat("Tie"), 500, 33, 28, RAYWHITE);
    }
    else if(final_score.first < final_score.second) {
        draw_text_hd(TextFormat("White wins by %.1f", final_score.second - final_score.first), 500, 33, 28, RAYWHITE);
    }
    else {
        draw_text_hd(TextFormat("Black wins by %.1f", final_score.first - final_score.second), 500, 33, 28, RAYWHITE);
    }

    draw_button(btn_back_from_end, mouse_pos, false, 36);
}

void UI::draw_config() {
    Vector2 mouse_pos = GetMousePosition();

    draw_text_hd("SETTINGS", 390, 120, 40, RAYWHITE);

    //Audio
    draw_text_hd("Master Volume", 320, 250, 26, RAYWHITE);
    draw_button(btn_master_down, mouse_pos, false, 24);
    draw_text_hd(is_muted_master ? "Muted" : TextFormat("%d%%", master_volume),
                 660, 252, 24, is_muted_master ? RED : RAYWHITE);
    draw_button(btn_master_up, mouse_pos, false, 24);
    draw_button(btn_master_mute, mouse_pos, is_muted_master, 22);

    draw_text_hd("Music Volume", 320, 340, 26, RAYWHITE);
    draw_button(btn_music_down, mouse_pos, false, 24);
    draw_text_hd(is_muted_music ? "Muted" : TextFormat("%d%%", music_volume),
                 660, 342, 24, is_muted_music ? RED : RAYWHITE);
    draw_button(btn_music_up, mouse_pos, false, 24);
    draw_button(btn_music_mute, mouse_pos, is_muted_music, 22);

    draw_text_hd("Sound Effects", 320, 430, 26, RAYWHITE);
    draw_button(btn_sound_down, mouse_pos, false, 24);
    draw_text_hd(is_muted_sound ? "Muted" : TextFormat("%d%%", sound_volume),
                 660, 432, 24, is_muted_sound ? RED : RAYWHITE);
    draw_button(btn_sound_up, mouse_pos, false, 24);
    draw_button(btn_sound_mute, mouse_pos, is_muted_sound, 22);

    draw_button(btn_change_music, mouse_pos, music_dropdown_open, 22);
    if (music_dropdown_open) {
        for (int i = 0; i < 3; i++) {
            draw_button(opt_music[i], mouse_pos, selected_music == i + 1, 22);
        }
    }

    draw_button(btn_back_config, mouse_pos, false, 26);
}

void UI::draw_loading() {
    Vector2 mouse_pos = GetMousePosition();

    draw_text_hd("Choose a slot to load", 390, 120, 32, RAYWHITE);

    for (int i = 0; i < 3; i++) {
        draw_button(btn_load_slots[i], mouse_pos, false, 28);
    }
    draw_button(btn_back_load, mouse_pos, false, 24);
}

void UI::draw_saving() {
    Vector2 mouse_pos = GetMousePosition();

    draw_text_hd("SAVE GAME", 390, 120, 36, RAYWHITE);
    if (overwrite_slot != -1) {
        draw_text_hd(TextFormat("Overwrite the saved game in Slot %i?", overwrite_slot + 1), 390, 250, 24, RAYWHITE);
        draw_button(btn_confirm_overwrite, mouse_pos, false, 24);
        draw_button(btn_cancel_overwrite, mouse_pos, false, 24);
        return;
    }
    draw_text_hd("Choose a slot to save.", 390, 175, 22, RAYWHITE);

    for (int i = 0; i < 3; i++) {
        draw_button(btn_save_slots[i], mouse_pos, false, 28);
    }
    draw_button(btn_back_save, mouse_pos, false, 24);
}

bool UI::screen_to_grid(Vector2 mouse_pos, const Board &board, int &row, int &col) const {
    int board_size = board.get_size();
    int cell_size = get_cell_size(board);

    col = (int)((mouse_pos.x - start_x + cell_size / 2.0f) / cell_size);
    row = (int)((mouse_pos.y - start_y + cell_size / 2.0f) / cell_size);

    return (row >= 0 && row < board_size && col >= 0 && col < board_size);
}

bool UI::handle_option_click(Vector2 mouse_pos, GamePhase &phase, Board &board) {
    switch (phase) {
        case GamePhase::SAVING:
            if (overwrite_slot != -1) {
                if (CheckCollisionPointRec(mouse_pos, btn_cancel_overwrite.bounds)) {
                    Audio::play(SFX::CLICK);
                    overwrite_slot = -1;
                } else if (CheckCollisionPointRec(mouse_pos, btn_confirm_overwrite.bounds)) {
                    std::string path = "saves/slot_" + std::to_string(overwrite_slot + 1) + ".json";
                    overwrite_slot = -1;
                    if (board.save_game(path)) {
                        Audio::play(SFX::POSITIVE);
                        phase = GamePhase::PLAYING;
                    } else {
                        Audio::play(SFX::NEGATIVE);
                        std::cerr << "Could not save to " << path << '\n';
                    }
                }
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_back_save.bounds)) {
                Audio::play(SFX::CLICK);
                phase = GamePhase::PLAYING;
                return true;
            }
            for (int i = 0; i < 3; i++) {
                if (CheckCollisionPointRec(mouse_pos, btn_save_slots[i].bounds)) {
                    std::string path = "saves/slot_" + std::to_string(i + 1) + ".json";
                    if (std::filesystem::exists(path)) {
                        overwrite_slot = i;
                        return true;
                    }
                    if (board.save_game(path)) {
                        Audio::play(SFX::POSITIVE);
                        phase = GamePhase::PLAYING;
                    } else {
                        Audio::play(SFX::NEGATIVE);
                        std::cerr << "Could not save to " << path << '\n';
                    }
                    return true;
                }
            }
            break;
        case GamePhase::LOADING:
            if (CheckCollisionPointRec(mouse_pos, btn_back_load.bounds)) {
                Audio::play(SFX::CLICK);
                phase = GamePhase::START;
                return true;
            }
            for (int i = 0; i < 3; i++) {
                if (CheckCollisionPointRec(mouse_pos, btn_load_slots[i].bounds)) {
                    std::string path = "saves/slot_" + std::to_string(i + 1) + ".json";
                    Board loaded = board;
                    if (loaded.load_game(path)) {
                        Audio::play(SFX::POSITIVE);
                        board = std::move(loaded);
                        phase = board.get_pass() == 2 ? GamePhase::GAMEOVER : GamePhase::PLAYING;
                    } else {
                        Audio::play(SFX::NEGATIVE);
                        std::cerr << "Could not load " << path << '\n';
                    }
                    return true;
                }
            }
            break;
        case GamePhase::START:
            if (CheckCollisionPointRec(mouse_pos, btn_new_game.bounds)) {
                Audio::play(SFX::CLICK);
                phase = GamePhase::CHOOSING_SIZE;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_config.bounds)) {
                Audio::play(SFX::CLICK);
                previous_phase = GamePhase::START;
                phase = GamePhase::CONFIG;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_load_game.bounds)) {
                Audio::play(SFX::CLICK);
                phase = GamePhase::LOADING;
                return true;
            }
            break;
        case GamePhase::CHOOSING_SIZE:
            if (CheckCollisionPointRec(mouse_pos, btn_size9.bounds)) {
                Audio::play(SFX::CLICK);
                board.reset(9);
                phase = GamePhase::MODE;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_size13.bounds)) {
                Audio::play(SFX::CLICK);
                board.reset(13);
                phase = GamePhase::MODE;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_size19.bounds)) {
                Audio::play(SFX::CLICK);
                board.reset(19);
                phase = GamePhase::MODE;
                return true;
            }
            break;
        case GamePhase::MODE:
            if (CheckCollisionPointRec(mouse_pos, btn_pvp.bounds)) {
                Audio::play(SFX::CLICK);
                board.set_mode(GameMode::PVP);
                phase = GamePhase::PLAYING;
                return true;
            } 
            if (CheckCollisionPointRec(mouse_pos, btn_pve.bounds)) {
                Audio::play(SFX::CLICK);
                board.set_mode(GameMode::PVE);
                phase = GamePhase::DIFFICULTY;
                return true;
            }
            break;
        case GamePhase::DIFFICULTY:
            if (CheckCollisionPointRec(mouse_pos, btn_easy.bounds)) {
                Audio::play(SFX::CLICK);
                board.set_difficulty(Difficulty::EASY);
                phase = GamePhase::PLAYING;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_medium.bounds)) {
                Audio::play(SFX::CLICK);
                board.set_difficulty(Difficulty::MEDIUM);
                phase = GamePhase::PLAYING;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_hard.bounds)) {
                Audio::play(SFX::CLICK);
                board.set_difficulty(Difficulty::HARD);
                phase = GamePhase::PLAYING;
                return true;
            }
            break;
        case GamePhase::PLAYING:
            if (CheckCollisionPointRec(mouse_pos, btn_back_from_play.bounds)) {
                Audio::play(SFX::CLICK);
                phase = GamePhase::START;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_undo.bounds)) {
                Audio::play(SFX::CLICK);
                board.undo_move(true);
                if (board.get_mode() == GameMode::PVE) {
                    board.undo_move(true);
                }
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_redo.bounds)) {
                Audio::play(SFX::CLICK);
                board.redo_move();
                if (board.get_mode() == GameMode::PVE) {
                    board.redo_move();
                }
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_reset.bounds)) {
                Audio::play(SFX::CLICK);
                board.reset(board.get_size());
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_pass.bounds)) {
                Audio::play(SFX::CLICK);
                board.toggle_turn();
                board.pass_turn();
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_save.bounds)) {
                Audio::play(SFX::CLICK);
                overwrite_slot = -1;
                phase = GamePhase::SAVING;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_setting_play.bounds)) {
                Audio::play(SFX::CLICK);
                previous_phase = GamePhase::PLAYING;
                phase = GamePhase::CONFIG;
                return true;
            }
            break;
        case GamePhase::CONFIG:
            if (CheckCollisionPointRec(mouse_pos, btn_change_music.bounds)) {
                Audio::play(SFX::CLICK);
                music_dropdown_open = 1 - music_dropdown_open;
                return true;
            }
            if (music_dropdown_open) {
                music_dropdown_open = false;
                for (int i = 0; i < 3; i++) {
                    if (CheckCollisionPointRec(mouse_pos, opt_music[i].bounds)) {
                        Audio::play(SFX::CLICK);
                        if (selected_music != i + 1) {
                            selected_music = i + 1;
                            selected_music = i + 1;
                            Audio::change_music(selected_music);
                        }
                        return true;
                    }
                }
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_master_down.bounds)) {
                Audio::play(SFX::CLICK);
                master_volume = std::max(0, master_volume - 10);
                if (master_volume == 0) is_muted_master = true;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_master_up.bounds)) {
                Audio::play(SFX::CLICK);
                master_volume = std::min(100, master_volume + 10);
                is_muted_master = false;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_master_mute.bounds)) {
                Audio::play(SFX::CLICK);
                is_muted_master = !is_muted_master;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_music_down.bounds)) {
                Audio::play(SFX::CLICK);
                music_volume = std::max(0, music_volume - 10);
                if (music_volume == 0) is_muted_music = true;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_music_up.bounds)) {
                Audio::play(SFX::CLICK);
                music_volume = std::min(100, music_volume + 10);
                is_muted_music = false;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_music_mute.bounds)) {
                Audio::play(SFX::CLICK);
                is_muted_music = !is_muted_music;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_sound_down.bounds)) {
                Audio::play(SFX::CLICK);
                sound_volume = std::max(0, sound_volume - 10);
                if (sound_volume == 0) is_muted_sound = true;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_sound_up.bounds)) {
                Audio::play(SFX::CLICK);
                sound_volume = std::min(100, sound_volume + 10);
                is_muted_sound = false;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_sound_mute.bounds)) {
                Audio::play(SFX::CLICK);
                is_muted_sound = !is_muted_sound;
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_back_config.bounds)) {
                Audio::play(SFX::CLICK);
                phase = previous_phase;
                return true;
            }
            break;
        case GamePhase::GAMEOVER:
            if (CheckCollisionPointRec(mouse_pos, btn_back_from_end.bounds)) {
                Audio::play(SFX::CLICK);
                board.reset(board.get_size());
                phase = GamePhase::START;
            }
            break;
        default:
            break;
    } 
    return false;
}

bool UI::save_setting(const std::string &path) {
    try {
        json data = {
            {"selected_music", this->selected_music},
            {"sound_volume", this->sound_volume},
            {"music_volume", this->music_volume},
            {"master_volume", this->master_volume},
            {"is_muted_sound", this->is_muted_sound},
            {"is_muted_music", this->is_muted_music},
            {"is_muted_master", this->is_muted_master}
        };
        
        std::ofstream stream(path);
        stream << data.dump(4);
        stream.close();

        return true;
    }
    catch(std::exception) {
        return false;
    }
}

bool UI::load_setting(const std::string &path) {
    try {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
        std::ifstream stream(path);

        if (!stream.is_open()) {
            json default_data = {
                {"selected_music", 3},
                {"sound_volume", 80},
                {"music_volume", 50},
                {"master_volume", 80},
                {"is_muted_sound", false},
                {"is_muted_music", false},
                {"is_muted_master", false}
            };
            
            std::ofstream outstream(path);
            outstream << default_data.dump(4);
            outstream.close();
        }
        
        stream.clear();
        stream.open(path);
        
        json data;
        stream >> data;
        stream.close();
        
        data.at("selected_music").get_to(this->selected_music);
        data.at("sound_volume").get_to(this->sound_volume);
        data.at("music_volume").get_to(this->music_volume);
        data.at("master_volume").get_to(this->master_volume);
        data.at("is_muted_sound").get_to(this->is_muted_sound);
        data.at("is_muted_music").get_to(this->is_muted_music);            
        data.at("is_muted_master").get_to(this->is_muted_master);

        return true;
    } 
    catch(std::exception) {
        return false;
    }
}