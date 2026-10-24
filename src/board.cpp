#include "../include/helper.h"
#include "../include/board.h"

Move::Move(int pos_x, int pos_y, StoneType stone_type, unsigned long long hash_value, std::vector<std::pair<int, int>> captured_stones) {
    this->pos_x = pos_x;
    this->pos_y = pos_y;
    this->stone_type = stone_type;
    this->hash_value = hash_value;
    this->captured_stones = captured_stones;
}

Board::Board() {
    turn = 0;
    pass = 0;
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = StoneType::EMPTY;
        }
    }
}

StoneType Board::get_state(const int &pos_x, const int &pos_y) const {
    return board[pos_x][pos_y];
}

int Board::get_captured(const int &type) const {
    return captured[type];
}

bool Board::get_turn() const {
    return turn;
}

void Board::toggle_turn() {
    turn = 1 - turn;
}

void Board::update_cell(const int &pos_x, const int &pos_y, const StoneType &prev_stone_type, const StoneType &new_stone_type) {
    hash_table.update_hash(pos_x, pos_y, prev_stone_type);
    board[pos_x][pos_y] = new_stone_type;
    hash_table.update_hash(pos_x, pos_y, new_stone_type);
}

void Board::play_move(Move new_move) {
    update_cell(new_move.pos_x, new_move.pos_y, StoneType::EMPTY, new_move.stone_type);
    
    if(new_move.stone_type == StoneType::BLACK) 
        captured[1] += new_move.captured_stones.size();
    else if(new_move.stone_type == StoneType::WHITE)
        captured[0] += new_move.captured_stones.size();

    StoneType opponent = opposite_stone(new_move.stone_type);
    for(auto [pos_x, pos_y] : new_move.captured_stones) {
        update_cell(pos_x, pos_y, opponent, StoneType::EMPTY);
    }

    new_move.hash_value = hash_table.get_hash();
    move_list.push_back(new_move);
}

void Board::undo_move(bool by_player) {
    
}

void Board::redo_move() {

}

bool Board::check_existed_state() const {

}



