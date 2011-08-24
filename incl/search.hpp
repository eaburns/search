#include <vector>

template <class D>
struct Result {
	unsigned long expd, gend;
	long dups;
	typename D::Cost cost;
	std::vector<typename D::State> path;

	Result() : expd(0), gend(0), dups(0), cost(D::InfCost) { }
};