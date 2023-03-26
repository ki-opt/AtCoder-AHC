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
#include <random>
#include <algorithm>

#define CONTEST 0 // true->コンテスト

#define N_SILHOUETTE 2
#define SILT_1 0
#define SILT_2 1
#define FRONT 0
#define RIGHT 1
#define X 0
#define Y 1
#define Z 2
#define N_ROTS 4
#define ROTATION_RAD 3.141592653589793 / 2.0
#define SEED 0

// パラメータ
#define N_ARROWED_BLKS 3

using namespace std;

int MAX_ID = 1;

class RandomNumber {
public:
	mt19937 mt;
	RandomNumber(int seed) : mt(seed) {} 
	int range(int min, int max) {
		uniform_int_distribution<> ransu(min,max); // [min,max)の乱数
		return ransu(mt);
	}
	int ransu32bit() {
		return mt();
	}
	void array_shuffle(vector<int> &vec) {
		shuffle(vec.begin(), vec.end(), mt);
	}
};

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
	int pair_flag;					// pairで使っているブロック
};

class AddedBlock {
public:
	Pos start_pos;
	vector<int> dir;
};

class Solver {
public:
	int START_BLOCK_VOLUME = 2;
	int D;
	vector<vector<Block>> block;
	vector<vector<vector<vector<int>>>> block_xyz;
	vector<vector<Silhouette>> silt, covered_silt;
	RandomNumber random_number;
	vector<vector<vector<vector<vector<Pos>>>>> overlap;
	Solver(vector<vector<Silhouette>> silt, vector<vector<Silhouette>> covered_silt, int D, RandomNumber random_number,
			vector<vector<vector<vector<vector<Pos>>>>> overlap) : 
			silt(silt), covered_silt(silt), D(D), random_number(random_number), overlap(overlap) {
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
		place_block();
		place_two_block();
		one_by_one_peace();	// 1×1のブロックを設置
		int ret = legal_check();
		return ret;
	}

/* place block */
	/*	int check(const int &silt_id, const int &right_front, const int &dir, const int &x, const int &y, const int &z) {
		if (dir == X) {
			return (silt[silt_id][right_front].zx_zy[z][y] == 1 &&
				silt[silt_id][1-right_front].zx_zy[z][x] == 1 && silt[silt_id][1-right_front].zx_zy[z][x+1] == 1 &&
				block_xyz[silt_id][x][y][z] == 0 && block_xyz[silt_id][x+1][y][z] == 0);
		} else if (dir == Y) {
			return (silt[silt_id][right_front].zx_zy[z][y] == 1 && silt[silt_id][right_front].zx_zy[z][y+1] == 1 &&
				silt[silt_id][1-right_front].zx_zy[z][x] == 1 &&
				block_xyz[silt_id][x][y][z] == 0 && block_xyz[silt_id][x][y+1][z] == 0);
		} else if (dir == Z) {
			return (silt[silt_id][right_front].zx_zy[z][y] == 1 && silt[silt_id][right_front].zx_zy[z+1][y] == 1 &&
				silt[silt_id][1-right_front].zx_zy[z][x] == 1 && silt[silt_id][1-right_front].zx_zy[z+1][x] == 1 &&
				block_xyz[silt_id][x][y][z] == 0 && block_xyz[silt_id][x][y][z+1] == 0);
		} else { cout << "error"; getchar(); return 0; }
	}
*/
	
	vector<Pos> rotation (const vector<Pos> &added_block, double x_rad, double y_rad, double z_rad) {
		vector<double> rad = {x_rad, y_rad, z_rad};
		vector<Pos> after_rot(added_block.size());
		Pos value = Pos{INT_MAX,INT_MAX,INT_MAX};
		//Pos value;
		for (int i = 0; i < added_block.size(); i++) {
			// x軸回転
			after_rot[i].x = added_block[i].x;
			after_rot[i].y = round(cos(rad[X]))*added_block[i].y + (-round(sin(rad[X]))*added_block[i].z);
			after_rot[i].z = round(sin(rad[X]))*added_block[i].y + round(cos(rad[X]))*added_block[i].z;
			// y軸回転
			Pos wrk = after_rot[i];
			after_rot[i].x = round(cos(rad[Y]))*wrk.x + round(sin(rad[Y]))*wrk.z;
			after_rot[i].y = wrk.y;
			after_rot[i].z = (-round(sin(rad[Y]))*wrk.x) + round(cos(rad[Y]))*wrk.z;
			// z軸回転
			wrk = after_rot[i];
			after_rot[i].x = round(cos(rad[Z]))*wrk.x + (-round(sin(rad[Z]))*wrk.y);
			after_rot[i].y = round(sin(rad[Z]))*wrk.x + round(cos(rad[Z]))*wrk.y;
			after_rot[i].z = wrk.z;
			// min value
			//*
			if (after_rot[i].x < value.x) value.x = after_rot[i].x;
			if (after_rot[i].y < value.y) value.y = after_rot[i].y;
			if (after_rot[i].z < value.z) value.z = after_rot[i].z;//*/			
		}
		for (int i = 0; i < added_block.size(); i++) {
			after_rot[i].x -= value.x;
			after_rot[i].y -= value.y;
			after_rot[i].z -= value.z;
		}//*
		return after_rot;
	}

	int check_other_silt(int other_silt_id, const vector<Pos> &rotate_added_block, vector<Pos> &other_added_block) {
		for (int x=0; x<D; x++) for (int y=0; y<D; y++) for (int z=0; z<D; z++) {
			vector<Pos> matched_block;
			for (int i = 0; i < rotate_added_block.size(); i++) {
				Pos cur_pos = Pos{rotate_added_block[i].x + x, rotate_added_block[i].y + y, rotate_added_block[i].z + z};
				if (cur_pos.x < 0 || cur_pos.x >= D ||	cur_pos.y < 0 || cur_pos.y >= D || cur_pos.z < 0 || cur_pos.z >= D) { 
					break; 
				}
				if (silt[other_silt_id][FRONT].zx_zy[cur_pos.z][cur_pos.x] == 1 && silt[other_silt_id][RIGHT].zx_zy[cur_pos.z][cur_pos.y] == 1 &&
						block_xyz[other_silt_id][cur_pos.x][cur_pos.y][cur_pos.z] == 0) {
					matched_block.push_back(cur_pos);
					if (i + 1 == rotate_added_block.size()) {
						other_added_block = matched_block;
						return 1;
					}
				} else {
					break;
				}
			}
		}		
		return 0;	// blockなし
	}

	void place_block() {
		// アルゴリズム
		// 1.シルエットの重なり部分からスタート(ランダム可->x,y,z)
		vector<Pos> start_xyz_pos(D*D*D); vector<int> start_xyz_pos_index(D*D*D);
		for (int x=0; x<D; x++) for (int y=0; y<D; y++) for (int z=0; z<D; z++) {
			start_xyz_pos[x*D*D+y*D+z] = Pos{x,y,z};
			start_xyz_pos_index[x*D*D+y*D+z] = x*D*D+y*D+z;
		}
		random_number.array_shuffle(start_xyz_pos_index);
		for (int index = 0; index < D*D*D; index++) {
		//for (int x=0; x<D; x++) for (int y=0; y<D; y++) for (int z=0; z<D; z++) {	// ランダムではないver.
			int x = start_xyz_pos[start_xyz_pos_index[index]].x, y = start_xyz_pos[start_xyz_pos_index[index]].y, z = start_xyz_pos[start_xyz_pos_index[index]].z;
			// 1-1.silt_idとright_frontの決定（silt_idとright_frontをランダムに決定しても良い）
			int silt_id = SILT_1;	//int right_front = RIGHT;
			int other_silt_id = 1 - silt_id;
			if (overlap[silt_id][x][y][z].size() == 0 || block_xyz[silt_id][x][y][z] > 0) continue; //x,y,zにおけるか調査
			Pos cur_pos = Pos{x,y,z};
			vector<Pos> added_block = {Pos{x,y,z}};	// silt_idにaddするblock
			vector<Pos> other_added_block;				// other_silt_idにaddするblock
			vector<vector<vector<int>>> fake_block(D,vector<vector<int>>(D,vector<int>(D,0)));
			fake_block[cur_pos.x][cur_pos.y][cur_pos.z] = 1;	// fake_block
			// 2.進行方向を決める
			// ovelap_indexの作成
			vector<int> overlap_index(overlap[silt_id][cur_pos.x][cur_pos.y][cur_pos.z].size());
			for (int i = 0; i < overlap_index.size(); i++) overlap_index[i] = i;
			random_number.array_shuffle(overlap_index);
			for (int cnt = 0; cnt < overlap_index.size(); cnt++) {
				int i = overlap_index[cnt];
				// 3.ブロックの重なりがないかチェック
				Pos next_pos = overlap[silt_id][cur_pos.x][cur_pos.y][cur_pos.z][i];
				if (fake_block[next_pos.x][next_pos.y][next_pos.z] == 1 || block_xyz[silt_id][next_pos.x][next_pos.y][next_pos.z] > 0) {
					continue;
				}
				added_block.push_back(next_pos);
				// 4.→ 同じ形状のブロックをもう一方におけるかチェック(ここは乱数必要ない)
				// 回転
				int find_flag = 0;
				for (int x_rad=0; x_rad<N_ROTS; x_rad++) {
					for (int y_rad=0; y_rad<N_ROTS; y_rad++) {
						for (int z_rad=0; z_rad<N_ROTS; z_rad++) {
							vector<Pos> rotate_added_block = rotation(added_block, ROTATION_RAD*x_rad, ROTATION_RAD*y_rad, ROTATION_RAD*z_rad);
							if (check_other_silt(other_silt_id, rotate_added_block, other_added_block) == 1) {	// もう一方にブロックを置けるか調査
								find_flag = 1;
								break;	// true->put block
							}
						}
						if (find_flag == 1) break;
					}
					if (find_flag == 1) break;
				}
				if (find_flag == 1) {
					// 次のブロックに進む
					fake_block[next_pos.x][next_pos.y][next_pos.z] = 1;
					cur_pos = next_pos;
					cnt = -1;
					// ovelap_indexの修正
					overlap_index.resize(overlap[silt_id][cur_pos.x][cur_pos.y][cur_pos.z].size());
					for (int i = 0; i < overlap_index.size(); i++) overlap_index[i] = i;
					random_number.array_shuffle(overlap_index);
				} else {
					// 最後のブロックを削除
					added_block.pop_back();
				}
			}
			if (added_block.size() >= N_ARROWED_BLKS) {
				// silt_idにblockを追加
				add_block(silt_id, added_block);
				// other_siltにblockを追加
				add_block(other_silt_id, other_added_block);
				MAX_ID++;
			}
			// 5.→ →置ければ2に戻る
			// 6.→ →置けなければそれまでのブロックを追加し、1に戻る		
		}
		/*
		for (int x = 0; x < D; x++) {
			for (int y = 0; y < D; y++) {
				for (int z = 0; z < D; z++) {
					// 縦棒が置けるか判定 -> z方向
					// ここ乱数でXYZの3択にしてもいいかもね
					//if (z + 1 < D) {
					vector<Pos> added_block = { Pos{x,y,z}, Pos{x,y,z+1} };
					if (check(silt_id, right_front, Z, x, y, z)) {	// シルエットにかかり、ブロックが置かれていない場合
						// 
						while();
						// 3次元で回転（回転なし+90°+180°+270°の4つ
						for (int rot_x = 1; rot_x < N_ROTS; rot_x++) for (int rot_y = 0; rot_y < N_ROTS; rot_y++) for (int rot_z = 0; rot_z < N_ROTS; rot_z++) {
							vector<Pos> rotate_block = rotation(added_block, ROTATION_RAD*rot_x, ROTATION_RAD*rot_y, ROTATION_RAD*rot_z);
						}
					}
					//}
					// 後で修正 if (x + 1 < D) {}
					if (false) {
						//DFS(Pos{x,y,z}, START_BLOCK_VOLUME, Y);
						//DFS(Pos{x,y,z}, START_BLOCK_VOLUME, Z);
					}
				}
			}
		}*/
	}
/* place block */

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
	}
	
	int reuse_block(int silt_id, int block_size, int overlap_flag) {
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
			if (overlap_flag == 1) {
				// 横棒（回転含む）の調査 -> y方向 -> 重なりがあっても配置 
				for (int z = 0; z < D; z++) {
					for (int y = 0; y < D - 1; y++) {
						//if (covered_silt[silt_id][RIGHT].zx_zy[z][y] == 0 || covered_silt[silt_id][RIGHT].zx_zy[z][y+1] == 0) continue;
						if (!(silt[silt_id][RIGHT].zx_zy[z][y] == 1 && silt[silt_id][RIGHT].zx_zy[z][y+1] == 1)) continue;
						int x;
						for (x = 0; x < D; x++) {
							//if (covered_silt[silt_id][FRONT].zx_zy[z][x] == 1) {
							if (silt[silt_id][FRONT].zx_zy[z][x] == 1 && (block_xyz[silt_id][x][y][z] == 0 && block_xyz[silt_id][x][y+1][z] == 0)) {
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
				// 横棒の調査 -> x方向 -> 重なりがあっても配置
				for (int z = 0; z < D; z++) {
					for (int x = 0; x < D - 1; x++) {
						//if (covered_silt[silt_id][FRONT].zx_zy[z][x] == 0 || covered_silt[silt_id][FRONT].zx_zy[z][x+1] == 0) continue;
						if (!(silt[silt_id][FRONT].zx_zy[z][x] == 1 && silt[silt_id][FRONT].zx_zy[z][x+1] == 1)) continue;
						int y;
						for (y = 0; y < D; y++) {
							//if (covered_silt[silt_id][RIGHT].zx_zy[z][y] == 1) {
							if (silt[silt_id][RIGHT].zx_zy[z][y] == 1 && (block_xyz[silt_id][x][y][z] == 0 && block_xyz[silt_id][x+1][y][z] == 0)) {
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
					if (reuse_block(SILT_2, add_pos.size(), 0) == 1) {		// reuse可能か調査->SILT_2
						add_block(SILT_1, add_pos);							// blockなどの更新
						// block_idの加算
						MAX_ID++;
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
					if (reuse_block(SILT_2, add_pos.size(), 0) == 1) {
						add_block(SILT_1, add_pos);
						// block_idの加算
						MAX_ID++;
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
					if (reuse_block(SILT_2, add_pos.size(), 0) == 1) {
						add_block(SILT_1, add_pos);
						// block_idの加算
						MAX_ID++;
						x++;
					}
				}
			}
		}
		// 横棒（y方向）-> 重なりがあっても配置
		for (int z = 0; z < D; z++) {
			for (int y = 0; y < D - 1; y++) {
				if (silt[SILT_1][RIGHT].zx_zy[z][y] == 0 || silt[SILT_1][RIGHT].zx_zy[z][y+1] == 0) continue;
				int x;
				for (x = 0; x < D; x++) {
					if (silt[SILT_1][FRONT].zx_zy[z][x] == 1 && (block_xyz[SILT_1][x][y][z] == 0 && block_xyz[SILT_1][x][y+1][z] == 0)) {
						break;
					}
				}
				if (x < D) {
					vector<Pos> add_pos{ Pos{x,y,z}, Pos{x,y+1,z} };
					if (reuse_block(SILT_2, add_pos.size(), 1) == 1) {
						add_block(SILT_1, add_pos);
						// block_idの加算
						MAX_ID++;
						y++;
					}
				}
			}
		}
		// 横棒（x方向）-> 重なりがあっても配置
		for (int z = 0; z < D; z++) {
			for (int x = 0; x < D - 1; x++) {
				if (silt[SILT_1][FRONT].zx_zy[z][x] == 0 || silt[SILT_1][FRONT].zx_zy[z][x+1] == 0) continue;
				int y;
				for (y = 0; y < D; y++) {
					if (silt[SILT_1][RIGHT].zx_zy[z][y] == 1 && (block_xyz[SILT_1][x][y][z] == 0 && block_xyz[SILT_1][x+1][y][z] == 0)) {
						break;
					}
				}
				if (y < D) {
					vector<Pos> add_pos{ Pos{x,y,z}, Pos{x+1,y,z} };
					if (reuse_block(SILT_2, add_pos.size(), 1) == 1) {
						add_block(SILT_1, add_pos);
						// block_idの加算
						MAX_ID++;
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
						if (i == SILT_1) {	// silt_idが0の場合, blockを再利用
							for (int z_2 = 0; z_2 < D; z_2++) {
								for (int x_2 = 0; x_2 < D; x_2++) {
									if (silt[SILT_2][FRONT].zx_zy[z_2][x_2] == 0) continue;
									for (int y_2 = 0; y_2 < D; y_2++) {
										if (silt[SILT_2][RIGHT].zx_zy[z_2][y_2] == 0) continue;
										if (!(covered_silt[SILT_2][FRONT].zx_zy[z_2][x_2] == 1 || covered_silt[SILT_2][RIGHT].zx_zy[z_2][y_2] == 1)) continue;
										block[SILT_2].push_back(Block{});
										block[SILT_2][block[SILT_2].size()-1].pos.push_back(Pos{x_2,y_2,z_2});
										block[SILT_2][block[SILT_2].size()-1].block_id = MAX_ID;
										block_xyz[SILT_2][x_2][y_2][z_2] = MAX_ID;
										covered_silt[SILT_2][FRONT].zx_zy[z_2][x_2] = 0; covered_silt[SILT_2][RIGHT].zx_zy[z_2][y_2] = 0;
										goto GOTO;
									}
								}
							}
						}
GOTO:
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
				for (int k = 0; k < block[1-i].size(); k++) {
					if (block[i][j].block_id == block[1-i][k].block_id) { 
						block[i][j].pair_flag = 1; 
						break;
					}
				}
				if (block[i][j].pair_flag == 0) {
					value += block[i][j].pos.size();
				}
			}
		}
		for (int j = 0; j < block[SILT_1].size(); j++) {				
			if (block[SILT_1][j].pair_flag == 1) {
				wrk_value += (1.0 / (double)block[SILT_1][j].pos.size());
			}
		}
		return round((double)pow(10, 9) * ((double)value + wrk_value));
	}

	int legal_check() {
		// block_sizeが一緒であることの言及が足りない
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
			for (int x = 0; x < D; x++) {
				for (int y = 0; y < D; y++) {
					for (int z = 0; z < D; z++) {
						cout << block_xyz[i][x][y][z] << " ";
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
		w_filename = "0000.txt";
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

	// overlapの計算
	vector<vector<vector<vector<vector<Pos>>>>> overlap(N_SILHOUETTE);
	for (int i = 0; i < N_SILHOUETTE; i++) {
		overlap[i].resize(D);
		for (int x = 0; x < D; x++) {
			overlap[i][x].resize(D);
			for (int y = 0; y < D; y++) {
				overlap[i][x][y].resize(D);
				for (int z = 0; z < D; z++) {
					if (silt[i][FRONT].zx_zy[z][x] == 1 && silt[i][RIGHT].zx_zy[z][y] == 1) { 
						if (z + 1 < D) if (silt[i][FRONT].zx_zy[z+1][x] == 1 && silt[i][RIGHT].zx_zy[z+1][y] == 1) overlap[i][x][y][z].push_back(Pos{x,y,z+1});
						if (z - 1 >= 0) if (silt[i][FRONT].zx_zy[z-1][x] == 1 && silt[i][RIGHT].zx_zy[z-1][y] == 1) overlap[i][x][y][z].push_back(Pos{x,y,z-1});
						if (x + 1 < D) if (silt[i][FRONT].zx_zy[z][x+1] == 1 && silt[i][RIGHT].zx_zy[z][y] == 1) overlap[i][x][y][z].push_back(Pos{x+1,y,z});
						if (x - 1 >= 0) if (silt[i][FRONT].zx_zy[z][x-1] == 1 && silt[i][RIGHT].zx_zy[z][y] == 1) overlap[i][x][y][z].push_back(Pos{x-1,y,z});
						if (y + 1 < D) if (silt[i][FRONT].zx_zy[z][x] == 1 && silt[i][RIGHT].zx_zy[z][y+1] == 1) overlap[i][x][y][z].push_back(Pos{x,y+1,z});
						if (y - 1 >= 0) if (silt[i][FRONT].zx_zy[z][x] == 1 && silt[i][RIGHT].zx_zy[z][y-1] == 1) overlap[i][x][y][z].push_back(Pos{x,y-1,z});
					}
				}
			}
		}
	}

#if !CONTEST
	int ret;
	for (int index=0;index<1;index++) {
		RandomNumber random_number(SEED+index);
		Solver solver(silt, silt, D, random_number, overlap);
		ret = solver.solve();
		if (ret == 1) {
			long double score = solver.evaluation();
			cout << "seed: " << w_filename << "  " << "legal" << ": " << score << "\n";
			//cerr << "legal" << ": " << solver.evaluation() << "\n";
			solver.output_result_file(output_filename);
			solver.output_score(score_filename, score);
			solver.print_block();
		}
	}
#else
	RandomNumber random_number(SEED);
	Solver solver(silt, silt, D, random_number, overlap);
	int ret = solver.solve();
	solver.print_block();
#endif
}
