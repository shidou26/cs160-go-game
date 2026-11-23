// AI-assisted: see AI09, AI10, AI13, AI14, AI31, AI34, AI35, AI36, AI39, AI40, AI41

#include <iostream>
#include <utility>
#include <filesystem>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string> 

#include "../include/ui.h"
#include "../include/helper.h"
#include "../include/rule.h"
#include "../include/score.h"
#include "../include/board.h"
#include "../include/audio.h"

namespace {

const Rectangle PICKER_PANEL_RECT   = { 190.0f, 474.0f, 900.0f, 344.0f };
const Rectangle PICKER_LIST_RECT    = { 214.0f, 496.0f, 340.0f, 300.0f };
const Rectangle PICKER_PREVIEW_RECT = { 578.0f, 496.0f, 488.0f, 300.0f };
const float PICKER_ITEM_H   = 52.0f;
const float PICKER_ITEM_GAP = 10.0f;

bool is_image_ext(const std::string &ext) {
    std::string lower = ext;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return (lower == ".png" || lower == ".jpg" || lower == ".jpeg");
}

Texture2D load_texture_hd(const std::string &path) {
    if (!std::filesystem::exists(path)) return Texture2D{};
    Texture2D tex = LoadTexture(path.c_str());
    if (tex.id != 0) {
        GenTextureMipmaps(&tex);
        SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
    }
    return tex;
}

Color parse_json_color(const json &j, const char *key, Color fallback) {
    if (!j.contains(key) || !j[key].is_array() || j[key].size() < 4) return fallback;
    return (Color){
        (unsigned char)std::clamp(j[key][0].get<int>(), 0, 255),
        (unsigned char)std::clamp(j[key][1].get<int>(), 0, 255),
        (unsigned char)std::clamp(j[key][2].get<int>(), 0, 255),
        (unsigned char)std::clamp(j[key][3].get<int>(), 0, 255)
    };
}

void load_theme_colors(const std::filesystem::path &dir, ThemeOption &opt) {
    std::filesystem::path cfg = dir / "colors.json";
    if (!std::filesystem::exists(cfg)) return;
    try {
        std::ifstream f(cfg);
        json data;
        f >> data;
        opt.bg_fallback       = parse_json_color(data, "bg_fallback",       opt.bg_fallback);
        opt.text_color        = parse_json_color(data, "text_color",        opt.text_color);
        opt.active_text_color = parse_json_color(data, "active_text_color", opt.active_text_color);
        opt.accent_color      = parse_json_color(data, "accent_color",      opt.accent_color);
        opt.line_color        = parse_json_color(data, "line_color",        opt.line_color);
        opt.board_border      = parse_json_color(data, "board_border",      opt.board_border);
    } catch (...) {}
}

Texture2D create_hd_stone_texture(bool is_black) {
    const int tex_size = 512;
    Image img = GenImageColor(tex_size, tex_size, BLANK);
    Color *pixels = (Color *)img.data;

    float center = (tex_size - 1) / 2.0f;
    float max_r = tex_size * 0.47f;

    for (int y = 0; y < tex_size; y++) {
        for (int x = 0; x < tex_size; x++) {
            float dx = (x - center) / max_r;
            float dy = (y - center) / max_r;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist > 1.0f) continue;

            // Sub-pixel anti-aliasing along the circle edge for crisp, smooth borders
            float edge_alpha = std::clamp((1.0f - dist) * (max_r / 2.0f), 0.0f, 1.0f);

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
    trophy_tex = LoadTexture("assets/images/golden_trophy.png");
    if (trophy_tex.id != 0) {
        SetTextureFilter(trophy_tex, TEXTURE_FILTER_BILINEAR);
    }

    title_font = LoadFontEx("assets/fonts/Cinzel-Bold.ttf", 150, nullptr, 0);
    heading_font = LoadFontEx("assets/fonts/Pretendard-SemiBold.otf", 40, nullptr, 0);
    button_font = LoadFontEx("assets/fonts/Pretendard-Medium.otf", 36, nullptr, 0);
    label_font = LoadFontEx("assets/fonts/Pretendard-SemiBold.otf", 26, nullptr, 0);
    body_font = LoadFontEx("assets/fonts/Pretendard-Regular.otf", 24, nullptr, 0);
    //algorithm to render better at different sizes
    SetTextureFilter(heading_font.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(button_font.texture, TEXTURE_FILTER_BILINEAR); 
    SetTextureFilter(body_font.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(label_font.texture, TEXTURE_FILTER_BILINEAR); 
    
    grid_width = DEFAULT_GRID_WIDTH;
    start_x = (GetScreenWidth() - grid_width) / 5.0f;
    start_y = (GetScreenHeight() - grid_width) / 2.0f;
    padding = PADDING;

    //Init option buttons for starting phase
    btn_new_game  = {{ 140, 590, 1000, 80 }, "New Game"};
    btn_load_game = {{ 140, 690, 1000, 80 }, "Load Game"};
    btn_config    = {{ 140, 790, 1000, 80 }, "Settings"};

    //Init option buttons for size phase
    btn_size9  = { { 390, 250, 500, 100 }, "9x9" };
    btn_size13 = { { 390, 400, 500, 100 }, "13x13" };
    btn_size19 = { { 390, 550, 500, 100 }, "19x19" };

    //Init option buttons for choosing mode
    btn_pvp = { { 390, 250, 500, 100 }, "Play against Friend" };
    btn_pve = { { 390, 400, 500, 100 }, "Play against Seol" };

    //Init option buttons for choosing difficulty
    btn_easy = { { 390, 250, 500, 100 }, "Easy" };
    btn_medium = { { 390, 400, 500, 100 }, "Medium" };
    btn_hard = { { 390, 550, 500, 100 }, "Hard" };

    //Init option buttons for playing phase
    const float play_button_width = 300.0f;
    const float play_button_height = 50.0f;
    const float play_button_step = 64.0f;
    const float play_button_x = GetScreenWidth() - play_button_width - 30.0f;
    // Align the bottom of the last button with the board's outer border.
    const float play_button_y = start_y + grid_width + padding + 4.0f
                              - play_button_height - 6 * play_button_step;
    btn_pass           = { { play_button_x, play_button_y,                        play_button_width, play_button_height }, "Pass" };
    btn_undo           = { { play_button_x, play_button_y + play_button_step,     play_button_width, play_button_height }, "Undo" };
    btn_redo           = { { play_button_x, play_button_y + 2 * play_button_step, play_button_width, play_button_height }, "Redo" };
    btn_save           = { { play_button_x, play_button_y + 3 * play_button_step, play_button_width, play_button_height }, "Save" };
    btn_setting_play   = { { play_button_x, play_button_y + 4 * play_button_step, play_button_width, play_button_height }, "Settings" };
    btn_reset          = { { play_button_x, play_button_y + 5 * play_button_step, play_button_width, play_button_height }, "Reset" };
    btn_back_from_play = { { play_button_x, play_button_y + 6 * play_button_step, play_button_width, play_button_height }, "Back" };

    //Init option buttons for config (Settings) phase — centered x=190..1090
    btn_master_down = { { 430, 146, 60, 44 }, "-" };
    btn_master_up   = { { 610, 146, 60, 44 }, "+" };
    btn_master_mute = { { 690, 146, 120, 44 }, "Mute" };
    btn_music_down  = { { 430, 206, 60, 44 }, "-" };
    btn_music_up    = { { 610, 206, 60, 44 }, "+" };
    btn_music_mute  = { { 690, 206, 120, 44 }, "Mute" };
    btn_sound_down  = { { 430, 266, 60, 44 }, "-" };
    btn_sound_up    = { { 610, 266, 60, 44 }, "+" };
    btn_sound_mute  = { { 690, 266, 120, 44 }, "Mute" };
    btn_music_prev  = { { 430, 326, 60, 44 }, "<" };
    btn_music_next  = { { 750, 326, 60, 44 }, ">" };
    btn_tab_theme   = { { 430, 398, 205, 50 }, "Game Theme" };
    btn_tab_board   = { { 655, 398, 205, 50 }, "Board" };
    btn_tab_stones  = { { 880, 398, 210, 50 }, "Stones" };
    btn_back_config = { { 390, 846, 500, 60 }, "Ok" };

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
    btn_back_from_end = { { (GetScreenWidth() - 360.0f) / 2.0f,
                           (GetScreenHeight() - 420.0f) / 2.0f + 324.0f,
                           360, 64 }, "Return to title screen"};

    // Auto-scan custom PNG themes, boards, and stone packs
    scan_custom_assets();
    apply_selected_assets();
}

void UI::scan_custom_assets() {
    // 1. Game Themes (auto-scanned from assets/themes/; "Classic" placed first)
    theme_options.clear();
    const std::string themes_dir = "assets/themes";
    bool found_classic = false;
    if (std::filesystem::exists(themes_dir)) {
        std::vector<std::filesystem::path> dirs;
        for (const auto &entry : std::filesystem::directory_iterator(themes_dir)) {
            if (entry.is_directory()) dirs.push_back(entry.path());
        }
        std::sort(dirs.begin(), dirs.end(), [](const auto &a, const auto &b) {
            if (a.filename() == "Classic") return true;
            if (b.filename() == "Classic") return false;
            return a.filename() < b.filename();
        });
        for (const auto &dir : dirs) {
            ThemeOption opt;
            opt.name           = dir.filename().string();
            opt.bg_tex         = load_texture_hd((dir / "bg.png").string());
            opt.btn_normal_tex = load_texture_hd((dir / "btn_normal.png").string());
            opt.btn_hover_tex  = load_texture_hd((dir / "btn_hover.png").string());
            opt.btn_active_tex = load_texture_hd((dir / "btn_active.png").string());
            load_theme_colors(dir, opt);
            if (opt.name == "Classic") found_classic = true;
            theme_options.push_back(opt);
        }
    }
    if (!found_classic) {
        ThemeOption fallback;
        fallback.name = "Classic";
        theme_options.insert(theme_options.begin(), fallback);
    }

    // 2. Boards (index 0 = built-in procedural Kaya Wood)
    board_options.clear();
    board_options.push_back({ "Kaya Wood", wood_board_tex });
    const std::string boards_dir = "assets/boards";
    if (std::filesystem::exists(boards_dir)) {
        std::vector<std::filesystem::path> files;
        for (const auto &entry : std::filesystem::directory_iterator(boards_dir)) {
            if (entry.is_regular_file() && is_image_ext(entry.path().extension().string())) {
                files.push_back(entry.path());
            }
        }
        std::sort(files.begin(), files.end());
        for (const auto &file : files) {
            Texture2D tex = load_texture_hd(file.string());
            if (tex.id != 0) {
                board_options.push_back({ file.stem().string(), tex });
            }
        }
    }

    // 3. Stones (index 0 = built-in procedural 3D Glossy)
    stone_options.clear();
    stone_options.push_back({ "3D Glossy", black_stone_tex, white_stone_tex });
    const std::string stones_dir = "assets/stones";
    if (std::filesystem::exists(stones_dir)) {
        std::vector<std::filesystem::path> dirs;
        for (const auto &entry : std::filesystem::directory_iterator(stones_dir)) {
            if (entry.is_directory()) dirs.push_back(entry.path());
        }
        std::sort(dirs.begin(), dirs.end());
        for (const auto &dir : dirs) {
            Texture2D b_tex = load_texture_hd((dir / "black.png").string());
            Texture2D w_tex = load_texture_hd((dir / "white.png").string());
            if (b_tex.id != 0 && w_tex.id != 0) {
                stone_options.push_back({ dir.filename().string(), b_tex, w_tex });
            } else {
                if (b_tex.id != 0) UnloadTexture(b_tex);
                if (w_tex.id != 0) UnloadTexture(w_tex);
            }
        }
    }
}

void UI::apply_selected_assets() {
    selected_theme_idx = 0;
    for (int i = 0; i < (int)theme_options.size(); i++) {
        if (theme_options[i].name == selected_theme) {
            selected_theme_idx = i;
            break;
        }
    }
    if (!theme_options.empty()) selected_theme = theme_options[selected_theme_idx].name;

    selected_board_idx = 0;
    for (int i = 0; i < (int)board_options.size(); i++) {
        if (board_options[i].name == selected_board) {
            selected_board_idx = i;
            break;
        }
    }
    if (!board_options.empty()) {
        selected_board = board_options[selected_board_idx].name;
        wood_board_tex = board_options[selected_board_idx].tex;
    }

    selected_stone_idx = 0;
    for (int i = 0; i < (int)stone_options.size(); i++) {
        if (stone_options[i].name == selected_stones) {
            selected_stone_idx = i;
            break;
        }
    }
    if (!stone_options.empty()) {
        selected_stones = stone_options[selected_stone_idx].name;
        black_stone_tex = stone_options[selected_stone_idx].black_tex;
        white_stone_tex = stone_options[selected_stone_idx].white_tex;
    }
}

const ThemeOption &UI::active_theme() const {
    static const ThemeOption default_theme{ "Classic" };
    if (selected_theme_idx >= 0 && selected_theme_idx < (int)theme_options.size()) {
        return theme_options[selected_theme_idx];
    }
    return default_theme;
}

void UI::draw_background() const {
    const Texture2D &bg = active_theme().bg_tex;
    if (bg.id != 0) {
        Rectangle src = { 0.0f, 0.0f, (float)bg.width, (float)bg.height };
        Rectangle dst = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
        DrawTexturePro(bg, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
    }
}

Color UI::get_background_color() const {
    return active_theme().bg_fallback;
}

void UI::unload() {
    for (auto &t : theme_options) {
        if (t.bg_tex.id != 0) UnloadTexture(t.bg_tex);
        if (t.btn_normal_tex.id != 0) UnloadTexture(t.btn_normal_tex);
        if (t.btn_hover_tex.id != 0) UnloadTexture(t.btn_hover_tex);
        if (t.btn_active_tex.id != 0) UnloadTexture(t.btn_active_tex);
    }
    for (auto &b : board_options) {
        if (b.tex.id != 0) UnloadTexture(b.tex);
    }
    for (auto &s : stone_options) {
        if (s.black_tex.id != 0) UnloadTexture(s.black_tex);
        if (s.white_tex.id != 0) UnloadTexture(s.white_tex);
    }
    if (trophy_tex.id != 0) UnloadTexture(trophy_tex);
    UnloadFont(title_font);
    UnloadFont(heading_font);
    UnloadFont(button_font);
    UnloadFont(label_font);
    UnloadFont(body_font);
}

int UI::get_cell_size(const Board &board) const {
    int board_size = board.get_size();
    return (board_size > 1) ? (grid_width / (board_size - 1)) : grid_width;
}

void UI::draw_button(const OptionButton &btn, Vector2 mouse_pos, bool is_active, int font_size, bool interactive) const {
    bool is_hovered = interactive && CheckCollisionPointRec(mouse_pos, btn.bounds);
    const ThemeOption &t = active_theme();

    Color text_col = is_active ? t.active_text_color : t.text_color;

    // Draw custom PNG button texture from active theme (fallback to simple rounded box if missing)
    Texture2D custom_tex{};
    if (is_active && t.btn_active_tex.id != 0) custom_tex = t.btn_active_tex;
    else if (is_hovered && t.btn_hover_tex.id != 0) custom_tex = t.btn_hover_tex;
    else if (t.btn_normal_tex.id != 0) custom_tex = t.btn_normal_tex;

    if (custom_tex.id != 0) {
        Rectangle src = { 0.0f, 0.0f, (float)custom_tex.width, (float)custom_tex.height };
        DrawTexturePro(custom_tex, src, btn.bounds, { 0.0f, 0.0f }, 0.0f, WHITE);
    } else {
        Color fill = is_active ? t.accent_color : (is_hovered ? (Color){ 60, 70, 95, 255 } : (Color){ 35, 42, 60, 255 });
        DrawRectangleRounded(btn.bounds, 0.28f, 16, fill);
        DrawRectangleRoundedLinesEx(btn.bounds, 0.28f, 16, 1.5f, is_active ? RAYWHITE : (Color){ 100, 115, 140, 255 });
    }

    Vector2 text_size = MeasureTextEx(button_font, btn.label, (float)font_size, 1.0f);
    DrawTextEx(
        button_font,
        btn.label,
        {btn.bounds.x + (btn.bounds.width - text_size.x) / 2.0f,
        btn.bounds.y + (btn.bounds.height - text_size.y) / 2.0f},
        (float)font_size,
        1.0f,
        text_col
    );
}

void UI::draw(Board &board, bool interactive) {
    draw_background();
    int board_size = board.get_size();
    int cell_size = get_cell_size(board);
    int actual_grid_width = (board_size - 1) * cell_size;
    Vector2 mouse_pos = GetMousePosition();

    float sidebar_x = btn_pass.bounds.x;
    float sidebar_width = btn_pass.bounds.width;
    float sidebar_top = start_y - padding - 4.0f;

    DrawTextEx(heading_font,
               board.get_turn() == 1 ? "Turn: BLACK" : "Turn: WHITE",
               {sidebar_x, sidebar_top}, 24, 1.0f, RAYWHITE);

    for (int player = 0; player < 2; ++player) {
        Rectangle info_box = {
            sidebar_x, sidebar_top + 44.0f + player * 162.0f,
            sidebar_width, 150.0f
        };
        Color fill = (Color){25, 30, 42, 255};
        Color text_color = active_theme().text_color;
        Color border_color = active_theme().accent_color;

        DrawRectangleRounded(info_box, 0.15f, 16, fill);
        DrawRectangleRoundedLinesEx(info_box, 0.15f, 16, 2.0f, border_color);

        const float text_x = info_box.x + 16.0f;
        int seconds = static_cast<int>(board.get_timer(player));
        DrawTextEx(heading_font, player == 0 ? "BLACK" : "WHITE",
                   {text_x, info_box.y + 16.0f}, 26, 1.0f, text_color);
        DrawTextEx(label_font,
                   TextFormat("Using Time: %02d:%02d", seconds / 60, seconds % 60),
                   {text_x, info_box.y + 62.0f}, 22, 1.0f, text_color);
        DrawTextEx(label_font,
                   TextFormat("Prisoned Stones: %i", board.get_captured(1 - player)),
                   {text_x, info_box.y + 104.0f}, 22, 1.0f, text_color);
    }

    draw_button(btn_pass, mouse_pos, false, 18, interactive);
    draw_button(btn_undo, mouse_pos, false, 18, interactive);
    draw_button(btn_redo, mouse_pos, false, 18, interactive);
    draw_button(btn_save, mouse_pos, false, 18, interactive);
    draw_button(btn_setting_play, mouse_pos, false, 18, interactive);
    draw_button(btn_reset, mouse_pos, false, 18, interactive);
    draw_button(btn_back_from_play, mouse_pos, false, 18, interactive);

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
    const ThemeOption &theme = active_theme();
    Rectangle outer_border = { board_rect.x - 4.0f, board_rect.y - 4.0f, board_rect.width + 8.0f, board_rect.height + 8.0f };
    DrawRectangleRounded(outer_border, 0.05f, 24, theme.board_border);
    DrawRectangleRounded(board_rect, 0.05f, 24, theme.accent_color);

    // Overlay natural HD wood grain texture onto the board surface
    Rectangle wood_src = { 0.0f, 0.0f, (float)wood_board_tex.width, (float)wood_board_tex.height };
    Rectangle wood_dst = { board_rect.x + 6.0f, board_rect.y + 6.0f, board_rect.width - 12.0f, board_rect.height - 12.0f };
    DrawTexturePro(wood_board_tex, wood_src, wood_dst, { 0.0f, 0.0f }, 0.0f, WHITE);
    DrawRectangleRoundedLinesEx(board_rect, 0.05f, 24, 4.0f, theme.board_border);

    // 3. [HD UPGRADE]: Draw grid lines with engraved groove effect (1 highlight line + 1 dark line)
    Color groove_highlight = { 245, 208, 145, 160 };
    for (int i = 0; i < board_size; i++) {
        float x_pos = (float)(start_x + i * cell_size);
        float y_pos = (float)(start_y + i * cell_size);

        // Thin bright line beside the groove for lighting depth
        DrawLineEx({ x_pos + 1.2f, (float)start_y }, { x_pos + 1.2f, (float)(start_y + actual_grid_width) }, 1.2f, groove_highlight);
        DrawLineEx({ (float)start_x, y_pos + 1.2f }, { (float)(start_x + actual_grid_width), y_pos + 1.2f }, 1.2f, groove_highlight);

        // Crisp dark main groove line
        DrawLineEx({ x_pos, (float)start_y }, { x_pos, (float)(start_y + actual_grid_width) }, 1.8f, theme.line_color);
        DrawLineEx({ (float)start_x, y_pos }, { (float)(start_x + actual_grid_width), y_pos }, 1.8f, theme.line_color);
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
            DrawCircleV(star_pos, std::max(4.0f, cell_size * 0.1f), theme.line_color);
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

    //Draw hover ghost stone preview
    if (interactive) {
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

    if (GetTime() < capture_notify_until) {
        const char *msg = "Stone Captured!";
        Vector2 sz = MeasureTextEx(heading_font, msg, 28, 1.0f);
        Rectangle box = {
            (GetScreenWidth() - (sz.x + 40.0f)) * 0.5f,
            32.0f,
            sz.x + 40.0f,
            52.0f
        };
        // Hiệu ứng mờ dần (fade out) trong 0.5 giây cuối
        float alpha = std::min(1.0f, (float)(capture_notify_until - GetTime()) / 0.5f);
        DrawRectangleRounded(box, 0.3f, 16, Fade(active_theme().bg_fallback, 0.9f * alpha));
        DrawRectangleRoundedLinesEx(box, 0.3f, 16, 2.0f, Fade(active_theme().accent_color, alpha));
        DrawTextEx(heading_font, msg, { box.x + 20.0f, box.y + 12.0f }, 28, 1.0f, Fade(active_theme().accent_color, alpha));
    }
}

void UI::draw_starting() {
    draw_background();
    Vector2 mouse_pos = GetMousePosition();

    Vector2 title_size = MeasureTextEx(title_font, TITLE, 150, 1.0f);
    DrawTextEx(title_font, TITLE,
        {(GetScreenWidth() - title_size.x) / 2.0f, 280},
        150, 1.0f, RAYWHITE);

    const float edge_padding = 20.0f;
    Vector2 hint_size = MeasureTextEx(body_font, "Press H to exit", 20, 1.0f);
    DrawTextEx(body_font, "Press H to exit",
        {GetScreenWidth() - hint_size.x - edge_padding,
         GetScreenHeight() - hint_size.y - edge_padding},
        20, 1.0f, RAYWHITE);

    draw_button(btn_new_game, mouse_pos, false, 36);
    draw_button(btn_load_game, mouse_pos, false, 36);
    draw_button(btn_config, mouse_pos, false, 36);
}

void UI::draw_mode() {
    draw_background();
    Vector2 mouse_pos = GetMousePosition();
    DrawTextEx(heading_font, "CHOOSE GAME MODE", {390, 120}, 36, 1.0f, RAYWHITE);
    draw_button(btn_pvp, mouse_pos, CheckCollisionPointRec(mouse_pos, btn_pvp.bounds), 28);
    draw_button(btn_pve, mouse_pos, CheckCollisionPointRec(mouse_pos, btn_pve.bounds), 28);
}

void UI::draw_difficulty() {
    draw_background();
    Vector2 mouse_pos = GetMousePosition();
    DrawTextEx(heading_font, "CHOOSE DIFFICULTY", {390, 120}, 36, 1.0f, RAYWHITE);
    draw_button(btn_easy,   mouse_pos, CheckCollisionPointRec(mouse_pos, btn_easy.bounds), 28);
    draw_button(btn_medium, mouse_pos, CheckCollisionPointRec(mouse_pos, btn_medium.bounds), 28);
    draw_button(btn_hard,   mouse_pos, CheckCollisionPointRec(mouse_pos, btn_hard.bounds), 28);
}

void UI::draw_choosing_board_size(const Board &board) {
    draw_background();
    Vector2 mouse_pos = GetMousePosition();
    DrawTextEx(heading_font, "CHOOSE BOARD SIZE", {390, 120}, 36, 1.0f, RAYWHITE);
    draw_button(btn_size9,  mouse_pos, CheckCollisionPointRec(mouse_pos, btn_size9.bounds), 28);
    draw_button(btn_size13, mouse_pos, CheckCollisionPointRec(mouse_pos, btn_size13.bounds), 28);
    draw_button(btn_size19, mouse_pos, CheckCollisionPointRec(mouse_pos, btn_size19.bounds), 28);
}

void UI::draw_ending(const Board &board) {
    Vector2 mouse_pos = GetMousePosition();

    //Panel
    DrawRectangle(
        0, 0, GetScreenWidth(), GetScreenHeight(),
        Fade(BLACK, 0.65f)
    );
    float panel_start_x = (GetScreenWidth() - 560.0f) / 2.0f;
    float panel_start_y = (GetScreenHeight() - 420.0f) / 2.0f;
    Rectangle panel = {
        panel_start_x,
        panel_start_y,
        560.0f,
        420.0f
    };
    DrawRectangleRounded(panel, 0.08f, 16, active_theme().bg_fallback);
    DrawRectangleRoundedLinesEx(panel, 0.08f, 16, 2.0f, active_theme().accent_color);

    //Trophy
    if (trophy_tex.id != 0) {
        const float scale = 120.0f / std::max(trophy_tex.width, trophy_tex.height);
        const float width = trophy_tex.width * scale;
        const float height = trophy_tex.height * scale;
        DrawTexturePro(trophy_tex,
                       {0, 0, (float)trophy_tex.width, (float)trophy_tex.height},
                       {panel.x + (panel.width - width) / 2.0f,
                        panel.y + 24.0f, width, height},
                       {0, 0}, 0.0f, WHITE);
    }

    //Final score
    std::pair<double, double> final_score = scoring(board);
    auto draw_centered_text = [&](const char *text, float y, float font_size) {
        Vector2 text_size = MeasureTextEx(heading_font, text, font_size, 1.0f);
        DrawTextEx(heading_font, text,
                   {panel.x + (panel.width - text_size.x) / 2.0f, y},
                   font_size, 1.0f, RAYWHITE);
    };
    if (final_score.first == final_score.second) {
        draw_centered_text("Tie", panel.y + 164.0f, 36);
    }
    else if(final_score.first < final_score.second) {
        draw_centered_text("White Wins", panel.y + 164.0f, 36);
        draw_centered_text(TextFormat("by %.1f komi", final_score.second - final_score.first),
                           panel.y + 220.0f, 28);
    }
    else {
        draw_centered_text("Black Wins", panel.y + 164.0f, 36);
        draw_centered_text(TextFormat("by %.1f komi", final_score.first - final_score.second),
                           panel.y + 220.0f, 28);
    }

    //Back
    btn_back_from_end.bounds = {
        panel.x + (panel.width - 360.0f) / 2.0f,
        panel.y + panel.height - 32.0f - 64.0f,
        360.0f, 64.0f
    };
    draw_button(btn_back_from_end, mouse_pos, false, 24);
}

void UI::draw_select_preview(Rectangle preview_box) const {
    DrawRectangleRounded(preview_box, 0.08f, 16, (Color){ 16, 20, 30, 255 });
    DrawRectangleRoundedLinesEx(preview_box, 0.08f, 16, 1.5f, (Color){ 90, 105, 130, 255 });

    std::string header_text = "Preview";
    if (active_select_tab == 0 && selected_theme_idx < (int)theme_options.size()) {
        header_text = "Theme: " + theme_options[selected_theme_idx].name;
    } else if (active_select_tab == 1 && selected_board_idx < (int)board_options.size()) {
        header_text = "Board: " + board_options[selected_board_idx].name;
    } else if (active_select_tab == 2 && selected_stone_idx < (int)stone_options.size()) {
        header_text = "Stones: " + stone_options[selected_stone_idx].name;
    }

    DrawTextEx(label_font, header_text.c_str(), { preview_box.x + 16.0f, preview_box.y + 12.0f }, 20, 1.0f, RAYWHITE);

    Rectangle inner = {
        preview_box.x + 16.0f,
        preview_box.y + 46.0f,
        preview_box.width - 32.0f,
        preview_box.height - 62.0f
    };

    const ThemeOption &cur = active_theme();

    if (active_select_tab == 0 && selected_theme_idx < (int)theme_options.size()) {
        const ThemeOption &t = theme_options[selected_theme_idx];
        if (t.bg_tex.id != 0) {
            Rectangle src = { 0, 0, (float)t.bg_tex.width, (float)t.bg_tex.height };
            DrawTexturePro(t.bg_tex, src, inner, { 0, 0 }, 0.0f, WHITE);
        } else {
            DrawRectangleRounded(inner, 0.06f, 12, t.bg_fallback);
        }
        DrawRectangleRoundedLinesEx(inner, 0.06f, 12, 1.2f, (Color){ 110, 125, 150, 180 });

        // Mini board on left of theme preview
        Rectangle mini_board = { inner.x + 18.0f, inner.y + (inner.height - 150.0f) * 0.5f, 150.0f, 150.0f };
        Rectangle b_src = { 0, 0, (float)wood_board_tex.width, (float)wood_board_tex.height };
        DrawTexturePro(wood_board_tex, b_src, mini_board, { 0, 0 }, 0.0f, WHITE);
        for (int i = 0; i < 4; i++) {
            float p = 24.0f + i * 34.0f;
            DrawLineEx({ mini_board.x + 24.0f, mini_board.y + p }, { mini_board.x + 126.0f, mini_board.y + p }, 1.5f, t.line_color);
            DrawLineEx({ mini_board.x + p, mini_board.y + 24.0f }, { mini_board.x + p, mini_board.y + 126.0f }, 1.5f, t.line_color);
        }
        Rectangle s_src = { 0, 0, (float)black_stone_tex.width, (float)black_stone_tex.height };
        DrawTexturePro(black_stone_tex, s_src, { mini_board.x + 44.0f, mini_board.y + 44.0f, 28.0f, 28.0f }, { 0, 0 }, 0.0f, WHITE);
        DrawTexturePro(white_stone_tex, s_src, { mini_board.x + 78.0f, mini_board.y + 44.0f, 28.0f, 28.0f }, { 0, 0 }, 0.0f, WHITE);

        // Mini Normal / Hover / Active buttons on right of theme preview
        float bx = inner.x + 190.0f;
        float bw = inner.width - 208.0f;
        OptionButton demo_norm = { { bx, inner.y + 34.0f,  bw, 42.0f }, "Normal" };
        OptionButton demo_hov  = { { bx, inner.y + 92.0f,  bw, 42.0f }, "Hover" };
        OptionButton demo_act  = { { bx, inner.y + 150.0f, bw, 42.0f }, "Active" };
        draw_button(demo_norm, { -1, -1 }, false, 18, false);
        draw_button(demo_hov,  { demo_hov.bounds.x + 5, demo_hov.bounds.y + 5 }, false, 18, true);
        draw_button(demo_act,  { -1, -1 }, true, 18, false);
    } else if (active_select_tab == 1 && selected_board_idx < (int)board_options.size()) {
        DrawRectangleRounded(inner, 0.06f, 12, cur.bg_fallback);
        float sz = std::min(inner.width, inner.height) - 24.0f;
        Rectangle mini_board = {
            inner.x + (inner.width - sz) * 0.5f,
            inner.y + (inner.height - sz) * 0.5f,
            sz, sz
        };
        const Texture2D &btex = board_options[selected_board_idx].tex;
        Rectangle src = { 0, 0, (float)btex.width, (float)btex.height };
        DrawTexturePro(btex, src, mini_board, { 0, 0 }, 0.0f, WHITE);
        DrawRectangleLinesEx(mini_board, 2.0f, cur.board_border);

        float pad = 24.0f;
        float step = (sz - pad * 2.0f) / 4.0f;
        for (int i = 0; i < 5; i++) {
            float p = pad + i * step;
            DrawLineEx({ mini_board.x + pad, mini_board.y + p }, { mini_board.x + sz - pad, mini_board.y + p }, 1.6f, cur.line_color);
            DrawLineEx({ mini_board.x + p, mini_board.y + pad }, { mini_board.x + p, mini_board.y + sz - pad }, 1.6f, cur.line_color);
        }
        DrawCircleV({ mini_board.x + pad + 2 * step, mini_board.y + pad + 2 * step }, 4.0f, cur.line_color);
        float r = step * 0.42f;
        Rectangle s_src = { 0, 0, (float)black_stone_tex.width, (float)black_stone_tex.height };
        DrawTexturePro(black_stone_tex, s_src, { mini_board.x + pad + step - r, mini_board.y + pad + step - r, r * 2, r * 2 }, { 0, 0 }, 0.0f, WHITE);
        DrawTexturePro(white_stone_tex, s_src, { mini_board.x + pad + 3 * step - r, mini_board.y + pad + step - r, r * 2, r * 2 }, { 0, 0 }, 0.0f, WHITE);
    } else if (active_select_tab == 2 && selected_stone_idx < (int)stone_options.size()) {
        Rectangle b_src = { 0, 0, (float)wood_board_tex.width, (float)wood_board_tex.height };
        DrawTexturePro(wood_board_tex, b_src, inner, { 0, 0 }, 0.0f, WHITE);
        DrawRectangleLinesEx(inner, 2.0f, cur.board_border);

        float cy = inner.y + inner.height * 0.46f;
        float lx = inner.x + inner.width * 0.30f;
        float rx = inner.x + inner.width * 0.70f;
        DrawLineEx({ inner.x, cy }, { inner.x + inner.width, cy }, 2.2f, cur.line_color);
        DrawLineEx({ lx, inner.y }, { lx, inner.y + inner.height }, 2.2f, cur.line_color);
        DrawLineEx({ rx, inner.y }, { rx, inner.y + inner.height }, 2.2f, cur.line_color);

        const StoneOption &so = stone_options[selected_stone_idx];
        float r = 46.0f;
        Rectangle bsrc = { 0, 0, (float)so.black_tex.width, (float)so.black_tex.height };
        Rectangle wsrc = { 0, 0, (float)so.white_tex.width, (float)so.white_tex.height };
        DrawCircleGradient({ lx + 4, cy + 5 }, r * 1.05f, { 0, 0, 0, 120 }, { 0, 0, 0, 0 });
        DrawTexturePro(so.black_tex, bsrc, { lx - r, cy - r, r * 2, r * 2 }, { 0, 0 }, 0.0f, WHITE);
        DrawCircleGradient({ rx + 4, cy + 5 }, r * 1.05f, { 0, 0, 0, 120 }, { 0, 0, 0, 0 });
        DrawTexturePro(so.white_tex, wsrc, { rx - r, cy - r, r * 2, r * 2 }, { 0, 0 }, 0.0f, WHITE);

        Vector2 b_sz = MeasureTextEx(label_font, "Black", 20, 1.0f);
        Vector2 w_sz = MeasureTextEx(label_font, "White", 20, 1.0f);
        DrawTextEx(label_font, "Black", { lx - b_sz.x * 0.5f, cy + r + 14.0f }, 20, 1.0f, cur.line_color);
        DrawTextEx(label_font, "White", { rx - w_sz.x * 0.5f, cy + r + 14.0f }, 20, 1.0f, cur.line_color);
    }
}

void UI::draw_config() {
    draw_background();
    Vector2 mouse_pos = GetMousePosition();

    // Centered SETTINGS heading
    Vector2 title_sz = MeasureTextEx(heading_font, "SETTINGS", 42, 1.0f);
    DrawTextEx(heading_font, "SETTINGS", { (GetScreenWidth() - title_sz.x) * 0.5f, 60.0f }, 42, 1.0f, RAYWHITE);

    // Centered Audio section (x = 190 .. 1090)
    auto draw_centered_val = [&](const char *txt, float y, Color col) {
        Vector2 sz = MeasureTextEx(label_font, txt, 22, 1.0f);
        DrawTextEx(label_font, txt, { 550.0f - sz.x * 0.5f, y }, 22, 1.0f, col);
    };

    DrawTextEx(label_font, "Master Volume", { 190, 154 }, 24, 1.0f, RAYWHITE);
    draw_button(btn_master_down, mouse_pos, false, 22);
    draw_centered_val(is_muted_master ? "Muted" : TextFormat("%d%%", master_volume), 156, is_muted_master ? RED : RAYWHITE);
    draw_button(btn_master_up, mouse_pos, false, 22);
    draw_button(btn_master_mute, mouse_pos, is_muted_master, 20);

    DrawTextEx(label_font, "Music Volume", { 190, 214 }, 24, 1.0f, RAYWHITE);
    draw_button(btn_music_down, mouse_pos, false, 22);
    draw_centered_val(is_muted_music ? "Muted" : TextFormat("%d%%", music_volume), 216, is_muted_music ? RED : RAYWHITE);
    draw_button(btn_music_up, mouse_pos, false, 22);
    draw_button(btn_music_mute, mouse_pos, is_muted_music, 20);

    DrawTextEx(label_font, "Sound Effects", { 190, 274 }, 24, 1.0f, RAYWHITE);
    draw_button(btn_sound_down, mouse_pos, false, 22);
    draw_centered_val(is_muted_sound ? "Muted" : TextFormat("%d%%", sound_volume), 276, is_muted_sound ? RED : RAYWHITE);
    draw_button(btn_sound_up, mouse_pos, false, 22);
    draw_button(btn_sound_mute, mouse_pos, is_muted_sound, 20);

    // Music Track selector with < and > arrows (supports unlimited tracks in assets/soundtrack/)
    DrawTextEx(label_font, "Music Track", { 190, 334 }, 24, 1.0f, RAYWHITE);
    draw_button(btn_music_prev, mouse_pos, false, 24);
    std::string track_name = Audio::get_music_name(selected_music);
    Vector2 track_sz = MeasureTextEx(label_font, track_name.c_str(), 22, 1.0f);
    DrawTextEx(label_font, track_name.c_str(), { 620.0f - track_sz.x * 0.5f, 336.0f }, 22, 1.0f, active_theme().accent_color);
    draw_button(btn_music_next, mouse_pos, false, 24);

    // Subtle divider
    DrawLineEx({ 190.0f, 384.0f }, { 1090.0f, 384.0f }, 1.0f, (Color){ 148, 163, 184, 70 });

    // Select tabs: Game Theme / Board / Stones
    DrawTextEx(label_font, "Select", { 190, 410 }, 24, 1.0f, RAYWHITE);
    draw_button(btn_tab_theme,  mouse_pos, active_select_tab == 0, 21);
    draw_button(btn_tab_board,  mouse_pos, active_select_tab == 1, 21);
    draw_button(btn_tab_stones, mouse_pos, active_select_tab == 2, 21);

    // Picker Panel container
    DrawRectangleRounded(PICKER_PANEL_RECT, 0.06f, 16, (Color){ 26, 32, 47, 240 });
    DrawRectangleRoundedLinesEx(PICKER_PANEL_RECT, 0.06f, 16, 1.5f, (Color){ 100, 115, 140, 160 });

    // Determine active list count and selected index
    int item_count = 0;
    int selected_idx = 0;
    if (active_select_tab == 0) {
        item_count = (int)theme_options.size();
        selected_idx = selected_theme_idx;
    } else if (active_select_tab == 1) {
        item_count = (int)board_options.size();
        selected_idx = selected_board_idx;
    } else {
        item_count = (int)stone_options.size();
        selected_idx = selected_stone_idx;
    }

    float total_h = item_count * (PICKER_ITEM_H + PICKER_ITEM_GAP) - PICKER_ITEM_GAP;
    float max_scroll = std::max(0.0f, total_h - PICKER_LIST_RECT.height);

    // Mouse wheel scrolling inside the left list area
    if (CheckCollisionPointRec(mouse_pos, PICKER_LIST_RECT)) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            select_scroll_offset -= wheel * 36.0f;
        }
    }
    select_scroll_offset = std::clamp(select_scroll_offset, 0.0f, max_scroll);

    // Draw scrollable items clipped inside PICKER_LIST_RECT
    BeginScissorMode((int)PICKER_LIST_RECT.x, (int)PICKER_LIST_RECT.y, (int)PICKER_LIST_RECT.width, (int)PICKER_LIST_RECT.height);
    float item_w = PICKER_LIST_RECT.width - 18.0f;
    for (int i = 0; i < item_count; i++) {
        float iy = PICKER_LIST_RECT.y + i * (PICKER_ITEM_H + PICKER_ITEM_GAP) - select_scroll_offset;
        if (iy + PICKER_ITEM_H < PICKER_LIST_RECT.y || iy > PICKER_LIST_RECT.y + PICKER_LIST_RECT.height) continue;

        const char *item_label = "";
        if (active_select_tab == 0) item_label = theme_options[i].name.c_str();
        else if (active_select_tab == 1) item_label = board_options[i].name.c_str();
        else item_label = stone_options[i].name.c_str();

        OptionButton item_btn = { { PICKER_LIST_RECT.x, iy, item_w, PICKER_ITEM_H }, item_label };
        bool hovered_in_list = CheckCollisionPointRec(mouse_pos, PICKER_LIST_RECT);
        draw_button(item_btn, hovered_in_list ? mouse_pos : Vector2{ -1, -1 }, i == selected_idx, 20, hovered_in_list);
    }
    EndScissorMode();

    // Scrollbar track & thumb
    float track_x = PICKER_LIST_RECT.x + PICKER_LIST_RECT.width - 8.0f;
    DrawRectangleRounded({ track_x, PICKER_LIST_RECT.y + 2.0f, 6.0f, PICKER_LIST_RECT.height - 4.0f }, 0.5f, 8, (Color){ 148, 163, 184, 55 });
    if (max_scroll > 0.0f) {
        float thumb_h = std::max(36.0f, (PICKER_LIST_RECT.height / total_h) * (PICKER_LIST_RECT.height - 4.0f));
        float thumb_y = PICKER_LIST_RECT.y + 2.0f + (select_scroll_offset / max_scroll) * ((PICKER_LIST_RECT.height - 4.0f) - thumb_h);
        DrawRectangleRounded({ track_x, thumb_y, 6.0f, thumb_h }, 0.5f, 8, active_theme().accent_color);
    } else {
        DrawRectangleRounded({ track_x, PICKER_LIST_RECT.y + 2.0f, 6.0f, PICKER_LIST_RECT.height - 4.0f }, 0.5f, 8, active_theme().accent_color);
    }

    // Live Preview panel on right
    draw_select_preview(PICKER_PREVIEW_RECT);

    // Ok button at bottom
    draw_button(btn_back_config, mouse_pos, false, 26);
}

void UI::draw_loading() {
    draw_background();
    Vector2 mouse_pos = GetMousePosition();

    DrawTextEx(heading_font, "Choose a slot to load", {390, 120}, 32, 1.0f, RAYWHITE);

    for (int i = 0; i < 3; i++) {
        draw_button(btn_load_slots[i], mouse_pos, false, 28);
    }
    draw_button(btn_back_load, mouse_pos, false, 24);
}

void UI::draw_saving() {
    draw_background();
    Vector2 mouse_pos = GetMousePosition();

    DrawTextEx(heading_font, "SAVE GAME", {390, 120}, 36, 1.0f, RAYWHITE);
    if (overwrite_slot != -1) {
        DrawTextEx(body_font, TextFormat("Overwrite the saved game in Slot %i?", overwrite_slot + 1), {390, 250}, 24, 1.0f, RAYWHITE);
        draw_button(btn_confirm_overwrite, mouse_pos, false, 24);
        draw_button(btn_cancel_overwrite, mouse_pos, false, 24);
        return;
    }
    DrawTextEx(body_font, "Choose a slot to save.", {390, 175}, 22, 1.0f, RAYWHITE);

    for (int i = 0; i < 3; i++) {
        draw_button(btn_save_slots[i], mouse_pos, false, 28);
    }
    draw_button(btn_back_save, mouse_pos, false, 24);
}

void UI::notify_stone_captured() {
    capture_notify_until = GetTime() + 2.0;
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
            if (CheckCollisionPointRec(mouse_pos, btn_music_prev.bounds)) {
                Audio::play(SFX::CLICK);
                int count = Audio::get_music_count();
                if (count > 0) {
                    selected_music = (selected_music <= 1) ? count : (selected_music - 1);
                    Audio::change_music(selected_music);
                }
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_music_next.bounds)) {
                Audio::play(SFX::CLICK);
                int count = Audio::get_music_count();
                if (count > 0) {
                    selected_music = (selected_music >= count) ? 1 : (selected_music + 1);
                    Audio::change_music(selected_music);
                }
                return true;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_tab_theme.bounds)) {
                Audio::play(SFX::CLICK);
                active_select_tab = 0;
                select_scroll_offset = 0.0f;
                return false;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_tab_board.bounds)) {
                Audio::play(SFX::CLICK);
                active_select_tab = 1;
                select_scroll_offset = 0.0f;
                return false;
            }
            if (CheckCollisionPointRec(mouse_pos, btn_tab_stones.bounds)) {
                Audio::play(SFX::CLICK);
                active_select_tab = 2;
                select_scroll_offset = 0.0f;
                return false;
            }
            if (CheckCollisionPointRec(mouse_pos, PICKER_LIST_RECT)) {
                int item_count = (active_select_tab == 0) ? (int)theme_options.size()
                               : (active_select_tab == 1) ? (int)board_options.size()
                                                          : (int)stone_options.size();
                float item_w = PICKER_LIST_RECT.width - 18.0f;
                for (int i = 0; i < item_count; i++) {
                    float iy = PICKER_LIST_RECT.y + i * (PICKER_ITEM_H + PICKER_ITEM_GAP) - select_scroll_offset;
                    Rectangle item_rect = { PICKER_LIST_RECT.x, iy, item_w, PICKER_ITEM_H };
                    if (CheckCollisionPointRec(mouse_pos, item_rect)) {
                        Audio::play(SFX::CLICK);
                        if (active_select_tab == 0) selected_theme = theme_options[i].name;
                        else if (active_select_tab == 1) selected_board = board_options[i].name;
                        else selected_stones = stone_options[i].name;
                        apply_selected_assets();
                        return true;
                    }
                }
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
            {"is_muted_master", this->is_muted_master},
            {"selected_theme", this->selected_theme},
            {"selected_board", this->selected_board},
            {"selected_stones", this->selected_stones}
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
                {"is_muted_master", false},
                {"selected_theme", "Classic"},
                {"selected_board", "Kaya Wood"},
                {"selected_stones", "3D Glossy"}
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
        this->selected_theme  = data.value("selected_theme", std::string("Classic"));
        this->selected_board  = data.value("selected_board", std::string("Kaya Wood"));
        this->selected_stones = data.value("selected_stones", std::string("3D Glossy"));
        apply_selected_assets();

        return true;
    } 
    catch(std::exception) {
        return false;
    }
}
