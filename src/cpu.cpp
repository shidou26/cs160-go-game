#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <limits>
#include <queue>
#include <random>
#include <tuple>
#include <utility>
#include <vector>

#include "../include/cpu.h"
#include "../include/rule.h"
#include "../include/score.h"

namespace {

std::mt19937 rng(std::random_device{}());

std::pair<int, int> get_group_info(const Board &current_board, int start_x, int start_y, bool visited_group[BOARD_SIZE][BOARD_SIZE]) {
    StoneType color = current_board.get_state(start_x, start_y);
    if (color == StoneType::EMPTY) return {0, 0};

    std::queue<std::pair<int, int>> heap;
    bool visited_liberty[BOARD_SIZE][BOARD_SIZE]{};

    int stones_count = 0;
    int liberties_count = 0;

    visited_group[start_x][start_y] = true;
    heap.emplace(start_x, start_y);

    while (!heap.empty()) {
        int u, v;
        std::tie(u, v) = heap.front();
        heap.pop();
        stones_count++;

        for (int d = 0; d < 4; d++) {
            int x = u + direction_x[d];
            int y = v + direction_y[d];

            if (current_board.in_board(x, y)) {
                StoneType state = current_board.get_state(x, y);
                if (state == StoneType::EMPTY) {
                    if (!visited_liberty[x][y]) {
                        visited_liberty[x][y] = true;
                        liberties_count++;
                    }
                } else if (state == color && !visited_group[x][y]) {
                    visited_group[x][y] = true;
                    heap.emplace(x, y);
                }
            }
        }
    }

    return {stones_count, liberties_count};
}

int count_group_liberties_at(const Board &current_board, int start_x, int start_y) {
    bool visited_group[BOARD_SIZE][BOARD_SIZE]{};
    return get_group_info(current_board, start_x, start_y, visited_group).second;
}

bool is_own_eye(const Board &current_board, int x, int y, StoneType stone) {
    if (current_board.get_state(x, y) != StoneType::EMPTY) return false;

    for (int d = 0; d < 4; d++) {
        int nx = x + direction_x[d];
        int ny = y + direction_y[d];
        if (current_board.in_board(nx, ny)) {
            if (current_board.get_state(nx, ny) != stone) return false;
            if (count_group_liberties_at(current_board, nx, ny) <= 1) return false;
        }
    }

    int diag_x[4] = {1, 1, -1, -1};
    int diag_y[4] = {1, -1, 1, -1};
    int friendly_diag = 0;
    int total_diag = 0;

    for (int d = 0; d < 4; d++) {
        int nx = x + diag_x[d];
        int ny = y + diag_y[d];
        if (current_board.in_board(nx, ny)) {
            total_diag++;
            if (current_board.get_state(nx, ny) == stone) {
                friendly_diag++;
            }
        }
    }

    return (total_diag - friendly_diag) <= 1;
}

bool has_stone_nearby(const Board &current_board, int x, int y, int radius) {
    int n = current_board.get_size();
    for (int dx = -radius; dx <= radius; dx++) {
        for (int dy = -radius; dy <= radius; dy++) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < n && ny >= 0 && ny < n) {
                if (current_board.get_state(nx, ny) != StoneType::EMPTY) {
                    return true;
                }
            }
        }
    }
    return false;
}

double positional_bonus(int x, int y, int board_size) {
    int dist_x = std::min(x, board_size - 1 - x);
    int dist_y = std::min(y, board_size - 1 - y);
    int min_edge = std::min(dist_x, dist_y);

    double bonus = 0.0;
    if (min_edge == 0) {
        bonus -= 3.0;
    } else if (min_edge == 2 || min_edge == 3) {
        bonus += 2.5;
    } else if (min_edge == 1) {
        bonus += 0.5;
    }

    double center = (board_size - 1) / 2.0f;
    double dist_center = std::abs(x - center) + std::abs(y - center);
    bonus += std::max(0.0, (board_size - dist_center) * 0.15);

    return bonus;
}

double get_real_score_diff(const Board &current_board, StoneType ai_stone) {
    std::pair<double, double> sc = scoring(current_board);
    return (ai_stone == StoneType::BLACKSTONE) ? (sc.first - sc.second) : (sc.second - sc.first);
}

bool is_endgame(const Board &current_board) {
    int n = current_board.get_size();
    int occupied = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (current_board.get_state(i, j) != StoneType::EMPTY) occupied++;
        }
    }
    return occupied >= (int)(n * n * 0.65);
}

bool should_pass_on_opponent_pass(const Board &current_board, StoneType ai_stone) {
    if (current_board.get_pass() < 1) return false;
    return get_real_score_diff(current_board, ai_stone) > 0.0;
}

double evaluate_board(const Board &current_board, StoneType ai_stone) {
    int n = current_board.get_size();

    int ai_captured = (ai_stone == StoneType::BLACKSTONE) ? current_board.get_captured(1) : current_board.get_captured(0);
    int opp_captured = (ai_stone == StoneType::BLACKSTONE) ? current_board.get_captured(0) : current_board.get_captured(1);

    // Tactical capture bonus (on top of territory + captures + komi from scoring())
    double score = (ai_captured - opp_captured) * 25.0;

    bool has_black = false;
    bool has_white = false;
    bool visited[BOARD_SIZE][BOARD_SIZE]{};

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            StoneType state = current_board.get_state(i, j);
            if (state == StoneType::EMPTY) continue;
            if (state == StoneType::BLACKSTONE) has_black = true;
            else has_white = true;

            double pos_val = positional_bonus(i, j, n) * 0.6;
            if (state == ai_stone) score += pos_val;
            else score -= pos_val;

            if (!visited[i][j]) {
                auto [group_size, liberties] = get_group_info(current_board, i, j, visited);
                double group_val = std::min(liberties, 6) * 2.5;

                if (liberties == 1) {
                    group_val -= group_size * 28.0;
                } else if (liberties == 2) {
                    group_val -= group_size * 8.0;
                }

                if (state == ai_stone) {
                    score += group_val;
                } else {
                    score -= group_val;
                }
            }
        }
    }

    // Include real Go territory + captures + Komi once both players have stones on the board
    if (has_black && has_white) {
        score += get_real_score_diff(current_board, ai_stone) * 14.0;
    }

    return score;
}

double score_move_quick(Board &board, int x, int y, StoneType stone, bool &is_valid) {
    is_valid = false;
    if (!board.in_board(x, y) || board.get_state(x, y) != StoneType::EMPTY) return -1e9;
    if (is_own_eye(board, x, y, stone)) return -1e9;

    StoneType opponent = opposite_stone(stone);

    int saved_friendly_stones = 0;

    for (int d = 0; d < 4; d++) {
        int nx = x + direction_x[d];
        int ny = y + direction_y[d];
        if (board.in_board(nx, ny)) {
            StoneType adj_state = board.get_state(nx, ny);
            if (adj_state == stone) {
                bool vis[BOARD_SIZE][BOARD_SIZE]{};
                auto [g_size, g_libs] = get_group_info(board, nx, ny, vis);
                if (g_libs == 1) saved_friendly_stones += g_size;
            }
        }
    }

    int cap_before = (stone == StoneType::BLACKSTONE) ? board.get_captured(1) : board.get_captured(0);

    if (add_move(board, Move(x, y, stone)) != 0) {
        return -1e9;
    }

    is_valid = true;
    int cap_after = (stone == StoneType::BLACKSTONE) ? board.get_captured(1) : board.get_captured(0);
    int captured_now = cap_after - cap_before;

    bool vis_self[BOARD_SIZE][BOARD_SIZE]{};
    auto [my_size, my_libs] = get_group_info(board, x, y, vis_self);

    double heuristic = positional_bonus(x, y, board.get_size());
    heuristic += captured_now * 60.0;

    if (my_libs == 1) {
        heuristic -= my_size * 45.0;
    } else {
        heuristic += std::min(my_libs, 5) * 4.0;
        if (saved_friendly_stones > 0) {
            heuristic += saved_friendly_stones * 35.0;
        }
    }

    for (int d = 0; d < 4; d++) {
        int nx = x + direction_x[d];
        int ny = y + direction_y[d];
        if (board.in_board(nx, ny) && board.get_state(nx, ny) == opponent) {
            bool vis_opp[BOARD_SIZE][BOARD_SIZE]{};
            auto [opp_size, opp_libs] = get_group_info(board, nx, ny, vis_opp);
            if (opp_libs == 1) {
                heuristic += opp_size * 22.0;
            } else if (opp_libs == 2) {
                heuristic += opp_size * 6.0;
            }
        }
    }

    board.undo_move(false);
    return heuristic;
}

std::vector<Move> get_candidate_moves(Board &board, StoneType stone, int max_candidates) {
    int n = board.get_size();
    bool any_stone_on_board = false;

    for (int i = 0; i < n && !any_stone_on_board; i++) {
        for (int j = 0; j < n; j++) {
            if (board.get_state(i, j) != StoneType::EMPTY) {
                any_stone_on_board = true;
                break;
            }
        }
    }

    std::vector<std::pair<double, Move>> scored_moves;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (board.get_state(i, j) != StoneType::EMPTY) continue;

            if (any_stone_on_board && !has_stone_nearby(board, i, j, 2)) {
                int dist_x = std::min(i, n - 1 - i);
                int dist_y = std::min(j, n - 1 - j);
                bool is_star = (dist_x == 2 || dist_x == 3) && (dist_y == 2 || dist_y == 3);
                if (!is_star) continue;
            }

            bool is_valid = false;
            double h_score = score_move_quick(board, i, j, stone, is_valid);
            if (is_valid) {
                scored_moves.push_back({h_score, Move(i, j, stone)});
            }
        }
    }

    if (scored_moves.empty()) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (board.get_state(i, j) != StoneType::EMPTY) continue;
                bool is_valid = false;
                double h_score = score_move_quick(board, i, j, stone, is_valid);
                if (is_valid) {
                    scored_moves.push_back({h_score, Move(i, j, stone)});
                }
            }
        }
    }

    std::sort(scored_moves.begin(), scored_moves.end(), [](const auto &a, const auto &b) {
        return a.first > b.first;
    });

    std::vector<Move> candidates;
    int limit = std::min((int)scored_moves.size(), max_candidates);
    for (int i = 0; i < limit; i++) {
        candidates.push_back(scored_moves[i].second);
    }

    return candidates;
}

double minimax(Board &board, int depth, bool is_maximizing, StoneType ai_stone) {
    if (depth == 0) {
        return evaluate_board(board, ai_stone);
    }

    StoneType current_stone = is_maximizing ? ai_stone : opposite_stone(ai_stone);
    std::vector<Move> candidates = get_candidate_moves(board, current_stone, 6);

    if (candidates.empty()) {
        return evaluate_board(board, ai_stone);
    }

    if (is_maximizing) {
        double best_val = -1e18;
        for (const Move &m : candidates) {
            if (add_move(board, m) == 0) {
                double val = minimax(board, depth - 1, false, ai_stone);
                board.undo_move(false);
                best_val = std::max(best_val, val);
            }
        }
        return (best_val <= -1e17) ? evaluate_board(board, ai_stone) : best_val;
    } else {
        double best_val = 1e18;
        for (const Move &m : candidates) {
            if (add_move(board, m) == 0) {
                double val = minimax(board, depth - 1, true, ai_stone);
                board.undo_move(false);
                best_val = std::min(best_val, val);
            }
        }
        return (best_val >= 1e17) ? evaluate_board(board, ai_stone) : best_val;
    }
}

double minimax_alpha_beta(Board &board, int depth, double alpha, double beta, bool is_maximizing, StoneType ai_stone,
                          std::chrono::steady_clock::time_point deadline) {
    if (depth == 0 || std::chrono::steady_clock::now() >= deadline) {
        return evaluate_board(board, ai_stone);
    }

    StoneType current_stone = is_maximizing ? ai_stone : opposite_stone(ai_stone);
    int branch_limit = (board.get_size() >= 19) ? 5 : ((board.get_size() >= 13) ? 6 : 7);
    std::vector<Move> candidates = get_candidate_moves(board, current_stone, branch_limit);

    if (candidates.empty()) {
        return evaluate_board(board, ai_stone);
    }

    if (is_maximizing) {
        double max_eval = -1e18;
        for (const Move &m : candidates) {
            if (std::chrono::steady_clock::now() >= deadline) break;
            if (add_move(board, m) == 0) {
                double eval = minimax_alpha_beta(board, depth - 1, alpha, beta, false, ai_stone, deadline);
                board.undo_move(false);
                max_eval = std::max(max_eval, eval);
                alpha = std::max(alpha, eval);
                if (beta <= alpha) {
                    break;
                }
            }
        }
        return (max_eval <= -1e17) ? evaluate_board(board, ai_stone) : max_eval;
    } else {
        double min_eval = 1e18;
        for (const Move &m : candidates) {
            if (std::chrono::steady_clock::now() >= deadline) break;
            if (add_move(board, m) == 0) {
                double eval = minimax_alpha_beta(board, depth - 1, alpha, beta, true, ai_stone, deadline);
                board.undo_move(false);
                min_eval = std::min(min_eval, eval);
                beta = std::min(beta, eval);
                if (beta <= alpha) {
                    break;
                }
            }
        }
        return (min_eval >= 1e17) ? evaluate_board(board, ai_stone) : min_eval;
    }
}

} // namespace

Move easy_play(Board &board) {
    StoneType ai_stone = (board.get_turn() == 1) ? StoneType::BLACKSTONE : StoneType::WHITESTONE;
    if (should_pass_on_opponent_pass(board, ai_stone)) {
        return Move(-1, -1, ai_stone);
    }

    int n = board.get_size();

    std::vector<Move> safe_moves;
    std::vector<Move> fallback_moves;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (board.get_state(i, j) != StoneType::EMPTY) continue;
            if (is_own_eye(board, i, j, ai_stone)) continue;

            Move candidate(i, j, ai_stone);
            if (add_move(board, candidate) == 0) {
                int libs = count_group_liberties_at(board, i, j);
                board.undo_move(false);

                if (libs > 1) {
                    safe_moves.push_back(candidate);
                } else {
                    fallback_moves.push_back(candidate);
                }
            }
        }
    }

    if (!safe_moves.empty()) {
        std::uniform_int_distribution<int> dist(0, (int)safe_moves.size() - 1);
        return safe_moves[dist(rng)];
    }

    if (board.get_pass() >= 1) {
        return Move(-1, -1, ai_stone);
    }

    if (!fallback_moves.empty()) {
        std::uniform_int_distribution<int> dist(0, (int)fallback_moves.size() - 1);
        return fallback_moves[dist(rng)];
    }

    return Move(-1, -1, ai_stone);
}

Move medium_play(Board &board) {
    StoneType ai_stone = (board.get_turn() == 1) ? StoneType::BLACKSTONE : StoneType::WHITESTONE;
    if (should_pass_on_opponent_pass(board, ai_stone)) {
        return Move(-1, -1, ai_stone);
    }

    std::vector<Move> candidates = get_candidate_moves(board, ai_stone, 8);
    if (candidates.empty()) {
        return Move(-1, -1, ai_stone);
    }

    std::uniform_real_distribution<double> mistake_noise(-8.0, 8.0);
    Move best_move = candidates[0];
    double max_raw_score = -1e18;
    double best_noisy_score = -1e18;

    for (const Move &m : candidates) {
        if (add_move(board, m) == 0) {
            double raw = minimax(board, 1, false, ai_stone);
            board.undo_move(false);
            if (raw > max_raw_score) max_raw_score = raw;

            double noisy = raw + mistake_noise(rng);
            if (noisy > best_noisy_score) {
                best_noisy_score = noisy;
                best_move = m;
            }
        }
    }

    // Only consider proactive Pass in endgame, comparing fairly (opponent still gets to move after Pass)
    if (is_endgame(board)) {
        double pass_after_opp = minimax(board, 1, false, ai_stone);
        if (max_raw_score < pass_after_opp - 2.0) {
            return Move(-1, -1, ai_stone);
        }
    }

    return best_move;
}

Move hard_play(Board &board) {
    StoneType ai_stone = (board.get_turn() == 1) ? StoneType::BLACKSTONE : StoneType::WHITESTONE;
    if (should_pass_on_opponent_pass(board, ai_stone)) {
        return Move(-1, -1, ai_stone);
    }

    std::vector<Move> candidates = get_candidate_moves(board, ai_stone, 10);
    if (candidates.empty()) {
        return Move(-1, -1, ai_stone);
    }

    // Use even-ply search (depth = 3 -> 4 plies: AI -> Human -> AI -> Human) to avoid odd-ply bias,
    // protected by an 800ms time budget so 19x19 never stalls.
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(800);
    int search_depth = 3;

    Move best_move = candidates[0];
    double best_score = -1e18;
    double alpha = -1e18;
    double beta = 1e18;

    for (const Move &m : candidates) {
        if (std::chrono::steady_clock::now() >= deadline && best_score > -1e17) break;
        if (add_move(board, m) == 0) {
            double score = minimax_alpha_beta(board, search_depth, alpha, beta, false, ai_stone, deadline);
            board.undo_move(false);

            if (score > best_score) {
                best_score = score;
                best_move = m;
            }
            alpha = std::max(alpha, best_score);
        }
    }

    // Only consider proactive Pass in endgame, comparing fairly (opponent still gets to move after Pass)
    if (is_endgame(board) && std::chrono::steady_clock::now() < deadline) {
        double pass_after_opp = minimax_alpha_beta(board, search_depth, -1e18, 1e18, false, ai_stone, deadline);
        if (best_score < pass_after_opp - 2.0) {
            return Move(-1, -1, ai_stone);
        }
    }

    return best_move;
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

    if (!board.in_board(seol.pos_x, seol.pos_y)) {
        board.toggle_turn();
        board.pass_turn();
    }

    return seol;
}
