#pragma once 

#include "../include/board.h"
#include "../include/helper.h"

/**
 * @brief Return random / simple AI move for Easy difficulty
 * 
 * @param board 
 * @return Move 
 */
Move easy_play(Board &board);

/**
 * @brief Return AI move using Minimax with shallow depth for Medium difficulty
 * 
 * @param board 
 * @return Move 
 */
Move medium_play(Board &board);

/**
 * @brief Return AI move using Minimax with Alpha-Beta pruning for Hard difficulty
 * 
 * @param board 
 * @return Move 
 */
Move hard_play(Board &board);

/**
 * @brief Return AI move depends on difficulty
 * 
 * @param board The board containing the selected difficulty.
 * @return Move 
 */
Move cpu_play(Board &board);
