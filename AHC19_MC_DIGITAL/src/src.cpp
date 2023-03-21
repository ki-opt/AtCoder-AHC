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
#define SILT_1 0
#define SILT_2 1
#define FRONT 0
#define RIGHT 1

using namespace std;

int MAX_ID = 1;

class Silhouette {
public:
	vector<vector<int>> zx_zy;
};

class Pos {
public:
	int x, y, z;
};

class Block {
public:
	vector<Pos> pos;				// blockのpos
	int block_id;					// blockのid
	int pair_use = -1;			// pairで使っているblock
	int index_pair_use = -1;	// pairで使っているblockのindex
};

class Solver {
public:
	int D;
	vector<vector<Block>> block;
	vector<vector<vector<vector<int>>>> block_xyz;
	vector<vector<Silhouette>> silt, covered_silt;
	Solver(vector<vector<Silhouette>> silt, vector<vector<Silhouette>> covered_silt, int D) : 
		silt(silt), covered_silt(silt), D(D) {
		block.resize(N_SILHOUETTE);
		block_xyz.resize(N_SILHOUETTE);
		for (int i = 0; i < N_SILHOUETTE; i++) {
			block_xyz[i].resize(D);
				for (int x = 0; x < D; x++) {
					block_xyz[i][x].resize(D);
					for (int y = 0; y < D; y++) { 
						block_xyz[i][x][y].resize(D);
						for (int z = 0; z < D; z++) {
							block_xyz[i][x][y][z] = 0;
						}
					}
				}
		}
	}
	
	int solve() {
		place_two_block();
		one_by_one_peace();	// 1×1のブロックを設置
		//refine_block_id();	// 多分バグ
		int ret = legal_check();
		return ret;
	}

	void add_block(int silt_id, const vector<Pos> &add_pos) {
		// blockの追加, block_xyzの更新
		block[silt_id].push_back(Block{});
		for (int i = 0; i < add_pos.size(); i++) {
			block[silt_id][block[silt_id].size()-1].pos.push_back(add_pos[i]);
			block_xyz[silt_id][add_pos[i].x][add_pos[i].y][add_pos[i].z] = MAX_ID;
			// covered_siltの更新
			covered_silt[silt_id][FRONT].zx_zy[add_pos[i].z][add_pos[i].x] = 0;
			covered_silt[silt_id][RIGHT].zx_zy[add_pos[i].z][add_pos[i].y] = 0;
		}
		block[silt_id][block[silt_id].size()-1].block_id = MAX_ID;
		// block_idの加算
		MAX_ID++;
	}
	
	int reuse_block(int silt_id, int block_size) {
		if (block_size <= 2) {
			// 縦棒の調査
			for (int y = 0; y < D; y++) {
				for (int z = 0; z < D - 1; z++) {
					if (covered_silt[silt_id][RIGHT].zx_zy[z][y] == 0) continue;		// 既にカバー済み
					// z方向調査 -> right
					if (covered_silt[silt_id][RIGHT].zx_zy[z+1][y] == 0) continue;
					// frontの調査
					int x;
					for (x = 0; x < D; x++) {
						if (covered_silt[silt_id][FRONT].zx_zy[z][x] == 1 && covered_silt[silt_id][FRONT].zx_zy[z+1][x] == 1) {
							break;
						}
					}
					if (x < D) {
						vector<Pos> add_pos{ Pos{x,y,z}, Pos{x,y,z+1} };
						add_block(silt_id, add_pos);
						return 1;
					}
				}
			}
			// 横棒（回転含む）の調査 -> y方向 
			for (int z = 0; z < D; z++) {
				for (int y = 0; y < D - 1; y++) {
					if (covered_silt[silt_id][RIGHT].zx_zy[z][y] == 0 || covered_silt[silt_id][RIGHT].zx_zy[z][y+1] == 0) continue;
					int x;
					for (x = 0; x < D; x++) {
						if (covered_silt[silt_id][FRONT].zx_zy[z][x] == 1) {
							break;
						}
					}
					if (x < D) {
						vector<Pos> add_pos{ Pos{x,y,z}, Pos{x,y+1,z} };
						add_block(silt_id, add_pos);
						return 1;
					}
				}
			}
			// 横棒の調査 -> x方向
			for (int z = 0; z < D; z++) {
				for (int x = 0; x < D - 1; x++) {
					if (covered_silt[silt_id][FRONT].zx_zy[z][x] == 0 || covered_silt[silt_id][FRONT].zx_zy[z][x+1] == 0) continue;
					int y;
					for (y = 0; y < D; y++) {
						if (covered_silt[silt_id][RIGHT].zx_zy[z][y] == 1) {
							break;
						}
					}
					if (y < D) {
						vector<Pos> add_pos{ Pos{x,y,z}, Pos{x+1,y,z} };
						add_block(silt_id, add_pos);
						return 1;
					}
				}
			}
		} else if (block_size > 2) {
			cout << "not implemented"; getchar();
		}
		return 0;
	}

	void place_two_block() {
		// シルエット1の調査 -> 縦棒
		for (int y_1 = 0; y_1 < D; y_1++) {
			for (int z_1 = 0; z_1 < D - 1; z_1++) {
				if (covered_silt[SILT_1][RIGHT].zx_zy[z_1][y_1] == 0) continue;	// すでにカバー済み
				// z方向調査 -> rightの調査
				if (covered_silt[SILT_1][RIGHT].zx_zy[z_1+1][y_1] == 0) continue;	// 縦棒がおけない(2by1)
				// [[[front]]]の縦方向の調査
				int x_1;
				for (x_1 = 0; x_1 < D; x_1++) {
					if (covered_silt[SILT_1][FRONT].zx_zy[z_1][x_1] == 1 && covered_silt[SILT_1][FRONT].zx_zy[z_1+1][x_1] == 1) {
						break;
					}
				}
				if (x_1 < D) {	// 該当シルエットを発見
					vector<Pos> add_pos{ Pos{x_1,y_1,z_1}, Pos{x_1,y_1,z_1+1} };
					if (reuse_block(SILT_2, add_pos.size()) == 1) {		// reuse可能か調査->SILT_2
						add_block(SILT_1, add_pos);							// blockなどの更新
						z_1++;
					}
				}
			}
		}
		// 横棒（y方向）
		for (int z = 0; z < D; z++) {
			for (int y = 0; y < D - 1; y++) {
				if (covered_silt[SILT_1][RIGHT].zx_zy[z][y] == 0 || covered_silt[SILT_1][RIGHT].zx_zy[z][y+1] == 0) continue;
				int x;
				for (x = 0; x < D; x++) {
					if (covered_silt[SILT_1][FRONT].zx_zy[z][x] == 1) {
						break;
					}
				}
				if (x < D) {
					vector<Pos> add_pos{ Pos{x,y,z}, Pos{x,y+1,z} };
					if (reuse_block(SILT_2, add_pos.size()) == 1) {
						add_block(SILT_1, add_pos);
						y++;
					}
				}
			}
		}
		// 横棒（x方向）
		for (int z = 0; z < D; z++) {
			for (int x = 0; x < D - 1; x++) {
				if (covered_silt[SILT_1][FRONT].zx_zy[z][x] == 0 || covered_silt[SILT_1][FRONT].zx_zy[z][x+1] == 0) continue;
				int y;
				for (y = 0; y < D; y++) {
					if (covered_silt[SILT_1][RIGHT].zx_zy[z][y] == 1) {
						break;
					}
				}
				if (y < D) {
					vector<Pos> add_pos{ Pos{x,y,z}, Pos{x+1,y,z} };
					if (reuse_block(SILT_2, add_pos.size()) == 1) {
						add_block(SILT_1, add_pos);
						x++;
					}
				}
			}
		}
		// 再帰関数的に探した方が良さそう。。
	}

	void one_by_one_peace() {
		for (int i = 0; i < N_SILHOUETTE; i++) {
			for (int z = 0; z < D; z++) {
				for (int x = 0; x < D; x++) {		// x:=front f(z,x), y:=right r(z,y)
					if (silt[i][FRONT].zx_zy[z][x] == 0) continue;
					for (int y = 0; y < D; y++) {
						if (silt[i][RIGHT].zx_zy[z][y] == 0) continue;
						if (!(covered_silt[i][FRONT].zx_zy[z][x] == 1 || covered_silt[i][RIGHT].zx_zy[z][y] == 1)) continue;	// シルエットカバー済み
						// blockの追加
						block[i].push_back(Block{});
						block[i][block[i].size()-1].pos.push_back(Pos{x,y,z});
						block[i][block[i].size()-1].block_id = MAX_ID;
						// block_xyzの更新
						block_xyz[i][x][y][z] = MAX_ID;
						covered_silt[i][FRONT].zx_zy[z][x] = 0; covered_silt[i][RIGHT].zx_zy[z][y] = 0;	// シルエットカバー済み
						// block_idの加算
						MAX_ID++;
					}
				}
			}
		}
		//MAX_ID = block_id - 1;
	}

	long double evaluation() {
		int value = 0;
		double wrk_value = 0;
		for (int i = 0; i < N_SILHOUETTE; i++) {
			for (int j = 0; j < block[i].size(); j++) {
				int cor_flag = 0;
				for (int k = 0; k < block[1-i].size(); k++) {
					if (block[i][j].block_id == block[1-i][k].block_id) { cor_flag = 1; break; }
				}
				if (cor_flag == 0) value += block[i][j].pos.size();
				else wrk_value += (1.0 / (double)block[i][j].pos.size());
			}
		}
		return round((double)pow(10, 9) * ((double)value + wrk_value));
	}

	int legal_check() {
		int ret = 0;
		for (int i = 0; i < N_SILHOUETTE; i++) {
			for (int z = 0; z < D; z++) {
				for (int x = 0; x < D; x++) {
					if (silt[i][FRONT].zx_zy[z][x] == 0) continue;
					int valid_flag = 0;
					for (int y = 0; y < D; y++) {
						if (block_xyz[i][x][y][z] >= 1) { valid_flag = 1; break; }
					}
					if (valid_flag == 0) goto INVALID;
				}
				for (int y = 0; y < D; y++) {
					if (silt[i][RIGHT].zx_zy[z][y] == 0) continue;
					int valid_flag = 0;
					for (int x = 0; x < D; x++) {
						if (block_xyz[i][x][y][z] >= 1) { valid_flag = 1; break; }
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

		fprintf(fpOutput, "%d\n", MAX_ID - 1);
		// 出力
		for (int i = 0; i < N_SILHOUETTE; i++) {
			for (int x = 0; x < D; x++) {
				for (int y = 0; y < D; y++) {
					for (int z = 0; z < D; z++) {
						fprintf(fpOutput, "%d ", block_xyz[i][x][y][z]);
					}
				}
			}
			fprintf(fpOutput, "\n");
		}
		fclose(fpOutput);
	}

	void output_score(string output_filename, long double score) {
		FILE* fpScore;
		fpScore = fopen(output_filename.c_str(), "w");
		if (fpScore == NULL) { cout << "cannot write output file"; getchar(); }

		fprintf(fpScore, "%.0LF\n", score);
		fclose(fpScore);
	}

	void print_block() {
		cout << MAX_ID - 1 << endl;
		for (int i = 0; i < N_SILHOUETTE; i++) {
			vector<vector<vector<int>>> result(D, vector<vector<int>>(D, vector<int>(D, 0)));
			for (int iter = 0; iter < block[i].size(); iter++) {
				for (int j = 0; j < block[i][iter].pos.size(); j++) {
					result[block[i][iter].pos[j].x][block[i][iter].pos[j].y][block[i][iter].pos[j].z] = (i * block[0].size()) + iter + 1;
				}
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
	string filename, output_filename, score_filename, w_filename;
	if (argc >= 2) {
		w_filename = argv[1];
		filename = "tester/inst/" + w_filename;
		output_filename = "tester/out/" + w_filename;
		score_filename = "tester/score/" + w_filename;
	} else {
		w_filename = "0097.txt";
		filename = "debug/tester/inst/" + w_filename;
		output_filename = "debug/tester/out/" + w_filename;
		score_filename = "debug/tester/score/" + w_filename;
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
			silt[i][f_r].zx_zy.resize(D);
			for (int z = 0; z < D; z++) {
				string data;
				cin >> data;
				bitset<16> data_bit(data);
				//cout << data << "\n";
				//silt[i][f_r].zx_zy[d] = data;
				//cout << data_bit << ": ";
				for (int d_2 = 0; d_2 < D; d_2++) {
					silt[i][f_r].zx_zy[z].push_back(data_bit.test((D - 1) - d_2));
					//cout << data_bit.test((D - 1) - d_2) << " ";
				}
				//cout << "\n";
			}
		}
	}

	Solver solver(silt, silt, D);
	int ret = solver.solve();
#if !CONTEST
	//cout << ret << "\n";
	if (ret == 1) {
		long double score = solver.evaluation();
		cout << "seed: " << w_filename << "  " << "legal" << ": " << score << "\n";
		//cerr << "legal" << ": " << solver.evaluation() << "\n";
		solver.output_result_file(output_filename);
		solver.output_score(score_filename, score);
		//solver.print_block();
	}
#else
	solver.print_block();
#endif
}
