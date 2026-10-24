#pragma once

#include <utility>
#include <vector>
#include <array>

#include "helper.h"
#include "zobrist_hash.h"

struct Move {
    int pos_x, pos_y;
    StoneType stone_type; 
    unsigned long long hash_value; 
    std::vector<std::pair<int, int>> captured_stones;
    Move(int pos_x = -1, int pos_y = -1, StoneType stone_type = StoneType::EMPTY, unsigned long long hash_value = 0, std::vector<std::pair<int, int>> captured_stones = {});
};

class Board {
private:
    bool turn;
    int pass;
    ZobristHash hash_table;
    std::array<int, 2> captured;
    std::vector<Move> move_list;
    std::vector<Move> undo_list;
    StoneType board[BOARD_SIZE][BOARD_SIZE];

public:
    Board();

    /**
     * @brief Return the state of cell
     * 
     * @param pos_x 
     * @param pos_y 
     * @return char 
     */
    StoneType get_state(const int &pos_x, const int &pos_y) const;

    /**
     * @brief Return the captured pieces: 0 for black pieces that died and 1 for white
     * 
     * @param type
     * @return int
     */
    int get_captured(const int &type) const;

    /**
     * @brief Return the turn of the game
     * 
     * @return true/1 for white
     * @return false/0 for black
     */
    bool get_turn() const;

    /**
     * @brief Toggle the turn of the game
     */
    void toggle_turn();
    
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
     * @brief Make an virtual move
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

    bool check_existed_state() const;
};