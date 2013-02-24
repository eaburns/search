#pragma once
#include "../utils/pool.hpp"
#include <limits>
#include <utility>
#include <stack>

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

	// Depth returns  the maximum depth of any branch of the tree.
	// This is an O(n) routine.
	unsigned int depth() const;

	// Size returns the number of points in the tree.
	unsigned long size() const;

	class iterator {
	public:

		N* operator*() const {
			return nodes.top();
		}

		N operator->() const {
			return *nodes.top();
		}

		void operator++() {
			N *n = nodes.top();
			nodes.pop();
			if (n->right)
				nodes.push(n->right);
			if (n->left)
				nodes.push(n->left);
		}

		bool operator==(const iterator &o) const {
			return nodes == o.nodes;
		}

		bool operator!=(const iterator &o) const {
			return nodes != o.nodes;
		}

	private:
		friend class Kdtree<K, Data>;

		iterator() { }

		std::stack<N*> nodes;
	};

	iterator begin() {
		iterator it;
		if (root)
			it.nodes.push(root);
		return it;
	}

	iterator end() {
		return iterator();
	}

private:

	N *insert(N*, unsigned int, N*);
	const N* nearest(const N*, const double[], double*) const;
	unsigned int depth(const N*) const;
	inline double sqdist(const N*, const double[]) const;

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
	return nearest(root, pt, &r);
}

template<unsigned int K, class Data>
const typename Kdtree<K, Data>::N* Kdtree<K, Data>::nearest(const N *t, const double pt[], double *range) const {
	if (!t)
		return NULL;

	double diff = pt[t->split] - t->point[t->split];

	const N *thisSide, *otherSide;
	if (diff < 0) {
		thisSide = t->left;
		otherSide = t->right;
		diff = -diff;
	} else {
		thisSide = t->right;
		otherSide = t->left;
	}

	const N* n = nearest(thisSide, pt, range);

	if (diff*diff > *range)
		return n;

	double d = sqdist(t, pt);
	if (d <= *range) {
		n = t;
		*range = std::min(d, *range);
	}

	const N *m = nearest(otherSide, pt, range);
	if (m)
		n = m;

	return n;
}

template<unsigned int K, class Data>
double Kdtree<K, Data>::sqdist(const N *n, const double pt[]) const {
	static const double Inf = std::numeric_limits<double>::infinity();

	if (!n)
		return Inf;

	double s = 0;
	for (unsigned int i = 0; i < K; i++) {
		double d = n->point[i] - pt[i];
		s += d*d;
	}
	return s;
}

template<unsigned int K, class Data>
unsigned int Kdtree<K, Data>::depth() const {
	return depth(root);
}

template<unsigned int K, class Data>
unsigned int Kdtree<K, Data>::depth(const N *n) const {
	if (!n)
		return 0;
	return std::max(depth(n->left), depth(n->right)) + 1;
}

template<unsigned int K, class Data>
unsigned long Kdtree<K, Data>::size() const {
	return num;
}