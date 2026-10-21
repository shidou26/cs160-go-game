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
    ZobristHash hash_value;
    std::array<int, 2> captured;
    std::vector<Move> move_list;
    std::vector<Move> undo_list;
    StoneType board[BOARD_SIZE][BOARD_SIZE];

    Board() {
        turn = 0;
        pass = 0;
    }

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
     * @brief Return the turn of the game
     * 
     * @return true/1 for white
     * @return false/0 for black
     */
    bool get_turn() const;

    /**
     * @brief Toggle the turn of the game
     */
    void toggle_turn() const;

    void update_cell(const int &pos_x, const int &pos_y, const char &prev_stone_type, const char &new_stone_type);

    void play_move(Move new_move);

    void undo_move(bool by_player);

    void redo_move();

    /**
     * @brief Clean undo list
     */
    void clear_undo_list();

    bool check_existed_state();
};