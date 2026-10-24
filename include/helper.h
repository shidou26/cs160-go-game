#pragma once

#define BOARD_SIZE 19
#define KOMI 6.5

enum StoneType {
    EMPTY = '.',
    BLACK = 'X',
    WHITE = 'O' 
};

constexpr int direction_x[4] = {1, 0, -1, 0},
              direction_y[4] = {0, 1, 0, -1};

bool in_board(const int &pos_x, const int &pos_y) {
    return 0 <= pos_x && pos_x < BOARD_SIZE && 
           0 <= pos_y && pos_y < BOARD_SIZE;
}

StoneType opposite_stone(const StoneType &stone) {
    return (stone == StoneType::WHITE ? StoneType::BLACK : StoneType::WHITE);
}

