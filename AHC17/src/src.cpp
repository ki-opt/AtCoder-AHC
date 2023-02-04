#include <iostream>
#include <vector>
#include <string>
#include <random>

using std::vector; using std::cout; using std::cin;
using std::string; using std::mt19937; using std::uniform_int_distribution;

#define CONTEST false

class Instance {
// メンバ関数
public:
   int N, M, D, K;
   vector<vector<int>> edge_weight;
   vector<vector<int>> node_coord;

// コンストラクタ
public:
   Instance(string filename) { 
      FILE *fpFile;
      fpFile = fopen(filename.c_str(),"r");
      if (fpFile == NULL) { cout << "cannot read instance"; getchar(); }
      fscanf(fpFile,"%d %d %d %d", &N, &M, &D, &K);
      this->edge_weight.assign(M, vector<int>(3));
      this->node_coord.assign(N, vector<int>(2));
      for (int i = 0; i < M; i++) { for (int j = 0; j < 3; j++) fscanf(fpFile, "%d", &edge_weight[i][j]); }
      for (int i = 0; i < N; i++) { for (int j = 0; j < 2; j++) fscanf(fpFile, "%d", &node_coord[i][j]); }
      fclose(fpFile);
   }
   Instance() {
      cin >> this->N; cin >> this->M; cin >> this->D; cin >> this->K;
      this->edge_weight.assign(M, vector<int>(3));
      this->node_coord.assign(N, vector<int>(2));
      for (int i = 0; i < M; i++) { for (int j = 0; j < 3; j++) cin >> edge_weight[i][j]; }
      for (int i = 0; i < N; i++) { for (int j = 0; j < 2; j++) cin >> node_coord[i][j]; }
   };
};

class RandomNumber {
public:
   mt19937 rnd;
   RandomNumber(int seed) {
      this->rnd.seed(seed);
   }
   int random_distribution(int min_val, int max_val) {
      uniform_int_distribution<> dist(min_val, max_val);
      return dist(rnd);
   }
};

class Solution {
public:
   vector<int> sol;
   vector<int> kozi_count;
public:
   Solution(const Instance &inst) { 
      sol.resize(inst.M);
      kozi_count.assign(inst.D,0);
   }
public:
   void create_random_init_sol(const Instance &inst, RandomNumber &rnd) {
      for (int i = 0; i < inst.M; i++) {
         int random_number = rnd.random_distribution(1,inst.D);
         while (kozi_count[random_number - 1] > inst.K) random_number = rnd.random_distribution(1,inst.D);
         sol[i] = random_number;
         kozi_count[random_number - 1]++;
      }
   }
   void output(const Instance &inst) {
      for (int i = 0; i < inst.M - 1; i++) cout << sol[i] << " ";
      cout << sol[sol.size() - 1];
   }
   void output_txtfile(const Instance &inst) {
      FILE *fpWrite;
      string fnameWrite = "result.txt";
      fpWrite = fopen(fnameWrite.c_str(),"w");
      if (fpWrite == NULL) { cout << "cannot output result"; getchar(); }
      for (int i = 0; i < inst.M - 1; i++) fprintf(fpWrite, "%d ", sol[i]);
      fprintf(fpWrite, "%d ", sol[sol.size()-1]);
      fclose(fpWrite);
   }
};

int main() {
#if CONTEST
   Instance inst;                            // for AHC
#else
   Instance inst("inst/inst_0.txt");         // for test
#endif
   Solution solution(inst);
   RandomNumber rnd(0);

   // 初期解の生成
   solution.create_random_init_sol(inst, rnd);

   // 解の出力
#if CONTEST
   solution.output(inst);
#else
   solution.output_txtfile(inst);
#endif

   return 0;
}