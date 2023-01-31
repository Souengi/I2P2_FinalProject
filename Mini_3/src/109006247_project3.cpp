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

//#define max(a, b) a > b ? a : b;
//#define min(a, b) a < b ? a : b;

#define INF 1e9
const int Depth = 5;
const int SIZE = 8;
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
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};
using Board = std::array<std::array<int, SIZE>, SIZE>;
Point best;
int player;
//int opposite_player;

Board board;
//int player_disc;
//int opposite_disc;
std::vector<Point> next_valid_spots;
//std::vector<int> spots_value;
enum SPOT_STATE { 
    EMPTY = 0, BLACK = 1, WHITE = 2 
};
const std::array<Point, 8> directions{{
    Point(-1, -1), Point(-1, 0), Point(-1, 1),
    Point(0, -1), /*{0, 0}, */Point(0, 1),
    Point(1, -1), Point(1, 0), Point(1, 1)
}};
const std::array<std::array<int, SIZE>, SIZE> weight_point{{
    20, -3, 11, 8, 8, 11, -3, 20,
    -3, -7, -3, -1, -1, -3, -7, -3,
    11, -3, 2, 2, 2, 2, -3, 11,
    8, 1, 2, -3, -3, 2, 1, 8,
    8, 1, 2, -3, -3, 2, 1, 8,
    11, -3, 2, 2, 2, 2, -3, 11,
    -3, -7, -3, -1, -1, -3, -7, -3,
    20, -3, 11, 8, 8, 11, -3, 20
}};
const std::array<Point, 4> corners{{
    Point(0, 0), Point(0, 7), Point(7, 0), Point(7, 7)
}};



int get_next_player(int player){
    return 3 - player;
}
bool is_spot_on_board(Point p) {
    return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
}
void set_disc(Board& b, Point p, int disc){
    b[p.x][p.y] = disc;
}
int get_disc(Board b, Point p){
    return b[p.x][p.y];
}
bool is_disc_at(Board b, Point p, int disc){
    if(!is_spot_on_board(p))
        return false;
    if(get_disc(b, p) != disc)
        return false;
    return true;
}
void flip_disc(Board& b, Point center, int player){
    //int opposite =  3 - player;
    for(Point dir: directions){
        Point p = center + dir;
        if(!is_disc_at(b, p, get_next_player(player)))
            continue;
        std::vector<Point> discs({p});
        p = p + dir;
        while(is_spot_on_board(p) && get_disc(b, p) != EMPTY){
            if(is_disc_at(b, p, player)){
                for(Point s: discs){
                    set_disc(b, s, player);
                }
                break;
                //we count later
            }
            discs.push_back(p);
            p = p + dir;
        }
    }
}
bool is_spot_valid(Board b, Point center, int player){
    //int opposite = 3 - player;
    if(get_disc(b, center) != EMPTY)
        return false;
    for(Point dir: directions){
        Point p = center + dir;
        if(!is_disc_at(b, p, get_next_player(player)))
            continue;
        p = p + dir;
        while(is_spot_on_board(p) && get_disc(b, p) != EMPTY){
            if(is_disc_at(b, p, player))
                return true;
            p = p + dir;
        }
    }
    return false;
}
std::vector<Point> get_valid_spots(Board b, int player) {
    std::vector<Point> valid_spots;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            Point p = Point(i, j);
            if (b[i][j] != EMPTY)
                continue;
            if (is_spot_valid(b, p, player))
                valid_spots.push_back(p);
        }
    }
    return valid_spots;
}
void predict_board(Board& cur_board, Point move, int player){
    //no need to check valid since choose from valid spot
    //Board new_b = cur_board;
    //int opposite = 3 - player;
    set_disc(cur_board, move, player);
    flip_disc(cur_board, move, player);
    //return new_b;
}
double Board_value(Board b, int player){
    int coutn[3] = {};
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
    double Pos_Val = 0, End_Score = 0, Cor = 0, Dis_Ra = 0, Mov_Ra = 0, Cor_Sta = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if(b[i][j] == player) Pos_Val += player_weight[i][j];
            if(b[i][j] == get_next_player(player)) Pos_Val -= player_weight[i][j];
            coutn[b[i][j]]++;
        }
    }
    
    End_Score = coutn[player] - coutn[get_next_player(player)];
    if(coutn[player] == coutn[get_next_player(player)]) Dis_Ra = 0;
    else{
        Dis_Ra = (coutn[player] - coutn[get_next_player(player)]) / (coutn[player] + coutn[get_next_player(player)]);
    }

    coutn[1] = coutn[2] = 0;
    for(Point cn: corners){
        coutn[b[cn.x][cn.y]]++;
    }
    Cor = 25 * (coutn[player] - coutn[get_next_player(player)]);

    coutn[player] = get_valid_spots(b, player).size();
    coutn[get_next_player(player)] = get_valid_spots(b, get_next_player(player)).size();
    if(coutn[player] == coutn[get_next_player(player)]) Mov_Ra = 0;
    else{
        Mov_Ra = (coutn[player] - coutn[get_next_player(player)]) / (coutn[player] + coutn[get_next_player(player)]);
    }
    
    coutn[1] = coutn[2] = 0;
    for(Point c: corners){
        if (b[c.x][c.y] == EMPTY) {
            for (int j = 1; j <= 7; j += 2) {
                Point p = c + directions[j];
                if (is_spot_on_board(p)) coutn[b[p.x][p.y]]++;
            }
        }
    }

    Cor_Sta = -20.25 * (coutn[player] - coutn[get_next_player(player)]);

    double score = (20*Pos_Val) + End_Score + (805.131 * Cor) + (204.451 * Dis_Ra) + (541 * Mov_Ra) + (375.78 * Cor_Sta);
    return score;
}



double alphabeta(Board board, int depth, int cur_player, double alpha, double beta){
    //
    if(depth == 0){//reach depth step
        return Board_value(board, cur_player);
    }
    std::vector<Point> board_valid_spots = get_valid_spots(board, player);
    if(board_valid_spots.size() == 0)
        return alphabeta(board, depth - 1, get_next_player(cur_player), alpha, beta);
    double value;
    if(cur_player == player){//max player
        value = -INF;
        for(Point max_p: board_valid_spots){
            Board next = board;
            predict_board(board, max_p, cur_player);
            value = std::max(value, alphabeta(next, depth - 1, get_next_player(cur_player), alpha, beta));
            alpha = std::max(alpha, value);
            if(alpha >= beta)
                break; //beta cut
        }
        return value;
    }else{//min player
        value = INF;
        for(Point min_p: board_valid_spots){
            Board oppo_next = board;
            predict_board(board, min_p, cur_player);
            value = std::min(value, alphabeta(oppo_next, depth - 1, get_next_player(cur_player), alpha, beta));
            beta = std::min(beta, value);
            if(beta <= alpha)
                break; //alpha cut
        }
        return value;
    }
}//alpha-beta prunning //this just make search faster

void read_board(std::ifstream& fin) {
    fin >> player;
    //opposite_player = 3 - player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
            //if(board[i][j] == player) player_disc++;
            //if(board[i][j] == opposite_player) opposite_disc++;
        }
    }
}


void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}

Point find_spot_value(int depth, std::ofstream& fout){
    //int d_depth = depth;
    //if(next_valid_spots.size() > 6) d_depth--;
    //if(next_valid_spots.size() > 4) d_depth--;
    //if(next_valid_spots.size() < 4) d_depth++;
    double bestVal = -INF;
    for (auto p : next_valid_spots) {
        Board valid_board = board;
        predict_board(valid_board, p , player);
        double spot_value = alphabeta(valid_board, depth, get_next_player(player), -INF, INF);

        if (spot_value > bestVal) {
            bestVal = spot_value;
            best = p;
        }


        fout << best.x << " " << best.y << "\n";
    }
    return best;
}

void write_valid_spot(std::ofstream& fout) {
    //int n_valid_spots = next_valid_spots.size();
    //if(!next_valid_spots.size()) return; //no valid
    //find the max path
    Point p = find_spot_value(Depth, fout);    
    // Remember to flush the output to ensure the last action is written to file.
    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    //find_spot_value(board, player);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
