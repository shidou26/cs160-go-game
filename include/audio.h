#pragma once 

namespace Audio {
    void init();
    void play(int id);
    void update();
    void set_volume(int master_volume, int sound_volume, int music_volume);
    void shutdown();
}