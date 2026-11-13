#pragma once

#include "helper.h"

class ZobristHash {
public:
    unsigned long long key[BOARD_SIZE][BOARD_SIZE][2]; 
    unsigned long long current_hash;

    ZobristHash();

    /**
     * @brief Update hash value after a move
     */
    void update_hash(const int &pos_x, const int &pos_y, const StoneType &stone_type);
};
