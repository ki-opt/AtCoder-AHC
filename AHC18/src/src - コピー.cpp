#include <cassert>
#include <iostream>
#include <vector>
#include <fstream>
#include <climits>
#include <queue>

#define CONTEST 1 // true->コンテスト

#define POWER 100 							// 掘削のパワー
#define EXCAVATION_LIMIT 5				// 掘削回数の制限(POWER*EXCAVATION_LIMIT=掘削可能な頑丈さ)
#define Y_DIR 1
#define X_DIR 2

using namespace std;

class Pos {
public:
   int y, x;
};

class PosDir {
public:
	Pos pos;
	int dir;	// y方向=1, x方向=0
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
	vector<vector<int>> connected_to_source;	// 今のところ使わない？
	Field field;

	Solver(int N, const vector<Pos>& source_pos, const vector<Pos>& house_pos, int K, int C) : 
		N(N), source_pos(source_pos), house_pos(house_pos), K(K), C(C), house_done(K, 0), 
		connected_to_source(N, vector<int>(N, 0)), field(N, C) {
	}

	void solve() {
      // (初期化)水源を掘削 
		excavation_source();

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
                  shortest_broken_pos = Pos{field.broken_cell[j].y, field.broken_cell[j].x};
                  shortest_house_pos = Pos{house.y, house.x};
                  house_index = i;
               }
            }
         }
         // 移動
         repeat_partial_move(shortest_house_pos, shortest_broken_pos);
			//move(shortest_house_pos, shortest_broken_pos);
         house_done[house_index] = 1;
      }//*/
      
		// should receive Response::finish and exit before entering here
		assert(false);
	}

	// バグってるっぽい
	bool check_source_to_house(const Pos& start, const Pos& goal) {	// 2点間にパスが存在するか確認
		if (!field.is_broken[start.y][start.x]) return 0;	// スタート地点が掘削されていないので終了
		queue<Pos> que;
		que.push(start);
		while (!que.empty()) {
			Pos v = que.front(); que.pop();
			if (field.is_broken[v.y-1][v.x]) { 
				que.push(Pos{v.y-1, v.x});
				if (v.y-1 == goal.y && v.x == goal.x) return 1;
			}
			if (field.is_broken[v.y+1][v.x]) { 
				que.push(Pos{v.y+1, v.x}); 
				if (v.y+1 == goal.y && v.x == goal.x) return 1;
			}
			if (field.is_broken[v.y][v.x-1]) { 
				que.push(Pos{v.y, v.x-1}); 
				if (v.y == goal.y && v.x-1 == goal.x) return 1;
			}
			if (field.is_broken[v.y][v.x+1]) { 
				que.push(Pos{v.y, v.x+1});
				if (v.y == goal.y && v.x+1 == goal.x) return 1;
			}
		}
		return 0; 
	}

	Pos y_move(const Pos& start, const Pos& goal) {
		// down/up
		if (start.y < goal.y) {
			for (int y = start.y; y <= goal.y; y++) {
				destruct(y, start.x);
				if (!field.is_broken[y][start.x]) { return Pos{y, start.x}; }
			}
		} else {
			for (int y = start.y; y >= goal.y; y--) {
				destruct(y, start.x);
				if (!field.is_broken[y][start.x]) { return Pos{y, start.x}; }
			}
		}
		cout << "# y_move done" << endl;
		return Pos{goal.y, start.x};	// y_dir move done
	}
	Pos x_move(const Pos& start, const Pos& goal) {
		// right/left
		if (start.x < goal.x) {
			for (int x = start.x; x <= goal.x; x++) {
				destruct(start.y, x);	// ここstart.yにしないとまずい
				if (!field.is_broken[start.y][x]) { return Pos{start.y, x}; }
			}
		} else {
			for (int x = start.x; x >= goal.x; x--) {
				destruct(start.y, x);
				if (!field.is_broken[start.y][x]) { return Pos{start.y, x}; }
			}
		}
		cout << "# x_move done" << endl;
		return Pos{start.y, goal.x};	// x_dir move done
	}

	void repeat_partial_move(const Pos& start, const Pos& goal) {
		// you can output comment
		cout << "# move from (" << start.y << "," << start.x << ") to (" << goal.y << "," << goal.x << ")" << endl;
		queue<PosDir> que;
		que.push(PosDir{start, Y_DIR});	// y方向から先に探索
		que.push(PosDir{start, X_DIR});	// そのあとx
		// ゴールが掘削されるまで実行
		while (1) {	// check source to house
			PosDir popped = que.front(); que.pop();
			if (popped.dir == Y_DIR){
				Pos result_pos = y_move(popped.pos, goal);
				if (result_pos.y != goal.y) {								// 制限に引っ掛かり掘削できなかった場合
					que.push(PosDir{result_pos, Y_DIR});				// 制限に引っ掛かったため, y方向は後回し
				} else {															// y方向はゴールに到達
					result_pos = x_move(result_pos, goal);
					if (result_pos.y == goal.y && result_pos.x == goal.x) break;
					que.push(PosDir{result_pos, X_DIR});
				}
			} else if (popped.dir == X_DIR) {
				Pos result_pos = x_move(popped.pos, goal);
				if (result_pos.x != goal.x) {
					que.push(PosDir{result_pos, X_DIR});
				} else {
					result_pos = y_move(result_pos, goal);
					if (result_pos.y == goal.y && result_pos.x == goal.x) break; // ゴールに到達したか確認
					que.push(PosDir{result_pos, Y_DIR});
				}
			}
			//cout << "# aaa" << endl;
		}
	}

	void move(const Pos& start, const Pos& goal) {	// オリジナルの関数
		// you can output comment
		cout << "# move from (" << start.y << "," << start.x << ") to (" << goal.y << "," << goal.x << ")" << endl;
		// down/up
		y_move(start, goal); // 多分上手く動かない
		// right/left
		x_move(start, goal); 
	}

	void destruct(int y, int x) {
		// excavate (y, x) with fixed power until destruction
		int count = 0;
		while (!field.is_broken[y][x] && count < EXCAVATION_LIMIT) {
			Response result = field.query(y, x, POWER);
			if (result == Response::finish) {
				//cerr << "total_cost = " << field.total_cost << endl;
				exit(0);
			} else if (result == Response::invalid) {
				cerr << "invalid: y=" << y << " x=" << x << endl;
				exit(1);
			}
			count++;
		}
	}

	void excavation_source() {
      for (Pos source : source_pos) {
         move(source, source);
			connected_to_source[source.y][source.x] = 1;
      }
	}
};

int main() {
   
#if !CONTEST
   ifstream in("debug/tester/inst/0000.txt");
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
