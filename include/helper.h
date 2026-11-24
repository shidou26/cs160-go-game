#pragma once

#define BOARD_SIZE 19
#define KOMI 6.5
#define DEFAULT_GRID_WIDTH 720
#define START_X 160
#define START_Y 160
#define PADDING 50
#define TITLE "ANH'S BADUK"

enum StoneType {
    EMPTY = '.',
    BLACKSTONE = 'X',
    WHITESTONE = 'O' 
};

enum GamePhase {
    START,
    PLAYING,
    GAMEOVER,
    CONFIG,
    SAVING,
    LOADING,
    MODE,
    DIFFICULTY,
    CHOOSING_SIZE
};

enum GameMode {
    PVP,
    PVE 
};

enum Difficulty {
    EASY,
    MEDIUM,
    HARD
};

enum SFX {
    STONE,
    CLICK,
    POSITIVE,
    NEGATIVE,
    END,
    CAPTURE
};

constexpr int direction_x[4] = {1, 0, -1, 0},
              direction_y[4] = {0, 1, 0, -1};

inline StoneType opposite_stone(const StoneType &stone) {
    return (stone == StoneType::WHITESTONE ? StoneType::BLACKSTONE : StoneType::WHITESTONE);
}
