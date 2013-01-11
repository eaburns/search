#pragma once

#include <cstdio>
#include <typeinfo>
#include <cstring>

void dfpair(FILE *, const char *key, const char *fmt, ...);	// utils.hpp

enum { FillFact = 0 };

template<typename Node, typename D> struct ClosedEntry {
	Node *nxt;
};

template<typename Ops, typename Node, typename D> struct ClosedList {

	enum {
		Defsz = 1024,
		Growfact = 2,
		Fillfact = 0,
	};

	typedef typename D::PackedState PackedState;

	ClosedList(unsigned long szhint) :
			fill(0), ncollide(0), nresize(0), nbins(0), bins(NULL) {
		resize(szhint);
		nresize = 0;	// don't count the initial resize
	}

	virtual ~ClosedList() {
		if (bins)
			delete []bins;
	}

	void init(D &d) { dom = &d; }

	void clear() {
		fill = ncollide = 0;
		nresize = 0;
		for (unsigned int i = 0; i < nbins; i++)
			bins[i] = NULL;
	}

	void add(Node *n) {
		add(n, dom->hash(Ops::key(n)));
	}

	void add(Node *n, unsigned long h) {
		if (fill * Fillfact >= nbins)
			resize(nbins == 0 ? Defsz : nbins * Growfact);

		add(bins, nbins, n, h);
		fill++;
	}

	Node *find(PackedState &k) {
		return find(k, dom->hash(k));
	}

	Node *find(PackedState &k, unsigned long h) {
		for (Node *p = bins[h % nbins]; p; p = Ops::closedentry(p).nxt) {
			if (Ops::key(p) == k)
				return p;
		}

		return NULL;
	}

	void prstats(FILE *out, const char *prefix) {
		dfpair(out, "closed list type", "%s", "hash table");

		char key[strlen(prefix) + strlen("collisions") + 1];
		strcpy(key, prefix);

		strcat(key+strlen(prefix), "fill");
		dfpair(out, key, "%lu", fill);

		strcpy(key+strlen(prefix), "collisions");
		dfpair(out, key, "%lu", ncollide);

		strcpy(key+strlen(prefix), "resizes");
		dfpair(out, key, "%lu", nresize);

		strcpy(key+strlen(prefix), "buckets");
		dfpair(out, key, "%lu", nbins);
	}

protected:

	void add(Node *b[], unsigned int n, Node *e, unsigned long h) {
		unsigned int i = h % n;
		if (b[i])
			ncollide++;
		Ops::closedentry(e).nxt = b[i];
		b[i] = e;
	}

	void resize(unsigned int sz) {
		Node **b = new Node*[sz];

		for (unsigned int i = 0; i < sz; i++)
			b[i] = NULL;

		for (unsigned int i = 0; i < nbins; i++) {
		for (Node *p = bins[i]; p; p = Ops::closedentry(p).nxt)
			add(b, sz, p, dom->hash(Ops::key(p)));
		}

		if (bins)
			delete[] bins;

		bins = b;
		nbins = sz;
		nresize++;
	}

	D *dom;
	unsigned long fill, ncollide;
	unsigned int nresize, nbins;
	Node** bins;
};
