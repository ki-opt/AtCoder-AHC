#include <iostream>
#include <vector>
#include <fstream>

#define CONTEST true

using namespace std;

class Package {
public:
	int height, width, depth, lot, rot_flag, stack_flag;
};

class Solver {
public:
	int M, W, H, B, D;
	Solver(int M, int W, int H, int B, int D) : M(M), W(W), H(H), B(B), D(D) {}

	void solve() {

	}
};

int main() {
   
#if !CONTEST
   ifstream in("debug/tester/inst/0000.txt");
   if (!in) { cout << "error"; getchar(); }
   cin.rdbuf(in.rdbuf());
#endif

	int M, W, H, B, D;
	cin >> M >> W >> H >> B >> D;

#if !CONTEST
   string line;
   for (int i = 0; i < N + 1; i++) {   // 最初の行の改行コードのため+1
      getline(cin, line);
   }
#endif

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

	Solver solver(M, W, H, B, D);
	//solver.solve();
}
