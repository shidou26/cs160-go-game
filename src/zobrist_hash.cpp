#include <random>

#include "../include/zobrist_hash.h"

ZobristHash::ZobristHash() {
    current_hash = 0;
    std::mt19937_64 rng(1'000'005'277);
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            for(int t = 0; t < 2; t++) {
                key[i][j][t] = rng();
            }
        }
    }
}

unsigned long long ZobristHash::get_hash() const {
    return current_hash;
}

void ZobristHash::update_hash(const int &pos_x, const int &pos_y, const StoneType &stone_type) {
    if(stone_type == StoneType::BLACK) current_hash ^= key[pos_x][pos_y][0];
    else if(stone_type == StoneType::WHITE) current_hash ^= key[pos_x][pos_y][1];
}


