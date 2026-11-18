#include <raylib.h>

#include "../include/audio.h"

Sound sounds[5];
Music music;

namespace Audio {
    void init() {
        InitAudioDevice();

        music = LoadMusicStream("assets/soundtrack/song_1.mp3");
        PlayMusicStream(music);

        sounds[0] = LoadSound("assets/sfx/go_stone_1.wav");
        sounds[1] = LoadSound("assets/sfx/click.wav");
        sounds[2] = LoadSound("assets/sfx/positive.wav");
        sounds[3] = LoadSound("assets/sfx/negative.wav");
        sounds[4] = LoadSound("assets/sfx/end.wav");
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