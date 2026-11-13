#pragma once

#include <utility>
#include <vector>
#include <array>
#include <fstream>
#include <string> 

#include "../lib/json.hpp"
#include "../include/helper.h"
#include "../include/zobrist_hash.h"

using json = nlohmann::json;

struct Move {
    int pos_x, pos_y;
    StoneType stone_type; 
    int current_pass; 
    unsigned long long hash_value; 
    std::vector<std::pair<int, int>> captured_stones;
    Move(int pos_x = -1, int pos_y = -1, StoneType stone_type = StoneType::EMPTY, int current_pass = 0, unsigned long long hash_value = 0, std::vector<std::pair<int, int>> captured_stones = {});

};

void to_json(json &j, const Move &move);
void from_json(const json &j, Move &move);

class Board {
private:
    int current_size;
    int turn;
    int pass;
    ZobristHash hash_table;
    std::array<int, 2> captured;
    std::vector<Move> move_list;
    std::vector<Move> undo_list;
    std::vector<std::vector<StoneType>> grid;

public:
    Board(int size = BOARD_SIZE);

    int get_size() const;
    int get_pass() const;
    int get_cell(int row, int col) const; //(0: Empty, 1: Black, 2: White)
    int get_turn() const; //(1: Black, 2: White)
    int get_captured(const int &type) const; //(0: Dead black, 1: Dead white)
    StoneType get_state(const int &pos_x, const int &pos_y) const;

    /**
     * @brief Return if (pos_x, pos_y) is in board
     * 
     * @param pos_x 
     * @param pos_y 
     * @return true 
     * @return false 
     */
    bool in_board(const int &pos_x, const int &pos_y) const;

    /**
     * @brief Reset bàn cờ về trạng thái ban đầu (có thể truyền kích thước mới: 9, 13, 19)
     */
    void reset(int new_size = BOARD_SIZE);

    /**
     * @brief Đặt quân cờ lên vị trí (row, col) cho người chơi (1: Đen, 2: Trắng)
     */
    bool place_stone(int row, int col);

    /**
     * @brief Set the turn 
     * 
     * @param player 
     */
    void set_turn(int player);

    /**
     * @brief Toggle the turn of the game
     */
    void toggle_turn();
    
    /**
     * @brief Pass turn 
     * 
     */
    void pass_turn(); 

    /**
     * @brief Set the pass counter
     * 
     */
    void set_pass(int p);

    /**
     * @brief Update the cell with new_stone_type
     * 
     * @param pos_x 
     * @param pos_y 
     * @param prev_stone_type 
     * @param new_stone_type 
     */
    void update_cell(const int &pos_x, const int &pos_y, const StoneType &prev_stone_type, const StoneType &new_stone_type);

    /**
     * @brief Make a move on the board
     * 
     * @param new_move 
     */
    void play_move(Move new_move);

    /**
     * @brief Undo a move
     * 
     * @param by_player 
     */
    void undo_move(bool by_player);

    /**
     * @brief Redo a move
     * 
     */
    void redo_move();

    /**
     * @brief Clean undo list
     */
    void clear_undo_list();

    /**
     * @brief Check repition within 2 moves
     * 
     * @return true 
     * @return false 
     */
    bool check_existed_state() const;

    bool save_game(const std::string &path);
    bool load_game(const std::string &path);
};
