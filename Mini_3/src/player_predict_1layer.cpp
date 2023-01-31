#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

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

int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;
const std::array<Point, 8> directions{{
    Point(-1, -1), Point(-1, 0), Point(-1, 1),
    Point(0, -1), /*{0, 0}, */Point(0, 1),
    Point(1, -1), Point(1, 0), Point(1, 1)
}};
std::vector<int> predicted_value;
std::array<int, 3> disc_count;

int opposite_player(int player) {
    return 3 - player;
}

bool is_on_board(Point p) {
    return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
}

int get_disc(Point p){
    return board[p.x][p.y];
}

void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
            if(board[i][j] == player) disc_count[player]++;
            if(board[i][j] == opposite_player(player)) disc_count[opposite_player(player)]++;
        }
    }
}

bool is_disc_at(Point p, int disc) {
    if (!is_on_board(p))
         return false;
    if (get_disc(p) != disc)
        return false;
    return true;
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

void predict_value(){
    for(Point next: next_valid_spots){
        std::array<int, 3> next_disc_count = disc_count;
        for(Point dir: directions){
            Point p = next + dir;
            if(!is_disc_at(p, opposite_player(player)))
                continue;
            std::vector<Point> disc({p});
            p = p + dir;
            while(is_on_board(p) && get_disc(p) != 0){
                if(is_disc_at(p, player)){
                    next_disc_count[player] += disc.size();
                    next_disc_count[opposite_player(player)] -= disc.size();
                    break;
                }
                disc.push_back(p);
                p = p + dir;
            }
        }
        predicted_value.push_back(next_disc_count[player] - next_disc_count[opposite_player(player)]);
    }
}

void write_valid_spot(std::ofstream& fout) {
    int n_valid_spots = next_valid_spots.size();
    //srand(time(NULL));
    //stop random dude
    // Choose random spot. (Not random uniform here)
    int index;
    int value = -9999;
    for(int i = 0; i < n_valid_spots; i++){
        if(predicted_value[i] >= value){
            value = predicted_value[i];
            index = i;
        }
    }
    Point p = next_valid_spots[index];
    //these line for write to file
    // Remember to flush the output to ensure the last action is written to file.
    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    predict_value();
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
