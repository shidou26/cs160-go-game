#include <raylib.h>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "../include/audio.h"

namespace {
    Sound sounds[6]{};
    Music music{};
    bool has_music = false;
    float last_sound_vol = 0.8f;

    std::vector<std::string> music_paths;
    std::vector<std::string> music_names;

    std::vector<std::string> click_sfx_paths;
    std::vector<std::string> click_sfx_names;

    bool is_audio_ext(const std::string &ext) {
        std::string lower = ext;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return (lower == ".mp3" || lower == ".wav" || lower == ".ogg");
    }

    void scan_soundtracks() {
        music_paths.clear();
        music_names.clear();

        const std::string dir = "assets/soundtrack";
        if (!std::filesystem::exists(dir)) return;

        std::vector<std::filesystem::path> files;
        for (const auto &entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file() && is_audio_ext(entry.path().extension().string())) {
                files.push_back(entry.path());
            }
        }
        std::sort(files.begin(), files.end());

        for (const auto &p : files) {
            music_paths.push_back(p.string());
            music_names.push_back(p.stem().string());
        }
    }

    void scan_click_sfx() {
        click_sfx_paths.clear();
        click_sfx_names.clear();

        // Index 0 (id = 1): built-in default click sound
        click_sfx_paths.push_back("assets/sfx/click.wav");
        click_sfx_names.push_back("Default");

        const std::string dir = "assets/sfx/click";
        if (!std::filesystem::exists(dir)) return;

        std::vector<std::filesystem::path> files;
        for (const auto &entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file() && is_audio_ext(entry.path().extension().string())) {
                files.push_back(entry.path());
            }
        }
        std::sort(files.begin(), files.end());

        for (const auto &p : files) {
            click_sfx_paths.push_back(p.string());
            click_sfx_names.push_back(p.stem().string());
        }
    }
}

namespace Audio {
    void init() {
        InitAudioDevice();

        sounds[0] = LoadSound("assets/sfx/go_stone_1.wav");
        sounds[1] = LoadSound("assets/sfx/click.wav");
        sounds[2] = LoadSound("assets/sfx/positive.wav");
        sounds[3] = LoadSound("assets/sfx/negative.wav");
        sounds[4] = LoadSound("assets/sfx/end.wav");
        sounds[5] = LoadSound("assets/sfx/capture.mp3");

        scan_soundtracks();
        scan_click_sfx();
    }

    int get_music_count() {
        return (int)music_paths.size();
    }

    std::string get_music_name(int id) {
        if (music_names.empty()) return "None";
        int idx = ((id - 1) % (int)music_names.size() + (int)music_names.size()) % (int)music_names.size();
        return music_names[idx];
    }

    void change_music(int id) {
        if (music_paths.empty()) return;
        int idx = ((id - 1) % (int)music_paths.size() + (int)music_paths.size()) % (int)music_paths.size();

        if (has_music) {
            UnloadMusicStream(music);
            has_music = false;
        }
        music = LoadMusicStream(music_paths[idx].c_str());
        has_music = true;
        PlayMusicStream(music);
    }

    int get_click_sfx_count() {
        return (int)click_sfx_paths.size();
    }

    std::string get_click_sfx_name(int id) {
        if (click_sfx_names.empty()) return "Default";
        int idx = ((id - 1) % (int)click_sfx_names.size() + (int)click_sfx_names.size()) % (int)click_sfx_names.size();
        return click_sfx_names[idx];
    }

    void change_click_sfx(int id) {
        if (click_sfx_paths.empty()) return;
        int idx = ((id - 1) % (int)click_sfx_paths.size() + (int)click_sfx_paths.size()) % (int)click_sfx_paths.size();

        UnloadSound(sounds[1]);
        sounds[1] = LoadSound(click_sfx_paths[idx].c_str());
        SetSoundVolume(sounds[1], last_sound_vol);
    }

    void play(int id) {
        if (id >= 0 && id < 6) {
            PlaySound(sounds[id]);
        }
    }

    void update() {
        if (has_music) {
            UpdateMusicStream(music);
        }
    }

    void set_volume(int master_volume, int sound_volume, int music_volume) {
        last_sound_vol = sound_volume / 100.0f;
        SetMasterVolume(master_volume / 100.0f);
        for (Sound sound : sounds) SetSoundVolume(sound, last_sound_vol);
        if (has_music) {
            SetMusicVolume(music, music_volume / 100.0f);
        }
    }

    void shutdown() {
        for (Sound sound : sounds) UnloadSound(sound);
        if (has_music) {
            UnloadMusicStream(music);
            has_music = false;
        }
        CloseAudioDevice();
    }
}
