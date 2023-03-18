#include <iostream>
#include <vector>
#include <fstream>
#include <queue>

#define CONTEST false

using namespace std;

class Package {
public:
	int height, width, depth, lot, rot_flag, stack_flag;
};
class BL {
public:
	int h, w, d;
};

class Solver {
public:
	vector<Package> package;
	int M, W, H, B, D;
	Solver(vector<Package> package, int M, int W, int H, int B, int D) : 
		package(package), M(M), W(W), H(H), B(B), D(D) {}

	void solve() {
		sequential_packing(); // (g=0を除き)順番に置いていく, 回転もなし
	}

	void sequential_packing() {
		queue<Package> stack_que;
		queue<Package> remained_que;
		// 上積み可能な荷物stack_que, 上積み不可能な荷物remained_que
		for (int i = 0; i < M; i++) {
			if (package[i].stack_flag == 1) {
				for (int j = 0; j < package[i].lot; j++) stack_que.push(package[i]);
			} else {
				for (int j = 0; j < package[i].lot; j++) remained_que.push(package[i]);
			}
		}
		// stack_queにまとめる
		while (!remained_que.empty()) {
			stack_que.push(remained_que.front());
			remained_que.pop();
		}
		// blポイントの作成
		vector<BL> bl;
		add_initial_bl(bl);
		// stack_queから取り出し
		while (!stack_que.empty()) {
			Package que = stack_que.front(); stack_que.pop(); 
			for (int i = 0; i < bl.size(); i++) {
				
			}
		}
		getchar();
	}

	void add_initial_bl(vector<BL> &bl) {
		bl.push_back(BL{0,B,0});
		bl.push_back(BL{B,0,0});
	}

	void put_package() {

	}
};

int main() {
   
#if !CONTEST
   ifstream in("tools/in/0000.txt");
   if (!in) { cout << "error"; getchar(); }
   cin.rdbuf(in.rdbuf());
#endif

	int M, W, H, B, D;
	cin >> M >> W >> H >> B >> D;

	vector<Package> package(M);
	for (int i = 0; i < M; i++) {
		cin >> package[i].height >> package[i].width >> package[i].depth >> package[i].lot;
		string rotate_flag, stack_flag;
		cin >> rotate_flag >> stack_flag;
		if (rotate_flag == "Y") package[i].rot_flag = 1;
		else package[i].rot_flag = 0;
		if (stack_flag == "Y") package[i].stack_flag = 1;
		else package[i].stack_flag = 0;
	}

	Solver solver(package, M, W, H, B, D);
	solver.solve();
}
