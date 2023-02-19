#include <cassert>
#include <iostream>
#include <vector>
#include <fstream>
#include <climits>

#define CONTEST 1

using namespace std;

class Pos {
public:
   int y, x;
};

enum class Response {
   not_broken, broken, finish, invalid
};

class Field {
public:
	int N;
	int C;
	vector<vector<int>> is_broken;
   vector<Pos> broken_cell;
   int broken_cell_counter;
	int total_cost;

	Field(int N, int C) : N(N), C(C), is_broken(N, vector<int>(N, 0)),
      broken_cell(N*N, Pos{-1,-1}), broken_cell_counter(0), total_cost(0) {}

	Response query(int y, int x, int power) {
		total_cost += power + C;
		cout << y << " " << x << " " << power << endl; // endl does flush
		int r;
		cin >> r;
		switch (r) {
		case 0:
			return Response::not_broken;
		case 1:
			is_broken[y][x] = 1;
         broken_cell[broken_cell_counter] = Pos{y,x};
         broken_cell_counter++;
			return Response::broken;
		case 2:
			is_broken[y][x] = 1;
         broken_cell[broken_cell_counter] = Pos{y,x};
         broken_cell_counter++;
         return Response::finish;
		default:
			return Response::invalid;
		}
	}
};

class Solver {
public:
	int N;
   int K;
	int C;
	vector<Pos> source_pos;
	vector<Pos> house_pos;
   vector<int> house_done;
	Field field;

	Solver(int N, const vector<Pos>& source_pos, const vector<Pos>& house_pos, int K, int C) : 
		N(N), source_pos(source_pos), house_pos(house_pos), K(K), C(C), house_done(K, 0), field(N, C) {
	}

	void solve() {
      // (初期化)水源を掘削
      for (Pos source : source_pos) {
         move(source, source);
      }

      // 最も近い掘削の完了したセルを探索
      for (int reach_count = 0; reach_count < K; reach_count++) {
         int shortest_source_dist = INT_MAX;
         int house_index;
         Pos shortest_broken_pos, shortest_house_pos;
         for (int i = 0; i < K; i++) {
            if (house_done[i]) continue;
            Pos house = {house_pos[i].y, house_pos[i].x};
            for (int j = 0; j < field.broken_cell_counter; j++) {
               int dist = abs(field.broken_cell[j].y - house.y) + abs(field.broken_cell[j].x - house.x);
               if (dist < shortest_source_dist) {
                  shortest_source_dist = dist;
                  shortest_broken_pos = Pos{field.broken_cell[j].y, field.broken_cell[j].x}; // ディープコピーしたい
                  shortest_house_pos = Pos{house.y, house.x};                                // ディープコピーしたい
                  house_index = i;
               }
            }
         }
         // 移動
         move(shortest_house_pos, shortest_broken_pos);
         house_done[house_index] = 1;
      }//*/
      
      /*
      // sample_code
		// from each house, go straight to the first source
		for (Pos house : house_pos) {
			move(house, source_pos[0]);   // startからgoalへの移動(start, goal)
		}
      */

		// should receive Response::finish and exit before entering here
		assert(false);
	}

	void move(Pos start, Pos goal) {
		// you can output comment
		cout << "# move from (" << start.y << "," << start.x << ") to (" << goal.y << "," << goal.x << ")" << endl;

		// down/up
		if (start.y < goal.y) {
			for (int y = start.y; y < goal.y; y++) {
				destruct(y, start.x);
			}
		} else {
			for (int y = start.y; y > goal.y; y--) {
				destruct(y, start.x);
			}
		}

		// right/left
		if (start.x < goal.x) {
			for (int x = start.x; x <= goal.x; x++) {
				destruct(goal.y, x);
			}
		} else {
			for (int x = start.x; x >= goal.x; x--) {
				destruct(goal.y, x);
			}
		}
	}

	void destruct(int y, int x) {
		// excavate (y, x) with fixed power until destruction
		const int power = 100;
		while (!field.is_broken[y][x]) {
			Response result = field.query(y, x, power);
			if (result == Response::finish) {
				//cerr << "total_cost = " << field.total_cost << endl;
				exit(0);
			} else if (result == Response::invalid) {
				cerr << "invalid: y=" << y << " x=" << x << endl;
				exit(1);
			}
		}
	}
};

int main() {
   
#if !CONTEST
   ifstream in("debug/tester/inst/0005.txt");
   if (!in) { cout << "error"; getchar(); }
   cin.rdbuf(in.rdbuf());
#endif

	int N, W, K, C;
	cin >> N >> W >> K >> C;

#if !CONTEST
   string line;
   for (int i = 0; i < N + 1; i++) {   // 最初の行の改行コードのため+1
      getline(cin, line);
   }
#endif

	vector<Pos> source_pos(W);
	vector<Pos> house_pos(K);
	for (int i = 0; i < W; i++) {
		cin >> source_pos[i].y >> source_pos[i].x;
	}
	for (int i = 0; i < K; i++) {
		cin >> house_pos[i].y >> house_pos[i].x;
	}

	Solver solver(N, source_pos, house_pos, K, C);
	solver.solve();
}
