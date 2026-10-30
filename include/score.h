#pragma once

#include <utility>
#include "../include/board.h"

/**
 * @brief Return a std::pair of score of (black, white)
 * 
 * @param current_board 
 * @return std::pair<double, double> 
 */
std::pair<double, double> scoring(const Board &current_board);