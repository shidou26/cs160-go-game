#pragma once 

#include "../include/board.h"
#include "../include/helper.h"

/**
 * @brief Return AI move depends on difficulty
 * 
 * @param board The board containing the selected difficulty.
 * @return Move 
 */
Move cpu_play(Board &board);
