#pragma once
#include "../utils/pool.hpp"
#include <limits>
#include <utility>

template<unsigned int K, class Data>
class Kdtree {
public:

	class N {
	public:
		double point[K];
		Data data;
	private:
		friend class Kdtree<K, Data>;
		unsigned int split;
		N *left, *right;
	};

	Kdtree();

	// Insert inserts a binding from the point to the data into the tree.
	void insert(double[K], Data);

	// Nearest returns a pointer to the nearest node in the tree, or NULL on error.
	const N* nearest(double[K]) const;

	// Size returns the number of points in the tree.
	unsigned long size() const;

private:

	N *insert(N*, unsigned int, N*);
	std::pair<const N*, double> nearest(const N*, double[], double*) const;
	double sqdist(const N*, double[]) const;

	N *root;
	unsigned long num;
	Pool<N> nodes;
};

template<unsigned int K, class Data>
Kdtree<K, Data>::Kdtree() : root(NULL), num(0) {
}

template<unsigned int K, class Data>
void Kdtree<K, Data>::insert(double pt[K], Data d) {
	N *n = nodes.construct();
	for (unsigned int i = 0; i < K; i++)
		n->point[i] = pt[i];
	n->data = d;
	root = insert(root, 0, n);
	num++;
}

template<unsigned int K, class Data>
typename Kdtree<K, Data>::N* Kdtree<K, Data>::insert(N *t, unsigned int depth, N *n) {
	if (!t) {
		n->split = depth % K;
		n->left = n->right = NULL;
		return n;
	}

	unsigned int s = t->split;
	if (n->point[s] < t->point[s])
		t->left = insert(t->left, depth+1, n);
	else
		t->right = insert(t->right, depth+1, n);
	return t;
}

template<unsigned int K, class Data>
const typename Kdtree<K, Data>::N* Kdtree<K, Data>::nearest(double pt[K]) const {
	double r = std::numeric_limits<double>::infinity();
	return nearest(root, pt, &r).first;
}

template<unsigned int K, class Data>
std::pair<const typename Kdtree<K, Data>::N*, double> Kdtree<K, Data>::nearest(const N *t, double pt[], double *range) const {
	if (!t)
		return std::make_pair<const N*, double>(NULL, std::numeric_limits<double>::infinity());

	unsigned int s = t->split;
	double diff = pt[s] - t->point[s];

	N *thisSide = t->right;
	N *otherSide = t->left;
	if (diff < 0) {
		thisSide = t->left;
		otherSide = t->right;
		diff = -diff;
	}

	auto near = nearest(thisSide, pt, range);

	if (diff*diff <= *range) {
		double d = sqdist(t, pt);
		if (d < *range) {
			near = std::make_pair(t, d);
			*range = std::min(near.second, *range);
		}

		auto m = nearest(otherSide, pt, range);
		if (m.second < near.second)
			near = m;
	}

	return near;
}

template<unsigned int K, class Data>
double Kdtree<K, Data>::sqdist(const N *n, double pt[]) const {
	if (!n)
		return std::numeric_limits<double>::infinity();

	double s = 0;
	for (unsigned int i = 0; i < K; i++) {
		double d = n->point[i] - pt[i];
		s += d*d;
	}
	return s;
}

template<unsigned int K, class Data>
unsigned long Kdtree<K, Data>::size() const {
	return num;
}