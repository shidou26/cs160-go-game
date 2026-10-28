#include "../include/helper.h"
#include "../include/board.h"

Move::Move(int pos_x, int pos_y, StoneType stone_type, unsigned long long hash_value, std::vector<std::pair<int, int>> captured_stones) {
    this->pos_x = pos_x;
    this->pos_y = pos_y;
    this->stone_type = stone_type;
    this->hash_value = hash_value;
    this->captured_stones = captured_stones;
}

Board::Board(int size) {
    current_size = (size > 0 && size <= BOARD_SIZE) ? size : 9;
    reset(current_size);
}

void Board::reset(int new_size) {
    if (new_size > 0 && new_size <= BOARD_SIZE) {
        current_size = new_size;
    }
    turn = 0;
    pass = 0;
    captured = {0, 0};
    move_list.clear();
    undo_list.clear();
    hash_table = ZobristHash();

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            grid[i][j] = StoneType::EMPTY;
        }
    }
}

int Board::get_size() const {
    return current_size;
}

bool Board::place_stone(int row, int col, int player) {
    if (row >= 0 && row < current_size && col >= 0 && col < current_size) {
        if (grid[row][col] == StoneType::EMPTY) {
            StoneType stone = (player == 1) ? StoneType::BLACK : StoneType::WHITE;
            play_move(Move(row, col, stone));
            clear_undo_list();
            toggle_turn();
            return true;
        }
    }
    return false;
}

int Board::get_cell(int row, int col) const {
    if (row >= 0 && row < current_size && col >= 0 && col < current_size) {
        if (grid[row][col] == StoneType::BLACK) return 1;
        if (grid[row][col] == StoneType::WHITE) return 2;
    }
    return 0;
}

StoneType Board::get_state(const int &pos_x, const int &pos_y) const {
    if (pos_x >= 0 && pos_x < BOARD_SIZE && pos_y >= 0 && pos_y < BOARD_SIZE) {
        return grid[pos_x][pos_y];
    }
    return StoneType::EMPTY;
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
    grid[pos_x][pos_y] = new_stone_type;
    hash_table.update_hash(pos_x, pos_y, new_stone_type);
}

void Board::play_move(Move new_move) {
    update_cell(new_move.pos_x, new_move.pos_y, StoneType::EMPTY, new_move.stone_type);
    
    if (new_move.stone_type == StoneType::BLACK) 
        captured[1] += (int)new_move.captured_stones.size();
    else if (new_move.stone_type == StoneType::WHITE)
        captured[0] += (int)new_move.captured_stones.size();

    StoneType opponent = opposite_stone(new_move.stone_type);
    for (auto [pos_x, pos_y] : new_move.captured_stones) {
        update_cell(pos_x, pos_y, opponent, StoneType::EMPTY);
    }

    new_move.hash_value = hash_table.get_hash();
    move_list.push_back(new_move);
}

void Board::undo_move(bool by_player) {
    if (move_list.empty()) return;

    Move last_move = move_list.back();
    move_list.pop_back();

    update_cell(last_move.pos_x, last_move.pos_y, last_move.stone_type, StoneType::EMPTY);

    StoneType opponent = opposite_stone(last_move.stone_type);
    for (auto [pos_x, pos_y] : last_move.captured_stones) {
        update_cell(pos_x, pos_y, StoneType::EMPTY, opponent);
    }

    if (last_move.stone_type == StoneType::BLACK)
        captured[1] -= (int)last_move.captured_stones.size();
    else if (last_move.stone_type == StoneType::WHITE)
        captured[0] -= (int)last_move.captured_stones.size();

    if (by_player) {
        undo_list.push_back(last_move);
        toggle_turn();
    }
}

void Board::redo_move() {
    if (undo_list.empty()) return;

    Move next_move = undo_list.back();
    undo_list.pop_back();

    play_move(next_move);
    toggle_turn();
}

void Board::clear_undo_list() {
    undo_list.clear();
}

bool Board::check_existed_state() const {
    if (move_list.size() < 2) return false;
    unsigned long long current_hash = move_list.back().hash_value;
    for (int i = 0; i + 1 < move_list.size(); i++) {
        if (move_list[i].hash_value == current_hash) {
            return true;
        }
    }
    return false;
}
