#include <raylib.h>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "../include/audio.h"

namespace {
    Sound sounds[5];
    Music music{};
    bool has_music = false;
    std::vector<std::string> music_paths;
    std::vector<std::string> music_names;

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
}

namespace Audio {
    void init() {
        InitAudioDevice();

        sounds[0] = LoadSound("assets/sfx/go_stone_1.wav");
        sounds[1] = LoadSound("assets/sfx/click.wav");
        sounds[2] = LoadSound("assets/sfx/positive.wav");
        sounds[3] = LoadSound("assets/sfx/negative.wav");
        sounds[4] = LoadSound("assets/sfx/end.wav");

        scan_soundtracks();
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

    void play(int id) {
        PlaySound(sounds[id]);
    }

    void update() {
        if (has_music) {
            UpdateMusicStream(music);
        }
    }

    void set_volume(int master_volume, int sound_volume, int music_volume) {
        SetMasterVolume(master_volume / 100.0f);
        for (Sound sound : sounds) SetSoundVolume(sound, sound_volume / 100.0f);
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
