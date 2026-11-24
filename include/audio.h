#pragma once

#include <string>

namespace Audio {
    void init();
    void play(int id);
    void change_music(int id);
    int get_music_count();
    std::string get_music_name(int id);
    void change_click_sfx(int id);
    int get_click_sfx_count();
    std::string get_click_sfx_name(int id);
    void update();
    void set_volume(int master_volume, int sound_volume, int music_volume);
    void shutdown();
}
