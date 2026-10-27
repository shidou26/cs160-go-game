#pragma once

#include <array>
#include <vector>
#include <utility>
#include "board.h"

extern Board current_board;

/**
 * @brief Get the captured stones 
 * 
 * @param start_x 
 * @param start_y 
 * @return std::array<std::vector<std::pair<int, int>>, 2> 
 */
std::array<std::vector<std::pair<int, int>>, 2> get_captured_stones(const int &start_x, const int &start_y);

/**
 * @brief Make a move, remember to clear the undo list
 * 
 * @param new_move 
 * @return int 
 */
int add_move(Move new_move);

/**
 * @brief Check if there exists a way to violate the KO rule
 * 
 * @return true 
 * @return false 
 */
bool check_ko_rule();

/**
 * @brief Check if this move will make a KO threat
 * 
 * @param new_move 
 * @return true 
 * @return false 
 */
bool check_ko_threat(Move new_move);
