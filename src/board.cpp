#include <iostream>
#include <filesystem>

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

void to_json(json &j, const Move &move) {
    j = {
        {"pos_x", move.pos_x},
        {"pos_y", move.pos_y},
        {"stone_type", move.stone_type},
        {"current_pass", move.current_pass},
        {"hash_value", move.hash_value},
        {"captured_stones", move.captured_stones}
    };
}

void to_json(json &j, const ZobristHash &hash_table) {
    j = {
        {"key", hash_table.key},
        {"current_hash", hash_table.current_hash}
    };
}

void from_json(const json &j, Move &move) {
    j.at("pos_x").get_to(move.pos_x);
    j.at("pos_y").get_to(move.pos_y);
    j.at("stone_type").get_to(move.stone_type);
    j.at("current_pass").get_to(move.current_pass);
    j.at("hash_value").get_to(move.hash_value);
    j.at("captured_stones").get_to(move.captured_stones);
}

void from_json(const json &j, ZobristHash &hash_table) {
    j.at("key").get_to(hash_table.key);
    j.at("current_hash").get_to(hash_table.current_hash);
}

Board::Board(int size) {
    reset(size);
}

void Board::reset(int new_size) {
    current_size = new_size; 
    turn = 1;
    pass = 0;
    captured = {0, 0};
    move_list.clear();
    undo_list.clear();
    hash_table = ZobristHash();

    grid.assign(current_size, std::vector<StoneType>(current_size));
    for (int i = 0; i < current_size; i++) {
        for (int j = 0; j < current_size; j++) {
            grid[i][j] = StoneType::EMPTY;
        }
    }
}

int Board::get_size() const {
    return current_size;
}

GameMode Board::get_mode() const {
    return mode;
}

Difficulty Board::get_difficulty() const {
    return difficulty;
}

bool Board::is_human_turn() const {
    return mode == GameMode::PVP || turn == 1;
}

void Board::set_mode(GameMode new_mode) {
    mode = new_mode;
}

void Board::set_difficulty(Difficulty new_difficulty) {
    difficulty = new_difficulty;
}

bool Board::in_board(const int &pos_x, const int &pos_y) const {
    return pos_x >= 0 && pos_x < current_size && pos_y >= 0 && pos_y < current_size;
}

bool Board::place_stone(int row, int col) {
    if (in_board(row, col)) {
        if (grid[row][col] == StoneType::EMPTY) {
            StoneType stone = (get_turn() == 1) ? StoneType::BLACKSTONE : StoneType::WHITESTONE;
            if (add_move(*this, Move(row, col, stone))) return false;
            this->clear_undo_list();
            toggle_turn();
            set_pass(0); 
            return true;
        }
    }
    return false;
}

int Board::get_cell(int row, int col) const {
    if (in_board(row, col)) {
        if (grid[row][col] == StoneType::BLACKSTONE) return 1;
        if (grid[row][col] == StoneType::WHITESTONE) return 2;
    }
    return 0;
}

StoneType Board::get_state(const int &pos_x, const int &pos_y) const {
    if (in_board(pos_x, pos_y)) {
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

int Board::get_pass() const {
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
    new_move.hash_value = hash_table.current_hash;
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

    update_cell(next_move.pos_x, next_move.pos_y, StoneType::EMPTY, next_move.stone_type);
    StoneType opponent = opposite_stone(next_move.stone_type);
    for (auto [pos_x, pos_y] : next_move.captured_stones) {
        update_cell(pos_x, pos_y, opponent, StoneType::EMPTY);
    }

    if (next_move.stone_type == StoneType::BLACKSTONE)
        captured[1] += (int)next_move.captured_stones.size();
    else if (next_move.stone_type == StoneType::WHITESTONE)
        captured[0] += (int)next_move.captured_stones.size();

    set_pass(0);
    toggle_turn();
    move_list.push_back(next_move);
}

void Board::clear_undo_list() {
    undo_list.clear();
}

bool Board::check_existed_state() const {
    if ((int)move_list.size() < 3) return false;
    return hash_table.current_hash == move_list[(int)move_list.size() - 3].hash_value;
}

bool Board::save_game(const std::string &path) {
    try {
        json data = {
            {"current_size", this->current_size},
            {"mode", this->mode},
            {"difficulty", this->difficulty},
            {"turn", this->turn},
            {"pass", this->pass},
            {"hash_table", this->hash_table},
            {"captured", this->captured},
            {"move_list", this->move_list},
            {"undo_list", this->undo_list},
            {"grid", this->grid},
        };

        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
        std::ofstream stream(path);
        stream << data.dump(4);
        stream.close();

        return true;
    } 
    catch(std::exception) {
        return false;
    }
}

bool Board::load_game(const std::string &path) {
    try {
        json data;
        std::ifstream stream(path);
        if (!stream.is_open()) {
            return false;
        }
        stream >> data;
        stream.close();

        data.at("current_size").get_to(this->current_size);
        data.at("turn").get_to(this->turn);
        data.at("pass").get_to(this->pass);
        data.at("hash_table").get_to(this->hash_table);
        data.at("captured").get_to(this->captured);
        data.at("move_list").get_to(this->move_list);
        data.at("undo_list").get_to(this->undo_list);
        data.at("grid").get_to(this->grid);
        data.at("difficulty").get_to(this->difficulty);
        data.at("mode").get_to(this->mode);

        return true;
    }
    catch(std::exception) {
        return false;
    }
}
