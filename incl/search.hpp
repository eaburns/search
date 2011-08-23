#include <vector>

template <class D> struct Result {
	unsigned long expd, gend;
	long dups;
	D::Cost cost;
	vector<D::State> path;
};