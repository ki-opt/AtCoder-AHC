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
#include <chrono>
#include <unordered_map>

#define CONTEST 0 // true->コンテスト
#define INSTANCE_NAME "0090.txt"

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
#define N_ARROWED_BLKS 3							// >=N_ARROWED_BLKSの場合は許可
#define N_ALNS_REPEATS 100000						// ALNSでのrepeat回数最大値
#define N_ERASED_BLOCK_ALNS 4						// ALNSで消すブロックの数
#define INC_ERASED_BLK_THRESHOLD	100			// ALNSで消すブロックを増やす閾値
#define N_FIRST_PLACE_BLOCK_LIMIT 150			// place_block()の最初だけで使用
#define N_PLACE_BLOCK_LIMIT 30					// D + N_PLACE_BLOCK_LIMIT -> place_block()で試用
#define UPPER_BLOCK_SIZE 30						// ブロックサイズを制限
#define MOVE_COVERED_BLK_SIZE 10					// すでにカバーされているブロックに侵攻できる上限
// 時間パラメータ
#define UPPER_TIME_LIMIT 6000
#define DIFF_TIME 1000

using namespace std;

int GLOBAL_PLACE_BLK_FLAG = 0;
int PLACE_BLOCK_COUNT = 0;
int GLOBAL_INCREMENTAL_ERASED_BLK = 0;

namespace utility {
	struct timer {
private:
	const std::chrono::system_clock::time_point start;

public:
	const unsigned time_limit = UPPER_TIME_LIMIT - DIFF_TIME;  // 2000 ms くらいは余裕を持たせる

	timer() noexcept : start(std::chrono::system_clock::now()) {}

	// 経過時間 (ms) を返す
	[[nodiscard]] auto elapsed() const {
		using namespace std::chrono;
		return duration_cast<milliseconds>(system_clock::now() - start).count();
	}

    // 経過時間が制限時間の num/den 倍未満かを返す
    // 例えば frac<1, 2>() は経過時間が制限時間の 1/2 未満かを返す
   template <unsigned num, unsigned den> [[nodiscard]] bool frac() const {
		return elapsed() < time_limit * num / den;
   }

   // 経過時間が制限時間未満かを返す
   [[nodiscard]] bool good() const { return elapsed() < time_limit; }
  	};
}  // namespace utility

class RandomNumber {
public:
	mt19937 mt;
	RandomNumber(int seed) : mt(seed) {} 
	int range(int min, int max) {
		uniform_int_distribution<> ransu(min,max); // [min,max]の乱数
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
	int other_blk_index;			// もう一方で使用しているblockのindex
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
	unsigned int MAX_ID;
	//vector<unsigned int> erased_block;	// いらないかも
	vector<vector<Block>> block;
	vector<vector<vector<vector<int>>>> block_xyz;
	vector<vector<Silhouette>> cnt_block;
	vector<vector<Silhouette>> silt, covered_silt;	// covered_siltはカバーされていなかったら1が入っている
	RandomNumber random_number;
	vector<vector<vector<vector<vector<Pos>>>>> overlap;
	Solver(vector<vector<Silhouette>> silt, vector<vector<Silhouette>> covered_silt, int D, RandomNumber random_number,
			vector<vector<vector<vector<vector<Pos>>>>> overlap) : 
			silt(silt), covered_silt(silt), D(D), random_number(random_number), overlap(overlap), MAX_ID(1) {
		block.resize(N_SILHOUETTE);
		block_xyz.resize(N_SILHOUETTE);
		cnt_block.resize(N_SILHOUETTE);
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
			cnt_block[i].resize(N_SILHOUETTE);
			for (int f_r = 0; f_r < N_SILHOUETTE; f_r++) {
				cnt_block[i][f_r].zx_zy.resize(D);
				for (int z = 0; z < D; z++) {
					cnt_block[i][f_r].zx_zy[z].resize(D);
					for (int xy = 0; xy < D; xy++) cnt_block[i][f_r].zx_zy[z][xy] = 0;
				}
			}
		}
	}
	
	int solve(int lns_flag) {
#if !CONTEST
		clock_t start = clock();
#endif
		lns_flag = 1;
		if (lns_flag == 1) place_block();		// 1×2以上のブロックを設置->複数回繰り返した方が良さそう
		place_two_block();							// 1×2のブロックを設置
		one_by_one_peace();							// 1×1のブロックを設置
		add_index();									// blk_indexとpair_flagの修正
		erase_overlap_block();						// ブロックの重なりを除去
		add_index();									// blk_indexとpair_flagの修正 -> ブロック消去後は修正が必要
#if !CONTEST
		clock_t end = clock();
		//cout << "duration: " <<  (double)(end - start) / CLOCKS_PER_SEC << "sec\n";
#endif
		int ret = legal_check();
		return ret;
	}

	void add_index() {
		// 本来は2×1ブロック追加時(+それ以上時も)に考慮すべき
		int silt_id = SILT_1, other_silt_id = SILT_2;
		for (int j = 0; j < block[silt_id].size(); j++) {
			for (int k = 0; k < block[other_silt_id].size(); k++) {
				if (block[silt_id][j].block_id == block[other_silt_id][k].block_id) { 
					block[silt_id][j].pair_flag = 1;
					block[silt_id][j].other_blk_index = k;
					block[other_silt_id][k].pair_flag = 1;
					block[other_silt_id][k].other_blk_index = j;
					break;
				}
			}
		}
	}

	void erase_one_side_block(int silt_id, int index) {	// 片側にしか使ってないブロックの削除
		for (int i = 0; i < block[silt_id][index].pos.size(); i++) {
			Pos pos = block[silt_id][index].pos[i];
			// ブロックの削除
			cnt_block[silt_id][FRONT].zx_zy[pos.z][pos.x]--;
			cnt_block[silt_id][RIGHT].zx_zy[pos.z][pos.y]--;
			if (cnt_block[silt_id][FRONT].zx_zy[pos.z][pos.x] == 0) {
				covered_silt[silt_id][FRONT].zx_zy[pos.z][pos.x] = 1;
			}
			if (cnt_block[silt_id][RIGHT].zx_zy[pos.z][pos.y] == 0) {
				covered_silt[silt_id][RIGHT].zx_zy[pos.z][pos.y] = 1;
			}
			block_xyz[silt_id][pos.x][pos.y][pos.z] = 0;
		}
		// block自体の削除
		block[silt_id].erase(cbegin(block[silt_id]) + index);
	}

	void erase_double_side_block(int silt_id, int index) {
		int other_silt_id = 1 - silt_id;
		int other_index = block[silt_id][index].other_blk_index;
		// blockの削除
		for (int i = 0; i < block[silt_id][index].pos.size(); i++) {
			Pos pos_1 = block[silt_id][index].pos[i], pos_2 = block[other_silt_id][other_index].pos[i];
			// SILT_1
			cnt_block[silt_id][FRONT].zx_zy[pos_1.z][pos_1.x]--;
			cnt_block[silt_id][RIGHT].zx_zy[pos_1.z][pos_1.y]--;
			if (cnt_block[silt_id][FRONT].zx_zy[pos_1.z][pos_1.x] == 0) {
				covered_silt[silt_id][FRONT].zx_zy[pos_1.z][pos_1.x] = 1;
			}
			if (cnt_block[silt_id][RIGHT].zx_zy[pos_1.z][pos_1.y] == 0) {
				covered_silt[silt_id][RIGHT].zx_zy[pos_1.z][pos_1.y] = 1;
			}
			block_xyz[silt_id][pos_1.x][pos_1.y][pos_1.z] = 0;
			// SILT_2
			cnt_block[other_silt_id][FRONT].zx_zy[pos_2.z][pos_2.x]--;
			cnt_block[other_silt_id][RIGHT].zx_zy[pos_2.z][pos_2.y]--;
			if (cnt_block[other_silt_id][FRONT].zx_zy[pos_2.z][pos_2.x] == 0) {
				covered_silt[other_silt_id][FRONT].zx_zy[pos_2.z][pos_2.x] = 1;
			}
			if (cnt_block[other_silt_id][RIGHT].zx_zy[pos_2.z][pos_2.y] == 0) {
				covered_silt[other_silt_id][RIGHT].zx_zy[pos_2.z][pos_2.y] = 1;
			}
			block_xyz[other_silt_id][pos_2.x][pos_2.y][pos_2.z] = 0;
		}
		// block自体の削除(ここは先に削除した方が見やすいかも)
		block[silt_id].erase(cbegin(block[silt_id]) + index);
		block[other_silt_id].erase(cbegin(block[other_silt_id]) + other_index);
	}

	void erase_block() {
		int ransu;
		for (int cnt = 0; cnt < N_ERASED_BLOCK_ALNS + GLOBAL_INCREMENTAL_ERASED_BLK; cnt++) {
			if (block[SILT_1].size() == 0 || block[SILT_2].size() == 0) break;
			if (block[SILT_1].size() <= block[SILT_2].size()) { 
				ransu = random_number.range(0, block[SILT_2].size() - 1);	// for SILT_2
				if (block[SILT_2][ransu].pair_flag == 1) {
					erase_double_side_block(SILT_2, ransu);
				} else {
					erase_one_side_block(SILT_2, ransu);
				}
			} else if (block[SILT_1].size() > block[SILT_2].size()) { 		// for SILT_1
				ransu = random_number.range(0, block[SILT_1].size() - 1);
				if (block[SILT_1][ransu].pair_flag == 1) {
					erase_double_side_block(SILT_1, ransu);
				} else {
					erase_one_side_block(SILT_1, ransu);
				}
			}
			add_index();
		}
		//fix_other_blk_index();
	}

	void erase_overlap_block() {
		int silt_id = SILT_1, other_silt_id = SILT_2;
		for (int i = block[silt_id].size()-1; i >= 0; i--) {
			if (block[silt_id][i].pair_flag == 0) continue;
			int failed_flag = 0;
			vector<vector<Silhouette>> copy_cnt_block = cnt_block;
			// 左側のシルエットに着目(SILT_1)
			for (int j = 0; j < block[silt_id][i].pos.size(); j++) {
				Pos pos_1 = block[silt_id][i].pos[j];//, pos_2 block[1-silt_id][i].pos;
				copy_cnt_block[silt_id][FRONT].zx_zy[pos_1.z][pos_1.x]--;
				copy_cnt_block[silt_id][RIGHT].zx_zy[pos_1.z][pos_1.y]--;
				if (copy_cnt_block[silt_id][FRONT].zx_zy[pos_1.z][pos_1.x] <= 0 || copy_cnt_block[silt_id][RIGHT].zx_zy[pos_1.z][pos_1.y] <= 0) {
					failed_flag = 1;
					break;
				}
			}
			if (failed_flag == 1) continue;	// 消すとまずいブロックであることが判明(SILT_1)
			// 右側のシルエットに着目(SILT_2)
			for (int j = 0; j < block[other_silt_id][block[silt_id][i].other_blk_index].pos.size(); j++) {
				Pos pos_2 = block[other_silt_id][block[silt_id][i].other_blk_index].pos[j];
				copy_cnt_block[other_silt_id][FRONT].zx_zy[pos_2.z][pos_2.x]--;
				copy_cnt_block[other_silt_id][RIGHT].zx_zy[pos_2.z][pos_2.y]--;
				if (copy_cnt_block[other_silt_id][FRONT].zx_zy[pos_2.z][pos_2.x] <= 0 || copy_cnt_block[other_silt_id][RIGHT].zx_zy[pos_2.z][pos_2.y] <= 0) {
					failed_flag = 1;
					break;
				}
			}
			if (failed_flag == 1) continue;	// 消すとまずいブロック(SILT_2)

			// ブロックの削除 -> 削除しても問題なし
			for (int j = 0; j < block[silt_id][i].pos.size(); j++) {
				Pos pos_1 = block[silt_id][i].pos[j], pos_2 = block[other_silt_id][block[silt_id][i].other_blk_index].pos[j];
				// SILT_1
				cnt_block[silt_id][FRONT].zx_zy[pos_1.z][pos_1.x]--;
				cnt_block[silt_id][RIGHT].zx_zy[pos_1.z][pos_1.y]--;
				// SILT_1 -> cnt_block>0なのでcovered_siltの修正, は必要ない(前で評価済み)
				block_xyz[silt_id][pos_1.x][pos_1.y][pos_1.z] = 0;
				// SILT_2
				cnt_block[other_silt_id][FRONT].zx_zy[pos_2.z][pos_2.x]--;
				cnt_block[other_silt_id][RIGHT].zx_zy[pos_2.z][pos_2.y]--;
				// SILT_2 -> cnt_block>0なのでcovered_siltの修正, は必要ない(前で評価済み)
				block_xyz[other_silt_id][pos_2.x][pos_2.y][pos_2.z] = 0;
			}
			// block自体の削除
			int other_index = block[silt_id][i].other_blk_index;
			//erased_block.push_back(block[silt_id][i].block_id);
			//erase_block.push_back(block[other_silt_id][other_index].block_id); -> おなじblock_idがaddされるので必要ない
			block[silt_id].erase(cbegin(block[silt_id]) + i);
			block[other_silt_id].erase(cbegin(block[other_silt_id]) + other_index);
		}
	}

/* place block */	
	vector<Pos> rotation(const vector<Pos> &added_block, double x_rad, double y_rad, double z_rad) {
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
		//vector<Pos> start_xyz_pos; vector<int> start_xyz_pos_index;
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

	int overlap_check_for_added_block(int silt_id, int other_silt_id, const vector<Pos> &added_block, const vector<Pos> &other_added_block) {
		int overlap_count = 0;
		for (int i = 0; i < added_block.size(); i++) {
			int x = added_block[i].x, y = added_block[i].y, z = added_block[i].z;
			if (covered_silt[silt_id][FRONT].zx_zy[z][x] == 0 && covered_silt[silt_id][RIGHT].zx_zy[z][y] == 0) overlap_count++;
		}
		if (overlap_count == added_block.size()) {
			return 0;
		}
		return 1;
	}

	void check_not_covered_block(vector<vector<Pos>> &start_xyz_pos, vector<vector<int>> &start_xyz_pos_index) {
		int silt_id = SILT_1, other_silt_id = SILT_2;
		int cnt = 0, other_cnt = 0;
		for (int x = 0; x < D; x++) {
			for (int y = 0; y < D; y++) {
				for (int z = 0; z < D; z++) {
					if (block_xyz[silt_id][x][y][z] == 0 && ((covered_silt[silt_id][FRONT].zx_zy[z][x] == 1 && silt[silt_id][RIGHT].zx_zy[z][y] == 1) || \
							(silt[silt_id][FRONT].zx_zy[z][x] == 1 && covered_silt[silt_id][RIGHT].zx_zy[z][y] == 1))) {
						start_xyz_pos[silt_id][cnt] = Pos{x,y,z};
						start_xyz_pos_index[silt_id][cnt] = cnt;
						cnt++;
					}
					if (block_xyz[other_silt_id][x][y][z] == 0 && ((covered_silt[other_silt_id][FRONT].zx_zy[z][x] == 1 && silt[other_silt_id][RIGHT].zx_zy[z][y] == 1) || \
							(silt[other_silt_id][FRONT].zx_zy[z][x] == 1 && covered_silt[other_silt_id][RIGHT].zx_zy[z][y] == 1))) {
						start_xyz_pos[other_silt_id][other_cnt] = Pos{x,y,z};
						start_xyz_pos_index[other_silt_id][other_cnt] = other_cnt;
						other_cnt++;
					}
				}
			}
		}
		// 要素削除
		start_xyz_pos[silt_id].erase(start_xyz_pos[silt_id].begin()+cnt, start_xyz_pos[silt_id].end());
		start_xyz_pos_index[silt_id].erase(start_xyz_pos_index[silt_id].begin()+cnt, start_xyz_pos_index[silt_id].end());
		start_xyz_pos[other_silt_id].erase(start_xyz_pos[other_silt_id].begin()+other_cnt, start_xyz_pos[other_silt_id].end());
		start_xyz_pos_index[other_silt_id].erase(start_xyz_pos_index[other_silt_id].begin()+other_cnt, start_xyz_pos_index[other_silt_id].end());
		//if (cnt > other_cnt) cout << "aaa" << "\n";
		
		// インデックスの保存
		//cnt_index[silt_id] = cnt;
		//cnt_index[other_silt_id] = other_cnt;
	}

	void place_block() {
		PLACE_BLOCK_COUNT++;
		// アルゴリズム
		// 1.シルエットの重なり部分からスタート
		vector<vector<Pos>> start_xyz_pos(N_SILHOUETTE, vector<Pos>(D*D*D));
		vector<vector<int>> start_xyz_pos_index(N_SILHOUETTE, vector<int>(D*D*D));
		check_not_covered_block(start_xyz_pos, start_xyz_pos_index);
		int min_index_num;
		if (start_xyz_pos[SILT_1].size() < start_xyz_pos[SILT_2].size()) min_index_num = start_xyz_pos[SILT_1].size();
		else min_index_num = start_xyz_pos[SILT_2].size();
		random_number.array_shuffle(start_xyz_pos_index[SILT_1]); 
		random_number.array_shuffle(start_xyz_pos_index[SILT_2]);
		
		int limit = 0;
		// リミットの決定
		if (GLOBAL_PLACE_BLK_FLAG == 0) {
			if (N_FIRST_PLACE_BLOCK_LIMIT >= min_index_num) limit = min_index_num;
			else limit = N_FIRST_PLACE_BLOCK_LIMIT;
			GLOBAL_PLACE_BLK_FLAG = 1;
		} else {
			if (D + N_PLACE_BLOCK_LIMIT >= min_index_num) limit = min_index_num;
			else limit = D + N_PLACE_BLOCK_LIMIT;
		}
		// 探索の開始
		for (int index = 0; index < limit; index++) {
			// 1-1.silt_idとright_frontの決定
			int silt_id = random_number.range(0,1);	//int right_front = RIGHT;
			int other_silt_id = 1 - silt_id;
			int x = start_xyz_pos[silt_id][start_xyz_pos_index[silt_id][index]].x, y = start_xyz_pos[silt_id][start_xyz_pos_index[silt_id][index]].y, z = start_xyz_pos[silt_id][start_xyz_pos_index[silt_id][index]].z;
			if (overlap[silt_id][x][y][z].size() == 0 || block_xyz[silt_id][x][y][z] > 0) continue; //x,y,zにおけるか調査
			Pos cur_pos = Pos{x,y,z};
			vector<Pos> added_block = {Pos{x,y,z}};	// silt_idにaddするblock
			vector<Pos> other_added_block;				// other_silt_idにaddするblock
			vector<vector<vector<int>>> fake_block(D,vector<vector<int>>(D,vector<int>(D,0)));
			fake_block[cur_pos.x][cur_pos.y][cur_pos.z] = 1;	// fake_block
			// 2.進行方向を決める(ここもうちょっと頑張る？)
			// ovelap_indexの作成
			vector<int> overlap_index(overlap[silt_id][cur_pos.x][cur_pos.y][cur_pos.z].size());
			for (int i = 0; i < overlap_index.size(); i++) overlap_index[i] = i;
			random_number.array_shuffle(overlap_index);
			for (int cnt = 0; cnt < overlap_index.size(); cnt++) {
				int i = overlap_index[cnt];
				// 3.ブロックの重なりがないかチェック
				Pos next_pos = overlap[silt_id][cur_pos.x][cur_pos.y][cur_pos.z][i];
				if (fake_block[next_pos.x][next_pos.y][next_pos.z] == 1 || block_xyz[silt_id][next_pos.x][next_pos.y][next_pos.z] > 0 || \
						(added_block.size() >= MOVE_COVERED_BLK_SIZE && covered_silt[silt_id][FRONT].zx_zy[next_pos.z][next_pos.x] == 0 && covered_silt[silt_id][RIGHT].zx_zy[next_pos.z][next_pos.y] == 0)) {
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
					if (added_block.size() >= UPPER_BLOCK_SIZE) break;	// ブロックのサイズを制限
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
				if (overlap_check_for_added_block(silt_id, other_silt_id, added_block, other_added_block)) {
					// silt_idにblockを追加
					add_block(silt_id, added_block);
					// other_siltにblockを追加
					add_block(other_silt_id, other_added_block);
					MAX_ID++;
				}
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

/* place two or one block */
	void add_block(int silt_id, const vector<Pos> &add_pos) {
		// blockの追加, block_xyzの更新
		block[silt_id].push_back(Block{});
		for (int i = 0; i < add_pos.size(); i++) {
			block[silt_id][block[silt_id].size()-1].pos.push_back(add_pos[i]);
			block_xyz[silt_id][add_pos[i].x][add_pos[i].y][add_pos[i].z] = MAX_ID;
			cnt_block[silt_id][FRONT].zx_zy[add_pos[i].z][add_pos[i].x]++; cnt_block[silt_id][RIGHT].zx_zy[add_pos[i].z][add_pos[i].y]++;
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
						cnt_block[i][FRONT].zx_zy[z][x]++; cnt_block[i][RIGHT].zx_zy[z][y]++;
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
										cnt_block[SILT_2][FRONT].zx_zy[z_2][x_2]++; cnt_block[SILT_2][RIGHT].zx_zy[z_2][y_2]++;
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
/* place two or one block */

	long double evaluation() {
		int value = 0;
		double wrk_value = 0;
		for (int i = 0; i < N_SILHOUETTE; i++) {
			for (int j = 0; j < block[i].size(); j++) {
				for (int k = 0; k < block[1-i].size(); k++) {
					if (block[i][j].block_id == block[1-i][k].block_id) {
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

		int block_cnt = 1;
		unordered_map<string, int> hash, other_hash;	// stringよりintの方が早い
		int silt_id = SILT_1, other_silt_id = SILT_2;
		for (int index = 0; index < block[silt_id].size(); index++) {
			for (int i = 0; i < block[silt_id][index].pos.size(); i++) {
				Pos pos = block[silt_id][index].pos[i];
				string key = to_string(pos.x) + "_" + to_string(pos.y) + "_" + to_string(pos.z);
				hash[key] = block_cnt;
				if (block[silt_id][index].pair_flag == 1) {
					Pos other_pos = block[other_silt_id][block[silt_id][index].other_blk_index].pos[i];
					string other_key = to_string(other_pos.x) + "_" + to_string(other_pos.y) + "_" + to_string(other_pos.z);
					other_hash[other_key] = block_cnt;
				}
			}
			block_cnt++;
		}
		for (int index = 0; index < block[other_silt_id].size(); index++) {
			if (block[other_silt_id][index].pair_flag == 1) continue;	// 事前に追加済み
			for (int i = 0; i < block[other_silt_id][index].pos.size(); i++) {
				Pos other_pos = block[other_silt_id][index].pos[i];
				string other_key = to_string(other_pos.x) + "_" + to_string(other_pos.y) + "_" + to_string(other_pos.z);
				other_hash[other_key] = block_cnt;
			}
			block_cnt++;
		}
		// print out
		//vector<vector<Pos>> blk(2,vector<Pos>());
		fprintf(fpOutput, "%d\n", block_cnt - 1);
		for (int x = 0; x < D; x++) for (int y = 0; y < D; y++) for (int z = 0; z < D; z++) {
			string key = to_string(x) + "_" + to_string(y) + "_" + to_string(z);
			if (hash.count(key) == 0) {
				fprintf(fpOutput, "0 ");
			} else {
				fprintf(fpOutput, "%d ", hash[key]);
			}
		}
		fprintf(fpOutput, "\n");
		for (int x = 0; x < D; x++) for (int y = 0; y < D; y++) for (int z = 0; z < D; z++) {
			string key = to_string(x) + "_" + to_string(y) + "_" + to_string(z);
			if (other_hash.count(key) == 0) {
				fprintf(fpOutput, "0 ");
			} else {
				fprintf(fpOutput, "%d ", other_hash[key]);
			}
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
		int block_cnt = 1;
		unordered_map<string, int> hash, other_hash;	// stringよりintの方が早い
		int silt_id = SILT_1, other_silt_id = SILT_2;
		for (int index = 0; index < block[silt_id].size(); index++) {
			for (int i = 0; i < block[silt_id][index].pos.size(); i++) {
				Pos pos = block[silt_id][index].pos[i];
				string key = to_string(pos.x) + "_" + to_string(pos.y) + "_" + to_string(pos.z);
				hash[key] = block_cnt;
				if (block[silt_id][index].pair_flag == 1) {
					Pos other_pos = block[other_silt_id][block[silt_id][index].other_blk_index].pos[i];
					string other_key = to_string(other_pos.x) + "_" + to_string(other_pos.y) + "_" + to_string(other_pos.z);
					other_hash[other_key] = block_cnt;
				}
			}
			block_cnt++;
		}
		for (int index = 0; index < block[other_silt_id].size(); index++) {
			if (block[other_silt_id][index].pair_flag == 1) continue;	// 事前に追加済み
			for (int i = 0; i < block[other_silt_id][index].pos.size(); i++) {
				Pos other_pos = block[other_silt_id][index].pos[i];
				string other_key = to_string(other_pos.x) + "_" + to_string(other_pos.y) + "_" + to_string(other_pos.z);
				other_hash[other_key] = block_cnt;
			}
			block_cnt++;
		}
		// print out
		//vector<vector<Pos>> blk(2,vector<Pos>());
		cout << block_cnt - 1 << endl;
		for (int x = 0; x < D; x++) for (int y = 0; y < D; y++) for (int z = 0; z < D; z++) {
			string key = to_string(x) + "_" + to_string(y) + "_" + to_string(z);
			if (hash.count(key) == 0) {
				cout << 0 << " ";
			} else {
				cout << hash[key] << " ";
			}
		}
		cout << endl;
		for (int x = 0; x < D; x++) for (int y = 0; y < D; y++) for (int z = 0; z < D; z++) {
			string key = to_string(x) + "_" + to_string(y) + "_" + to_string(z);
			if (other_hash.count(key) == 0) {
				cout << 0 << " ";
			} else {
				cout << other_hash[key] << " ";
			}
		}
		cout << endl;
	}

	void check() {
		for (int i = 0; i < block[SILT_1].size(); i++) {
			for (int j = i + 1; j < block[SILT_1].size(); j++) {
				if (block[SILT_1][i].block_id == block[SILT_1][j].block_id) {
					getchar();
				}
			}
		}
		for (int i = 0; i < block[SILT_2].size(); i++) {
			for (int j = i + 1; j < block[SILT_2].size(); j++) {
				if (block[SILT_2][i].block_id == block[SILT_2][j].block_id) {
					getchar();
				}
			}
		}
	}
};

int main(int argc, char* argv[]) {

#if !CONTEST
	cout << "\n";
	string filename, output_filename, score_filename, w_filename;
	if (argc >= 2) {
		w_filename = argv[1];
		filename = "tester/inst/" + w_filename;
		output_filename = "tester/out/" + w_filename;
		score_filename = "tester/score/" + w_filename;
	} else {
		w_filename = INSTANCE_NAME;
		filename = "debug/tester/inst/" + w_filename;
		output_filename = "debug/tester/out/" + w_filename;
		score_filename = "debug/tester/score/" + w_filename;
	}
	ifstream in(filename);
	if (!in) { cout << "cannot read inst error"; getchar(); }
	cin.rdbuf(in.rdbuf());
#endif

	const utility::timer tm;
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
	RandomNumber random_number(SEED);
	Solver solver(silt, silt, D, random_number, overlap);
	int ret = solver.solve(0);
	long double score = solver.evaluation();
	// ALNSの準備
	int flag = 0;
	Solver solver_alns = solver;
	for (int cnt = 0; tm.good(); cnt++) {
		if (flag == 1) {
			// コピー
			solver_alns.MAX_ID = solver.MAX_ID;
			solver_alns.block = solver.block;
			solver_alns.block_xyz = solver.block_xyz;
			solver_alns.cnt_block = solver.cnt_block;
			solver_alns.silt = solver.silt;
			solver_alns.covered_silt = solver.covered_silt;
			solver_alns.overlap = solver.overlap;				
		}
		solver_alns.erase_block();
		int ret_2 = solver_alns.solve(1);
		long double alns_score = solver_alns.evaluation();
		//cout << score << " vs. " << alns_score << "\n";
		if (alns_score < score) {
			// コピー
			solver.MAX_ID = solver_alns.MAX_ID;
			solver.block = solver_alns.block;
			solver.block_xyz = solver_alns.block_xyz;
			solver.cnt_block = solver_alns.cnt_block;
			solver.silt = solver_alns.silt;
			solver.covered_silt = solver_alns.covered_silt;
			solver.overlap = solver_alns.overlap;
			// scoreの更新
			score = alns_score;
			//cout << score << "\n";
			//solver.print_block();solver.output_result_file(output_filename);cout << score << "\n"; //solver.check();	// 後で消す
			cout << score << "\n"; 
			flag = 0;
			// for 消すブロックの数を増加
			cnt = 0;
			GLOBAL_INCREMENTAL_ERASED_BLK = 0;
		} else {
			// for 消すブロックの数を増加
			if (cnt > INC_ERASED_BLK_THRESHOLD) {
				GLOBAL_INCREMENTAL_ERASED_BLK++;
				//cout << GLOBAL_INCREMENTAL_ERASED_BLK << "\n";
				if (GLOBAL_INCREMENTAL_ERASED_BLK == 3) GLOBAL_INCREMENTAL_ERASED_BLK = 3;
				cnt = 0;
			}
			//solver_alns.print_block();
			//solver_alns.check();	// 後で消す
			flag = 1;
		}
	}
	if (ret == 1) {
		score = solver.evaluation();
		cout << "seed: " << w_filename << "\t" << score << "\t";// << "legal" << ": " << score << "\n";
		solver.output_result_file(output_filename);
		solver.output_score(score_filename, score);
		//solver.print_block();
	} else {
		cout << "wtf"; getchar();
		score = solver.evaluation();
		cout << "seed: " << w_filename;// << "  " << "legal" << ": " << score << "\n";
		solver.output_result_file(output_filename);
		solver.output_score(score_filename, score);
		//solver.print_block();
	}
#else
	RandomNumber random_number(SEED);
	Solver solver(silt, silt, D, random_number, overlap);
	int ret = solver.solve(0);
	long double score = solver.evaluation();
	// LNSの準備
	int flag = 0;
	Solver solver_alns = solver;
	for (int cnt = 0; cnt < N_ALNS_REPEATS && tm.good(); cnt++) {
		if (flag == 1) {
			// コピー
			solver_alns.MAX_ID = solver.MAX_ID;
			solver_alns.block = solver.block;
			solver_alns.block_xyz = solver.block_xyz;
			solver_alns.cnt_block = solver.cnt_block;
			solver_alns.silt = solver.silt;
			solver_alns.covered_silt = solver.covered_silt;
			solver_alns.overlap = solver.overlap;				
		}
		solver_alns.erase_block();
		int ret_2 = solver_alns.solve(1);
		long double alns_score = solver_alns.evaluation();
		if (alns_score < score) {
			// コピー
			solver.MAX_ID = solver_alns.MAX_ID;
			solver.block = solver_alns.block;
			solver.block_xyz = solver_alns.block_xyz;
			solver.cnt_block = solver_alns.cnt_block;
			solver.silt = solver_alns.silt;
			solver.covered_silt = solver_alns.covered_silt;
			solver.overlap = solver_alns.overlap;
			// scoreの更新
			score = alns_score;
			flag = 0;
		} else {
			flag = 1;
		}
	}
	solver.print_block();
#endif
}
