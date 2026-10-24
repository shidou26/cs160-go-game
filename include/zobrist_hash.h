#pragma once

#include "helper.h"

class ZobristHash {
private:
    unsigned long long key[BOARD_SIZE][BOARD_SIZE][2]; 
    unsigned long long current_hash;

public:
    ZobristHash();

    /** 
     * @brief Get hash
     * @return unsigned long long
     */
    unsigned long long get_hash() const;

    /**
     * @brief Update hash value after a move
     */
    void update_hash(const int &pos_x, const int &pos_y, const StoneType &stone_type);
};