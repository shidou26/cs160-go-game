#include "../include/helper.h"
#include "../include/board.h"
#include "../include/rule.h"

Move::Move(int pos_x, int pos_y, StoneType stone_type, int current_pass, unsigned long long hash_value, std::vector<std::pair<int, int>> captured_stones) {
    this->pos_x = pos_x;
    this->pos_y = pos_y;
    this->stone_type = stone_type;
    this->current_pass = current_pass; 
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
    turn = 1;
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

bool Board::place_stone(int row, int col) {
    if (in_board(row, col)) {
        if (grid[row][col] == StoneType::EMPTY) {
            StoneType stone = (get_turn() == 1) ? StoneType::BLACKSTONE : StoneType::WHITESTONE;
            if (add_move(*this, Move(row, col, stone))) return false;
            toggle_turn();
            set_pass(0); 
            return true;
        }
    }
    return false;
}

int Board::get_cell(int row, int col) const {
    if (row >= 0 && row < current_size && col >= 0 && col < current_size) {
        if (grid[row][col] == StoneType::BLACKSTONE) return 1;
        if (grid[row][col] == StoneType::WHITESTONE) return 2;
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

int Board::get_turn() const {
    return turn;
}

void Board::set_turn(int player) {
    turn = player;
}

void Board::toggle_turn() {
    turn = 3 - turn;
}

void Board::pass_turn() {
    pass++;
}

void Board::set_pass(int p) {
    pass = p;
}

int Board::get_pass() {
    return pass;
}

void Board::update_cell(const int &pos_x, const int &pos_y, const StoneType &prev_stone_type, const StoneType &new_stone_type) {
    hash_table.update_hash(pos_x, pos_y, prev_stone_type);
    grid[pos_x][pos_y] = new_stone_type;
    hash_table.update_hash(pos_x, pos_y, new_stone_type);
}

void Board::play_move(Move new_move) {
    update_cell(new_move.pos_x, new_move.pos_y, StoneType::EMPTY, new_move.stone_type);
    
    if (new_move.stone_type == StoneType::BLACKSTONE) 
        captured[1] += (int)new_move.captured_stones.size();
    else if (new_move.stone_type == StoneType::WHITESTONE)
        captured[0] += (int)new_move.captured_stones.size();

    StoneType opponent = opposite_stone(new_move.stone_type);
    for (auto [pos_x, pos_y] : new_move.captured_stones) {
        update_cell(pos_x, pos_y, opponent, StoneType::EMPTY);
    }

    new_move.current_pass = get_pass();
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

    if (last_move.stone_type == StoneType::BLACKSTONE)
        captured[1] -= (int)last_move.captured_stones.size();
    else if (last_move.stone_type == StoneType::WHITESTONE)
        captured[0] -= (int)last_move.captured_stones.size();

    if (by_player) {
        undo_list.push_back(last_move);
        set_pass(last_move.current_pass);
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
    if ((int)move_list.size() < 3) return false;
    return hash_table.get_hash() == move_list[(int)move_list.size() - 3].hash_value;
}
