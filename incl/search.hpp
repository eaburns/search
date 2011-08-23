#include <vector>

template <class D>
struct Result {
	unsigned long expd, gend;
	long dups;
	unsigned int cost;
	std::vector<typename D::State> path;
};