#pragma once

#include <utility>
#include <vector>
#include <array>

#include "../include/helper.h"
#include "../include/zobrist_hash.h"

struct Move {
    int pos_x, pos_y;
    StoneType stone_type; 
    unsigned long long hash_value; 
    std::vector<std::pair<int, int>> captured_stones;
    Move(int pos_x = -1, int pos_y = -1, StoneType stone_type = StoneType::EMPTY, unsigned long long hash_value = 0, std::vector<std::pair<int, int>> captured_stones = {});
};

class Board {
private:
    int current_size;
    int turn;
    int pass;
    ZobristHash hash_table;
    std::array<int, 2> captured;
    std::vector<Move> move_list;
    std::vector<Move> undo_list;
    StoneType grid[BOARD_SIZE][BOARD_SIZE];

public:
    Board(int size = BOARD_SIZE);

    /**
     * @brief Reset bàn cờ về trạng thái ban đầu (có thể truyền kích thước mới: 9, 13, 19)
     */
    void reset(int new_size = -1);

    /**
     * @brief Lấy kích thước bàn cờ hiện tại (9, 13, hoặc 19)
     */
    int get_size() const;

    /**
     * @brief Đặt quân cờ lên vị trí (row, col) cho người chơi (1: Đen, 2: Trắng)
     */
    bool place_stone(int row, int col);

    /**
     * @brief Lấy giá trị ô cờ dạng số nguyên cho UI (0: Trống, 1: Đen, 2: Trắng)
     */
    int get_cell(int row, int col) const;

    /**
     * @brief Return the state of cell
     * 
     * @param pos_x 
     * @param pos_y 
     * @return StoneType 
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
     * @return 2 for white
     * @return 1 for black
     */
    int get_turn() const;

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
     * @brief 
     * 
     * @return true 
     * @return false 
     */
    bool check_existed_state() const;
};
