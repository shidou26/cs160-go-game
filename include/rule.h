#pragma once

#include <array>
#include <vector>
#include <utility>
#include "board.h"

/**
 * @brief Get the captured stones: 
 * captured[0] = all black stones with no liberties 
 * captured[1] = all white stones with no liberties
 * @param start_x 
 * @param start_y 
 * @return std::array<std::vector<std::pair<int, int>>, 2> 
 */
std::array<std::vector<std::pair<int, int>>, 2> get_captured_stones(const Board &current_board, const int &start_x, const int &start_y);

/**
 * @brief Make a move, remember to clear the undo list
 * 
 * @param new_move 
 * @return int 
 */
int add_move(Board &current_board, Move new_move);

/**
 * @brief Return the cell that under ko rule
 * 
 * @param current_board 
 * @return std::pair<int, int> 
 */
std::pair<int, int> check_ko_rule(Board &current_board);

/**
 * @brief Check if this move will make a KO threat
 * 
 * @param new_move 
 * @return true 
 * @return false 
 */
bool check_ko_threat(Board &current_board, Move new_move);
