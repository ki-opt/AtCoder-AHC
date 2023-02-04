#include <iostream>
#include <vector>
#include <string>

using std::vector; using std::cout; using std::cin;
using std::string;

class Instance {
// メンバ関数
public:
   int N, M, D, K;
   vector<vector<int>> edge_weight;
   vector<vector<int>> node_coord;

// コンストラクタ
public:
   //instance_read(string filename) { }
   Instance() {
      cin >> this->N; cin >> this->M; cin >> this->D; cin >> this->K;
      this->edge_weight.assign(M, vector<int>(3));
      this->node_coord.assign(N, vector<int>(2));
      for (int i = 0; i < M; i++) { for (int j = 0; j < 3; j++) cin >> edge_weight[i][j]; }
      for (int i = 0; i < N; i++) { for (int j = 0; j < 2; j++) cin >> node_coord[i][j]; }
   };
};

class Solution {
public:
   vector<int> sol;
};



int main() {
   Instance inst();
   Solution solution;


   return 0;
}