#include <queue>
#include <tuple>

#include "../include/score.h"
#include "../include/helper.h"

std::pair<double, double> scoring(const Board &current_board) {
    double black_point = current_board.get_captured(1);
    double white_point = KOMI + current_board.get_captured(0);

    bool visisted[BOARD_SIZE][BOARD_SIZE]{};
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            if(!visisted[i][j] && current_board.get_state(i, j) == StoneType::EMPTY) {
                std::queue<std::pair<int, int>> q;
                bool touch_black = false, touch_white = false;
                int point = 0;

                while(!q.empty()) {
                    int u, v; std::tie(u, v) = q.front(); q.pop();
                    point++;

                    for(int d = 0; d < 4; d++) {
                        int x = i + direction_x[d];
                        int y = j + direction_y[d];
                        
                        if(in_board(x, y)) {
                            if(current_board.get_state(x, y) == StoneType::BLACKSTONE) touch_black = true;
                            else if(current_board.get_state(x, y) == StoneType::WHITESTONE) touch_white = true;
                            else if(!visisted[x][y]) {
                                visisted[x][y] = true;
                                q.emplace(x, y);
                            }
                        }
                    }
                }

                if(!touch_black && touch_white) white_point += point;
                else if(!touch_white && touch_black) black_point += point;
            }
        }
    }

    return std::make_pair(black_point, white_point);
}