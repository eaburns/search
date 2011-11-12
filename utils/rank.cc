// Based on "Efficient Algorithms to Rank and Unrank
// Permutations in Lexicograhpic Order," B. Bonet

#include "utils.hpp"
#include <cstring>	// memset

static unsigned int left(unsigned int);
static unsigned int parent(unsigned int);
static bool odd(unsigned int);

Ranker::Ranker(unsigned int sz) :
		sz(sz), bits(ilog2(sz)), n(sz), treesz((1 << (bits + 1)) + 1) {
	tree = new unsigned int[treesz];
}

Ranker::Ranker(unsigned int sz, unsigned int n) :
		sz(sz), bits(ilog2(sz)), n(n), treesz((1 << (bits + 1)) + 1) {
	tree = new unsigned int[treesz];
}

Ranker::~Ranker(void) {
	delete tree;
}

unsigned long Ranker::rank(const unsigned int p[])
{
	unsigned long rank = 0;
	memset(tree, 0, treesz * sizeof(tree[0]));

	for (unsigned int i = 1; i <= n; i++) {
		unsigned int ctr = p[i-1];
		unsigned int nd = (1 << bits) + p[i-1];

		for (unsigned int j = 1; j <= bits; j++) {
			unsigned int prnt = parent(nd);
			if (odd(nd))
				ctr -= tree[left(prnt)];
			tree[nd]++;
			nd = prnt;
		}

		tree[nd]++;
		rank = rank * (sz + 1 - i) + ctr;
	}

	return rank;
}

static unsigned int left(unsigned int i) {
	return i * 2;
}

static unsigned int parent(unsigned int i) {
	return i / 2;
}

static bool odd(unsigned int i) {
	return i % 2 != 0;
}