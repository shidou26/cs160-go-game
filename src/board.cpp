#include "../include/helper.h"
#include "../include/board.h"

Move::Move(int pos_x, int pos_y, StoneType stone_type, unsigned long long hash_value, std::vector<std::pair<int, int>> captured_stones) {
    this->pos_x = pos_x;
    this->pos_y = pos_y;
    this->stone_type = stone_type;
    this->hash_value = hash_value;
    this->captured_stones = captured_stones;
}