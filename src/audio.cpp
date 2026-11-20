#include <raylib.h>
#include <string> 

#include "../include/audio.h"

Sound sounds[5];
Music music;

namespace Audio {
    void init() {
        InitAudioDevice();

        sounds[0] = LoadSound("assets/sfx/go_stone_1.wav");
        sounds[1] = LoadSound("assets/sfx/click.wav");
        sounds[2] = LoadSound("assets/sfx/positive.wav");
        sounds[3] = LoadSound("assets/sfx/negative.wav");
        sounds[4] = LoadSound("assets/sfx/end.wav");
    }

    void change_music(int id) {
        UnloadMusicStream(music);
        music = LoadMusicStream(("assets/soundtrack/song_" + std::to_string(id) + ".mp3").c_str());
        PlayMusicStream(music);
    }

    void play(int id) {
        PlaySound(sounds[id]);
    }

    void update() {
        UpdateMusicStream(music);
    }

    void set_volume(int master_volume, int sound_volume, int music_volume) {
        SetMasterVolume(master_volume / 100.0);
        for (Sound sound : sounds) SetSoundVolume(sound, sound_volume / 100.0);
        SetMusicVolume(music, music_volume / 100.0);
    }

    void shutdown() {
        for (Sound sound : sounds) UnloadSound(sound);
        UnloadMusicStream(music);
        CloseAudioDevice();
    }
}