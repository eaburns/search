#include "../utils/utils.hpp"
#include "kdtree.hpp"
#include <cstdlib>
#include <vector>

static const unsigned int K = 3;	// Dimensions
static const unsigned int N = 10;	// Number of insertions or whatever

static void pr(double[]);
static std::vector<unsigned int> nearest(double[][K], double[K]);
static double sqdist(double [], double[]);

bool kdtree_insert_test() {
	Kdtree<K, void*> root;
	for (unsigned int i = 0; i < N; i++) {
		double v[K];
		for (unsigned int i = 0; i < K; i++)
			v[i] = randgen.real();
		root.insert(v, NULL);
	}
	return root.size() == N;
}

bool kdtree_nearest_test() {
	Kdtree<K, unsigned int> root;
	double pts[N][K];

	for (unsigned int i = 0; i < N; i++) {
		for (unsigned int j = 0; j < K; j++)
			pts[i][j] = randgen.real();
	}

	for (unsigned int i = 0; i < N; i++)
		root.insert(pts[i], i);

	for (unsigned int i = 0; i < N; i++) {
		double pt[K];
		for (unsigned int j = 0; j < K; j++)
			pt[j] = randgen.real();
	
		auto n = root.nearest(pt);
		auto near = nearest(pts, pt);

		if (near.size() > 0 && !n) {
			testpr("Expected a near point, but got none");
			return false;
		}

		bool found = false;
		for (auto i : near) {
			if (n->data == i) {
				found = true;
				break;
			}
		}

		if (!found) {
			testpr("Found incorrect nearest:\n");
			testpr("point:\n\t");
			pr(pt);
			testpr("\npoints:\n");
			for (unsigned int i = 0; i < N; i++) {
				testpr("\t");
				pr(pts[i]);
				testpr("\n");
			}
			testpr("found:\n\t");
			pr(pts[n->data]);
			testpr(" (sqdist: %g)\n", sqdist(pts[n->data], pt));
			testpr("nearest:\n");
			for (auto i : near) {
				testpr("\t");
				pr(pts[i]);
				testpr(" (sqdist: %g)\n", sqdist(pts[i], pt));
			}
			return false;
		}
	}

	return true;
}

static void pr(double pt[]) {
	const char *prefix = "";
	for (unsigned int i = 0; i < K; i++) {
		testpr("%s%g", prefix, pt[i]);
		prefix = ", ";
	}
}

static std::vector<unsigned int> nearest(double pts[][K], double pt[K]) {

	std::vector<unsigned int> near;
	near.push_back(0);
	double dist = sqdist(pts[0], pt);

	for (unsigned int i = 1; i < N; i++) {
		double d = sqdist(pts[i], pt);
		if (d < dist) {
			near.clear();
			near.push_back(i);
			dist = d;
		} else if (d == dist) {
			near.push_back(i);
		}
	}

	return near;
}

static double sqdist(double a[], double b[]) {
	double s = 0;
	for (unsigned int i = 0; i < K; i++) {
		double d = a[i] - b[i];
		s += d*d;
	}
	return s;
}