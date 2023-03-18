// z軸は上から下
// ブロックを構成する立方体は連結である必要があるが、ブロック同士は非連結でも構わない

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <climits>
#include <queue>
#include <bitset>
#include <cmath>

#define CONTEST 0 // true->コンテスト

#define N_SILHOUETTE 2
#define FRONT 0
#define RIGHT 1

using namespace std;

class Silhouette {
public:
	vector<vector<int>> pos;
};

class Block {
public:
	int x, y, z;
	int volume;
	int id;
};

class Solver {
public:
	int D;
	vector<vector<Block>> block;
	vector<vector<vector<int>>> block_xyz;
	vector<vector<Silhouette>> silt;
	Solver(vector<vector<Silhouette>> silt, int D) : silt(silt), D(D) {
		block.resize(N_SILHOUETTE);
		block_xyz.resize(D);
		for (int x = 0; x < D; x++) {
			block_xyz[x].resize(D);
			for (int y = 0; y < D; y++) { 
				block_xyz[x][y].resize(D);
				for (int z = 0; z < D; z++) {
					block_xyz[x][y][z] = 0;
				}
			}
		}
	}
	
	int solve() {
		one_by_one_peace();	// 1×1のブロックを設置
		int ret = legal_check();
		return ret;
	}

	void one_by_one_peace() {
		int block_id = 1;
		for (int i = 0; i < N_SILHOUETTE; i++) {
			for (int z = 0; z < D; z++) {
				for (int x = 0; x < D; x++) {		// x:=front f(z,x), y:=right r(z,y)
					if (silt[i][FRONT].pos[z][x] == 0) continue;
					for (int y = 0; y < D; y++) {
						if (silt[i][RIGHT].pos[z][y] == 0) continue;
						block[i].push_back(Block{x,y,z,1,block_id});
						block_xyz[x][y][z] = 1;
						block_id++;
					}
				}
			}
		}
	}

	long double evaluation() {
		int value = 0;
		for (int i = 0; i < N_SILHOUETTE; i++) {
			for (int j = 0; j < block[i].size(); j++) {
				int cor_flag = 0;
				for (int k = 0; k < block[1-i].size(); k++) {
					if (block[i][j].id == block[1-i][k].id) { cor_flag = 1; break; }
				}
				if (cor_flag == 0) value++;
			}
		}
		double wrk_value = 0;
		for (int i = 0; i < N_SILHOUETTE; i++) {
			for (int j = 0; j < block[i].size(); j++) {
				wrk_value += (1.0 / (double)block[i][j].volume);
			}
		}
		return round((double)pow(10, 9) * ((double)value + wrk_value));
	}

	int legal_check() {
		int ret = 0;
		for (int i = 0; i < N_SILHOUETTE; i++) {
			for (int z = 0; z < D; z++) {
				for (int x = 0; x < D; x++) {
					if (silt[i][FRONT].pos[z][x] == 0) continue;
					int valid_flag = 0;
					for (int y = 0; y < D; y++) {
						if (block_xyz[x][y][z] == 1) { valid_flag = 1; break; }
					}
					if (valid_flag == 0) goto INVALID;
				}
				for (int y = 0; y < D; y++) {
					if (silt[i][RIGHT].pos[z][y] == 0) continue;
					int valid_flag = 0;
					for (int x = 0; x < D; x++) {
						if (block_xyz[x][y][z] == 1) { valid_flag = 1; break; }
					}
					if (valid_flag == 0) goto INVALID;
				}
			}
		}
		ret = 1;
INVALID:
		return ret;
	}

	void output_result_file(string output_filename) {
		FILE* fpOutput;
		fpOutput = fopen(output_filename.c_str(), "w");
		if (fpOutput == NULL) { cout << "cannot write output file"; getchar(); }
		fprintf(fpOutput, "%d\n", block[0].size() + block[1].size());
		for (int i = 0; i < N_SILHOUETTE; i++) {
			vector<vector<vector<int>>> result(D, vector<vector<int>>(D, vector<int>(D, 0)));
			for (int iter = 0; iter < block[i].size(); iter++) {
				result[block[i][iter].x][block[i][iter].y][block[i][iter].z] = (i * block[0].size()) + iter + 1;
			}
			for (int x = 0; x < D; x++) {
				for (int y = 0; y < D; y++) {
					for (int z = 0; z < D; z++) {
						//cout << result[x][y][z] << " ";
						fprintf(fpOutput, "%d ", result[x][y][z]);
					}
				}
			}
			fprintf(fpOutput, "\n");
		}
		fclose(fpOutput);
	}

	void print_block() {
		cout << block[0].size() + block[1].size() << endl;
		for (int i = 0; i < N_SILHOUETTE; i++) {
			vector<vector<vector<int>>> result(D, vector<vector<int>>(D, vector<int>(D, 0)));
			for (int iter = 0; iter < block[i].size(); iter++) {
				//cout << block[i][iter].x << " " << block[i][iter].y << " " << block[i][iter].z << "\n";
				result[block[i][iter].x][block[i][iter].y][block[i][iter].z] = (i * block[0].size()) + iter + 1;
			}
			for (int x = 0; x < D; x++) {
				for (int y = 0; y < D; y++) {
					for (int z = 0; z < D; z++) {
						cout << result[x][y][z] << " ";
					}
				}
			}
			cout << endl;
		}
	}
};

int main(int argc, char* argv[]) {

#if !CONTEST
	string filename, output_filename;
	if (argc >= 2) {
		string w_filename = argv[1];
		filename = "tester/inst/" + w_filename;
		output_filename = "tester/out/" + w_filename;
	} else {
		filename = "debug/tester/inst/0000.txt";
		output_filename = "debug/tester/out/0000.txt";
	}
	ifstream in(filename);
	if (!in) { cout << "cannot read inst error"; getchar(); }
	cin.rdbuf(in.rdbuf());
#endif

	int D;
	cin >> D;

	vector<vector<Silhouette>> silt(N_SILHOUETTE,vector<Silhouette>(N_SILHOUETTE));
	for (int i = 0; i < N_SILHOUETTE; i++) {
		for (int f_r = 0; f_r < N_SILHOUETTE; f_r++) {
			silt[i][f_r].pos.resize(D);
			for (int z = 0; z < D; z++) {
				string data;
				cin >> data;
				bitset<16> data_bit(data);
				//cout << data << "\n";
				//silt[i][f_r].pos[d] = data;
				//cout << data_bit << ": ";
				for (int d_2 = 0; d_2 < D; d_2++) {
					silt[i][f_r].pos[z].push_back(data_bit.test((D - 1) - d_2));
					//cout << data_bit.test((D - 1) - d_2) << " ";
				}
				//cout << "\n";
			}
		}
	}

	Solver solver(silt, D);
	int ret = solver.solve();
#if !CONTEST
	//cout << ret << "\n";
	if (ret == 1) {
		cout << solver.evaluation() << "\n";
		solver.output_result_file(output_filename);
	}
#else
	solver.print_block();
#endif
}
