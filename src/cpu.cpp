#include "../include/cpu.h"

Move easy_play(Board &board) {
    
}

Move medium_play(Board &board) {

}

Move hard_play(Board &board) {

}

Move cpu_play(Board &board) {
    Move seol;
    switch (board.get_difficulty()) {
        case Difficulty::EASY:
            seol = easy_play(board);
            break;
        case Difficulty::MEDIUM:
            seol = medium_play(board);
            break;
        case Difficulty::HARD:
            seol = hard_play(board);
            break;
    }
    return seol;
}
