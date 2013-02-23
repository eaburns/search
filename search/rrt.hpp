#pragma once
#include "../search/search.hpp"
#include "../structs/kdtree.hpp"
#include "../utils/pool.hpp"
#include <set>

template <class D> class RRT : public SearchAlgorithm<D> {
public:
	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Cost Cost;
	typedef typename D::Edge Edge;
	typedef typename D::Oper Oper;

	static const unsigned int K = D::State::K;
	
	struct Node {
		Node *parent;
		PackedState state;
		Oper op;
		std::set<Oper> ops;	// ops added to the tree
	};

	RRT(int argc, const char *argv[]) : SearchAlgorithm<D>(argc, argv), rng(0) {
		seed = randgen.bits();
		for (int i = 0; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-s") == 0) {
				char *end = NULL;
				seed = strtoull(argv[i++], &end, 10);
				if (end == NULL)
					fatal("Invalid seed: %s", argv[i-1]);
			}
		}
		rng = Rand(seed);
	}
	~RRT() {
	}

	void search(D &d, typename D::State &s0) {
		this->start();

		Node *root = pool.construct();
		root->parent = NULL;
		d.pack(root->state, s0);

		double pt[K];
		s0.vector(&d, pt);
		tree.insert(pt, root);

		for ( ; ; ) {
			sample(pt);
			auto near = tree.nearest(pt);
			Node *node = near->data;
			State buf, &state = d.unpack(buf, node->state);
				
			SearchAlgorithm<D>::res.expd++;
	
			Node *kid = pool.construct();
			kid->parent = node;
			double bestpt[K];
			double dist = std::numeric_limits<double>::infinity();

			typename D::Operators ops(d, state);
			for (unsigned int i = 0; i < ops.size(); i++) {
				SearchAlgorithm<D>::res.gend++;
				if (node->ops.find(ops[i]) != node->ops.end())
					continue;

				Edge e(d, state, ops[i]);

				if (d.isgoal(e.state)) {
					d.pack(kid->state, e.state);
					kid->op = ops[i];
					solpath<D, Node>(d, kid, this->res);
					goto done;
				}

				double kidpt[K];
				e.state.vector(&d, kidpt);
				
				double dst = sqdist(pt, kidpt);
				if (dst < dist) {
					dist = dst;
					for (unsigned int j = 0; j < K; j++)
						bestpt[j] = kidpt[j];
					d.pack(kid->state, e.state);
					kid->op = ops[i];
				}
			}
			if (std::isinf(dist)) {
				pool.destruct(kid);
				continue;
			}

			node->ops.insert(kid->op);
			tree.insert(bestpt, kid);
		}

done:

		this->finish();
	}

	virtual void reset() {
		SearchAlgorithm<D>::reset();
		pool.releaseall();
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
		dfpair(stdout, "node size", "%u", sizeof(Node));
		dfpair(stdout, "RRT seed", "%lu", (unsigned long) seed);
		dfpair(stdout, "KD-tree size", "%lu", (unsigned long) tree.size());
		dfpair(stdout, "KD-tree depth", "%lu", (unsigned long) tree.depth());

	}

private:

	double sqdist(double a[], double b[]) {
		double s = 0;
		for (unsigned int i = 0; i < K; i++) {
			double d = a[i] - b[i];
			s += d*d;
		}
		return s;
	}

	void sample(double s[]) {
		for (unsigned int i = 0; i < K; i++)
			s[i] = rng.real();
	}

	Rand rng;
	uint64_t seed;
	Kdtree<K, Node*> tree;
	Pool<Node> pool;
};
