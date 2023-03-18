#include <iostream>
#include <vector>
#include <fstream>
#include <climits>
#include <queue>

#define CONTEST 0 // true->コンテスト

using namespace std;

class Coordinate {
public:
	vector<vector<int>> pos;
};

class Solver {
public:

};

int main() {
   
#if !CONTEST
   ifstream in("debug/tester/inst/0000.txt");
   if (!in) { cout << "error"; getchar(); }
   cin.rdbuf(in.rdbuf());
#endif

	int D;
	cin >> D;

	vector<vector<Coordinate>> silhouette(2,vector<Coordinate>(2));
	for (int i = 0; i < 2; i++) {
		for (int f_r = 0; f_r < 2; f_r++) {
			for (int d_1 = 0; d_1 < D; d_1++) {
				for (int d_2 = 0; d_2 < D; d_2++) {
					int data;
					cin >> data;
					silhouette[i][f_r].pos[d_1].push_back(data);
				}
			}
		}
	}

	Solver solver();
	//solver.solve();
}
