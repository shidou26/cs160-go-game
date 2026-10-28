#include <array>
#include <queue>
#include <utility>
#include <vector>
#include <cstring>
#include <tuple>

#include "../include/board.h"
#include "../include/rule.h"
#include "../include/helper.h"

std::array<std::vector<std::pair<int, int>>, 2> get_captured_stones(const Board &current_board, const int &start_x, const int &start_y) {
    std::queue<std::pair<int, int>> heap;
    std::array<std::vector<std::pair<int, int>>, 2> captured_stones;
    bool visited[BOARD_SIZE][BOARD_SIZE]{};

    for(int k = 0; k <= 4; k++) {
        int i = start_x, j = start_y;
        if(k < 4) {
            i += direction_x[k];
            j += direction_y[k];
        }
        
        if(in_board(i, j) && !visited[i][j] && current_board.get_state(i, j) != StoneType::EMPTY) {
            visited[i][j] = true;

            bool alive = false;
            std::vector<std::pair<int, int>> stones;

            heap.emplace(i, j);
            while(!heap.empty()) {
                int u, v; std::tie(u, v) = heap.front(); heap.pop();
                stones.emplace_back(u, v);

                for(int d = 0; d < 4; d++) {
                    int x = u + direction_x[d];
                    int y = v + direction_y[d];
                    
                    if(in_board(x, y)) {
                        if(current_board.get_state(x, y) == StoneType::EMPTY) alive = true;
                        else {
                            if(current_board.get_state(x, y) == current_board.get_state(i, j) && !visited[x][y]) {
                                visited[x][y] = true;
                                heap.emplace(x, y);
                            }
                        }
                    }
                }
            }

            if(!alive) {
                while(!stones.empty()) {
                    captured_stones[current_board.get_state(i, j) == StoneType::WHITESTONE].push_back(stones.back());
                    stones.pop_back();
                }
            }
        }
    }

    return captured_stones;
}

int add_move(Board &current_board, Move new_move) {
    if(!in_board(new_move.pos_x, new_move.pos_y)) return 1;
    if(current_board.get_state(new_move.pos_x, new_move.pos_y) != StoneType::EMPTY) return 1;

    current_board.play_move(new_move);

    // Get captured stones
    std::array<std::vector<std::pair<int, int>>, 2> captured_stones = get_captured_stones(current_board, new_move.pos_x, new_move.pos_y);
    char current_stone_state = current_board.get_state(new_move.pos_x, new_move.pos_y);

    if (current_stone_state == StoneType::BLACKSTONE) {
        if (!captured_stones[0].empty() && captured_stones[1].empty()) {
            current_board.undo_move(false);
            return 1;
        } else {
            new_move.captured_stones = captured_stones[1];
        }
    } 
    else {
        if (!captured_stones[1].empty() && captured_stones[0].empty()) {
            current_board.undo_move(false);
            return 1;
        } else {
            new_move.captured_stones = captured_stones[0];
        }
    }

    // Actually making move
    current_board.undo_move(false);
    current_board.play_move(new_move);

    if(current_board.check_existed_state()) {
        current_board.undo_move(false);
        return 2;
    }

    current_board.clear_undo_list();
    return 0;
}   

std::pair<int, int> check_ko_rule(Board &current_board) {
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            int exit_code = add_move(current_board, Move(i, j, current_board.get_turn() == 1 ? StoneType::BLACKSTONE : StoneType::WHITESTONE));
            if(exit_code == 0) current_board.undo_move(false);
            else if(exit_code == 2) {
                return std::make_pair(i, j);
            }
        }
    }
    return std::make_pair(-1, -1);
}

bool check_ko_threat(Board &current_board, Move new_move) {
    if(check_ko_rule(current_board) == std::make_pair(-1, -1)) return false;
    if(add_move(current_board, new_move)) return false;
    
    bool visited[BOARD_SIZE][BOARD_SIZE]{};
    StoneType opponent = opposite_stone(new_move.stone_type);

    for(int k = 0; k < 4; k++) {
        int i = new_move.pos_x + direction_x[k];
        int j = new_move.pos_y + direction_y[k];
        
        if(in_board(i, j)) {
            std::queue<std::pair<int, int>> heap;
            memset(visited, false, sizeof(visited));
            
            int liberties = 0;
            visited[i][j] = true;
            heap.emplace(i, j);

            while(!heap.empty()) {
                int u, v; std::tie(u, v) = heap.front(); heap.pop();

                for(int d = 0; d < 4; d++) {
                    int x = u + direction_x[d];
                    int y = v + direction_y[d];
                    
                    if(in_board(x, y)) {
                        if(current_board.get_state(x, y) == opponent && !visited[x][y]) {
                            visited[x][y] = true;
                            heap.emplace(x, y);
                        } else if(current_board.get_state(x, y) == StoneType::EMPTY) {
                            liberties++;
                            if(liberties > 1) break;
                        }
                    }
                }
            }

            if(liberties == 1) {
                current_board.undo_move(false);
                return true;
            }
        }
    }

    current_board.undo_move(false);
    return false;
}