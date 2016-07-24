
#ifndef __W_MATCHING_H_
#define __W_MATCHING_H_

#include <vector>

const int maxn = 11;
const int inf = (1<<30) - 1;

class WMatching
{
	public:
		WMatching(int _n, std::vector< std::vector<int> > wvec);

		void read();
		bool dfs(int);
		bool find();
		void increase_matching();
		void calculate();

		int match[maxn];

	private:
		int n,u[maxn],v[maxn];
		int w[maxn][maxn];
		bool g[maxn][maxn];

		bool mark[maxn],matched[maxn];
};

#endif // __W_MATCHING_H_
