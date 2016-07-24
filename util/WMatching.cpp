
#include <WMatching.h>

#include <iostream>
#include <cstring>

#include <cmath>

using namespace std;

WMatching::WMatching(int _n, vector< vector<int> > wvec):
	n(_n)
{
	for (unsigned i = 0 ; i < wvec.size() ; i++)
		for (unsigned j = 0 ; j < wvec.size() ; j++)
			w[i][j] = wvec[i][j];
}

void WMatching::read()
{
	memset(u, 0, sizeof u);
	memset(v, 0, sizeof v);
	for (int i = 0 ; i < n ; ++i)
	{
		u[i]=-inf;
		for (int j = 0 ; j < n ; ++j)
			u[i]=max(u[i],w[i][j]);
	}
}

bool WMatching::dfs(int vv)
{
	mark[vv]=true;
	for (int i = 0 ; i < n ; ++i)
		if (g[vv][i] && (match[i]==-1 || !mark[match[i]] && dfs(match[i])))
			return (match[i]=vv),1;
	return false;
}

bool WMatching::find()
{
	memset(mark, 0, sizeof(mark));
	for (int i = 0 ; i < n ; ++i)
		if (!mark[i] && !matched[i] && dfs(i))
			return (matched[i]=true);
	return false;
}

void WMatching::increase_matching()
{
	for (int l = 0 ; l < n ; ++l)
	{
		for (int i = 0 ; i < n ; ++i)
			for (int j = 0 ; j < n ; ++j)
				g[i][j]=(u[i] + v[j] == w[i][j]);
		if (find())
			return ;
		int eps=inf;
		for (int i = 0 ; i < n ; ++i)
			for (int j = 0 ; j < n ; ++j)
				if (mark[i] && (match[j]==-1 || !mark[match[j]]))
					eps=min(eps, u[i] + v[j] - w[i][j]);
		for (int i = 0 ; i < n ; ++i)
		{
			if (mark[i])
				u[i]-=eps;
			if (match[i]!=-1 && mark[match[i]])
				v[i]+=eps;
		}
	}
}

void WMatching::calculate()
{
	read();
	memset(match,-1,sizeof(match));
	memset(matched,0,sizeof matched);
	for (int i = 0 ; i < n ; ++i)
		increase_matching();


//	for (int i = 0 ; i < n ; ++i)
//		cout << match[i] << ' ';
//	cout << endl;
}
