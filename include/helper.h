#pragma once

#ifdef BLACK
#undef BLACK
#endif
#ifdef WHITE
#undef WHITE
#endif

#define BOARD_SIZE 19
#define KOMI 6.5
#define DEFAULT_GRID_WIDTH 480
#define START_X 160
#define START_Y 160
#define PADDING 50

enum StoneType {
    EMPTY = '.',
    BLACK = 'X',
    WHITE = 'O' 
};

constexpr int direction_x[4] = {1, 0, -1, 0},
              direction_y[4] = {0, 1, 0, -1};

inline bool in_board(const int &pos_x, const int &pos_y) {
    return 0 <= pos_x && pos_x < BOARD_SIZE && 
           0 <= pos_y && pos_y < BOARD_SIZE;
}

inline StoneType opposite_stone(const StoneType &stone) {
    return (stone == StoneType::WHITE ? StoneType::BLACK : StoneType::WHITE);
}
