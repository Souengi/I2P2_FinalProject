#include <array>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <queue>
#include <vector>
#include<cassert>
#include<cstring>
#include<cstdint>

#define inf 1e9

const int DEPTH = 5;
const int SIZE = 8;
using Board = std::array<std::array<int, SIZE>, SIZE>;
using std::vector;


struct Point {
    int x, y;
    Point() : Point(0, 0) {}
    Point(int x, int y) : x(x), y(y) {}
    bool operator==(const Point& rhs) const { 
        return x == rhs.x && y == rhs.y; 
    }
    bool operator!=(const Point& rhs) const { 
        return !operator==(rhs); 
    }
    Point operator+(const Point& rhs) const
    {
        return Point(x + rhs.x, y + rhs.y);
    }
    Point operator-(const Point& rhs) const
    {
        return Point(x - rhs.x, y - rhs.y);
    }

};


int player;
Board board;
vector<Point> next_valid_spots;
Point bestMove;

enum SPOT_STATE { EMPTY = 0, BLACK = 1, WHITE = 2 };
const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
const std::array<Point, 4> corners = {{ 
    Point(0, 0), Point(0, 7), Point(7, 0), Point(7, 7) 
}};
int getNextPlayer(int player) { 
    return 3 - player; 
}
bool is_spot_on_board(Point p){ 
    return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE; 
}
int get_disc(Board _board, Point p) { 
    return _board[p.x][p.y]; 
}
void set_disc(Board& _board, Point p, int disc) { 
    _board[p.x][p.y] = disc; 
}
bool is_disc_at(Board _board, Point p, int disc)
{
    if (!is_spot_on_board(p)) return false;
    if (get_disc(_board, p) != disc) return false;
    return true;
}
bool is_spot_valid(Board _board, Point center, int curPlayer) {
    if (get_disc(_board, center) != EMPTY)
        return false;
    for (Point dir: directions) {
        Point p = center + dir;
        if (!is_disc_at(_board, p, getNextPlayer(curPlayer)))
            continue;
        p = p + dir;
        while (is_spot_on_board(p) && get_disc(_board, p) != EMPTY) {
            if (is_disc_at(_board, p, curPlayer))
                return true;
            p = p + dir;
        }
    }
    return false;
}
vector<Point> get_valid_spots(Board _board, int curPlayer) {
    std::vector<Point> v;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            Point p = Point(i, j);
            if (_board[i][j] != EMPTY)
                continue;
            if (is_spot_valid(_board, p, curPlayer))
                v.push_back(p);
        }
    }
    return v;
}
void flip_discs(Board& _board, Point center, int curPlayer)
{
    for (Point dir : directions) {
        Point p = center + dir;
        if (!is_disc_at(_board, p, getNextPlayer(curPlayer))) continue;
        std::vector<Point> discs({p});
        p = p + dir;
        while (is_spot_on_board(p) && get_disc(_board, p) != EMPTY) {
            if (is_disc_at(_board, p, curPlayer)) {
                for (Point s : discs) {
                    set_disc(_board, s, curPlayer);
                }
                break;
            }
            discs.push_back(p);
            p = p + dir;
        }
    }
}
void put_disc(Board& _board, Point p, int curPlayer)
{
    set_disc(_board, p, curPlayer);
    flip_discs(_board, p, curPlayer);
}
double Board_value(Board _board, int curPlayer)
{
    //int board_valid_spot = get_valid_spots(_board, curPlayer).size();
    int count[3] = {};
    double V = 0, D = 0, C = 0, CS = 0, MC = 0, SC = 0;
    Board player_weight = {
        40, -6, 15, 10, 10, 15, -6, 40,
        -6, -9, -6, -1, -1, -6, -9, -6,
        15, -6, 2, 2, 2, 2, -6, 15,
        10, 1, 2, -3, -3, 2, 1, 10,
        10, 1, 2, -3, -3, 2, 1, 10,
        15, -6, 2, 2, 2, 2, -6, 15,
        -6, -9, -6, -1, -1, -6, -9, -6,
        40, -6, 15, 10, 10, 15, -6, 40
    };
    // Position Values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (_board[i][j] == curPlayer)
                V += player_weight[i][j];
            else if (_board[i][j] == getNextPlayer(curPlayer))
                V -= player_weight[i][j];
            count[_board[i][j]]++;
        }
    }
    SC = count[curPlayer] - count[getNextPlayer(curPlayer)];

    // Corners Captured
    count[1] = count[2] = 0;
    for (Point c : corners)
        count[_board[c.x][c.y]]++;


    C = 25 * (count[curPlayer] - count[getNextPlayer(curPlayer)]);
    //int free_space = count[EMPTY];
    // DISC COUNT
    if (count[curPlayer] == count[getNextPlayer(curPlayer)]) D = 0;
    else {
        D = (count[curPlayer] - count[getNextPlayer(curPlayer)]) /
            (count[curPlayer] + count[getNextPlayer(curPlayer)]);
    }
    count[1] = count[2] = 0;
    for (Point c : corners) {
        if (_board[c.x][c.y] == EMPTY) {
            for (int j = 1; j <= 7; j += 2) {
                Point p = c + directions[j];
                if (is_spot_on_board(p)) count[_board[p.x][p.y]]++;
            }
        }
    }

    CS = -20.25 * (count[curPlayer] - count[getNextPlayer(curPlayer)]);
    // Valid Moves Count
    count[curPlayer] = get_valid_spots(_board, curPlayer).size();
    count[getNextPlayer(curPlayer)] =
        get_valid_spots(_board, getNextPlayer(curPlayer)).size();
    if (count[curPlayer] == count[getNextPlayer(curPlayer)]) MC = 0;
    else {
        MC = (count[curPlayer] - count[getNextPlayer(curPlayer)]) /
            (count[curPlayer] + count[getNextPlayer(curPlayer)]);
    }

    double score = (20 * V) + (204.451 * D) + (541 * MC) + (375.78 * CS) + (805.131 * C) + SC;
    return score;
}
double Alphabeta_search(Board _board, int depth, int curPlayer, double a, double b) {
    if (depth == 0) return Board_value(_board, curPlayer);

    vector <Point> nextMove = get_valid_spots(_board, curPlayer);
    if (nextMove.size() == 0) return Alphabeta_search(_board, depth - 1, getNextPlayer(curPlayer), a, b);
    double val;
    if (curPlayer == player) {//Maximizing
        val = -inf;
        for (Point p : nextMove) {
            Board next = _board;
            put_disc(next, p, curPlayer);
            val = std::max(val, Alphabeta_search(next, depth - 1, getNextPlayer(curPlayer), a, b));

            a = std::max(a, val);
            if (a >= b) break;
        }
        return val;
    }else {// Minimizing
        val = inf;
        for (Point p : nextMove) {
            Board oppo_next = _board;
            put_disc(oppo_next, p , curPlayer);
            val = std::min(val, Alphabeta_search(oppo_next, depth - 1, getNextPlayer(curPlayer), a, b));

            b = std::min(b, val);
            if (b <= a) break;
        }
        return val;
    }
}
Point find_spot_value(int depth, std::ofstream& fout) {
    int d_depth = depth;
    //if(next_valid_spots.size() > 6) d_depth--;
    //if(next_valid_spots.size() > 4) d_depth--;
    //if(next_valid_spots.size() < 4) d_depth++;
    double bestVal = -inf;
    for (auto p : next_valid_spots) {
        Board valid_board = board;
        put_disc(valid_board, p , player);
        double spot_value = Alphabeta_search(valid_board, d_depth, getNextPlayer(player), -inf, inf);

        if (spot_value > bestVal) {
            bestVal = spot_value;
            bestMove = p;
        }


        fout << bestMove.x << " " << bestMove.y << "\n";
    }
    return bestMove;
}

void read_board(std::ifstream& fin)
{
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin)
{
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}

void write_valid_spot(std::ofstream& fout)
{
    Point p;
    p = find_spot_value(DEPTH, fout);
    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}
int main(int, char** argv)
{
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
