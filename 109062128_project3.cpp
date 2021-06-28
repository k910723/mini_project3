#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

struct Point {
    int x, y;
    Point(int x, int y) : x(x), y(y) {}
};

const std::array<Point, 8> directions{ {
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    } };

int player;
const int SIZE = 8;
const int SEARCH_DEPTH = 5;
const int inf = 10000, neg_inf = -10000;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;
std::vector<std::pair<Point, int>> next_moves;
Point next_move(0, 0);

void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({ x, y });
    }
}

void write_random_spot(std::ofstream& fout) {
    int n_valid_spots = next_valid_spots.size();
    srand(time(NULL));
    // Choose random spot. (Not random uniform here)
    int index = (rand() % n_valid_spots);
    Point p = next_valid_spots[index];
    // Remember to flush the output to ensure the last action is written to file.
    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}

void write_next_spot(std::ofstream& fout) {
    fout << next_move.x << " " << next_move.y << std::endl;
    fout.flush();
}

void find_nextmove();
int Value(std::array<std::array<int, SIZE>, SIZE> cur_board);
int weight(std::array<std::array<int, SIZE>, SIZE> cur_board);
int minimax(int depth, bool maximizing, int alpha, int beta, std::array<std::array<int, SIZE>, SIZE> cur_board);
std::vector<Point> get_valid_spots(std::array<std::array<int, SIZE>, SIZE> cur_board, int cur_player);
bool is_spot_valid(Point center, std::array<std::array<int, SIZE>, SIZE> cur_board, int cur_player);
bool is_disc_at(Point p, int disc, std::array<std::array<int, SIZE>, SIZE> cur_board);
std::array<std::array<int, SIZE>, SIZE> flip_discs(Point center, std::array<std::array<int, SIZE>, SIZE> cur_board, int cur_player);

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);

    //write_random_spot(fout);

    find_nextmove();
    write_next_spot(fout);

    fin.close();
    fout.close();
    return 0;
}

void find_nextmove() {
    int val = minimax(SEARCH_DEPTH, true, neg_inf, inf, board);
    for (auto next : next_moves) {
        if (next.second == val) {
            next_move = next.first;
            break;
        }
    }
}

int Value(std::array<std::array<int, SIZE>, SIZE> cur_board) {
    int val = 0;
    /*
    int blackNum = 0, whiteNum = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (cur_board[i][j] == 1) blackNum++;
            else if (cur_board[i][j] == 2) whiteNum++;
        }
    }
    if (player == 1) val = blackNum - whiteNum; // plays as balck
    else if (player == 2) val = whiteNum - blackNum; // plays as white
    */
    val += weight(cur_board);

    return val;
}

int weight(std::array<std::array<int, SIZE>, SIZE> cur_board) {
    const int weight_table[SIZE][SIZE] =
    {
        { 500, -100, 70, 40, 40, 70, -100,  500},
        {-100, -200, 10, 3,  3,  10, -200, -100},
        { 70,   10,  15, 10, 10, 15,  10,   70},
        { 40,   3,   10, 3,  3,  10,  3,    40},
        { 40,   3,   10, 3,  3,  10,  3,    40},
        { 70,   10,  15, 10, 10, 15,  10,   70},
        {-100, -200, 10, 3,  3,  10, -200, -100},
        { 500, -100, 70, 40, 40, 70, -100,  500},
    };
    int val = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (cur_board[i][j] == player) val += weight_table[i][j];
            else if (cur_board[i][j] == (3 - player)) val -= weight_table[i][j];
        }
    }
    return val;
}

int minimax(int depth, bool maximizing, int alpha, int beta, std::array<std::array<int, SIZE>, SIZE> cur_board) {
    int cur_p = (maximizing == true) ? player : 3 - player;
    std::vector<Point> valid_spots = get_valid_spots(cur_board, cur_p);
    if (depth == 0 || valid_spots.empty()) return Value(cur_board);

    int ret;
    if (maximizing) {
        ret = neg_inf;
        if (depth == SEARCH_DEPTH) next_moves.clear();
        for (auto next : valid_spots) {
            auto tmp = cur_board;
            tmp[next.x][next.y] = cur_p;
            tmp = flip_discs(next, tmp, cur_p);
            int _ret = minimax(depth - 1, false, alpha, beta, tmp);
            ret = std::max(ret, _ret);
            alpha = std::max(alpha, ret);
            if (depth == SEARCH_DEPTH) next_moves.push_back(std::make_pair(next, _ret));
            if (alpha >= beta) break;
        }
        return ret;
    }
    else {
        ret = inf;
        for (auto next : valid_spots) {
            auto tmp = cur_board;
            tmp[next.x][next.y] = cur_p;
            tmp = flip_discs(next, tmp, cur_p);
            ret = std::min(ret, minimax(depth - 1, true, alpha, beta, tmp));
            beta = std::min(beta, ret);
            if (beta <= alpha) break;
        }
        return ret;
    }
}

std::vector<Point> get_valid_spots(std::array<std::array<int, SIZE>, SIZE> cur_board, int cur_player) {
    std::vector<Point> valid_spots;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            Point p(i, j);
            if (cur_board[i][j] != 0)
                continue;
            if (is_spot_valid(p, cur_board, cur_player))
                valid_spots.push_back(p);
        }
    }
    return valid_spots;
}

bool is_spot_valid(Point center, std::array<std::array<int, SIZE>, SIZE> cur_board, int cur_player) {
    if (cur_board[center.x][center.y] != 0)
        return false;
    for (Point dir : directions) {
        // Move along the direction while testing.
        Point p(center.x + dir.x, center.y + dir.y);
        if (!is_disc_at(p, 3 - cur_player, cur_board))
            continue;
        p.x = p.x + dir.x;
        p.y = p.y + dir.y;
        while (p.x >= 0 && p.x < SIZE && p.y >= 0 && p.y < SIZE && cur_board[p.x][p.y] != 0) {
            if (is_disc_at(p, cur_player, cur_board))
                return true;
            p.x = p.x + dir.x;
            p.y = p.y + dir.y;
        }
    }
    return false;
}

bool is_disc_at(Point p, int disc, std::array<std::array<int, SIZE>, SIZE> cur_board) {
    if (p.x < 0 || p.x >= SIZE || p.y < 0 || p.y >= SIZE)
        return false;
    if (cur_board[p.x][p.y] != disc)
        return false;
    return true;
}

std::array<std::array<int, SIZE>, SIZE> flip_discs(Point center, std::array<std::array<int, SIZE>, SIZE> cur_board, int cur_player) {
    for (Point dir : directions) {
        // Move along the direction while testing.
        Point p(center.x + dir.x, center.y + dir.y);
        if (!is_disc_at(p, 3 - cur_player, cur_board))
            continue;
        std::vector<Point> discs({ p });
        p.x = p.x + dir.x;
        p.y = p.y + dir.y;
        while (p.x >= 0 && p.x < SIZE && p.y >= 0 && p.y < SIZE && cur_board[p.x][p.y] != 0) {
            if (is_disc_at(p, cur_player, cur_board)) {
                for (Point s : discs) {
                    cur_board[s.x][s.y] = cur_player;
                }
                break;
            }
            discs.push_back(p);
            p.x = p.x + dir.x;
            p.y = p.y + dir.y;
        }
    }
    return cur_board;
}
