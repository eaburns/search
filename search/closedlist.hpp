#pragma once

#include <cstdio>
#include <typeinfo>
#include <cstring>

void dfpair(FILE *, const char *key, const char *fmt, ...);	// utils.hpp

enum { FillFact = 0 };

template<typename Node, typename D> struct ClosedEntry {
	ClosedEntry() : nxt(NULL) { }
	Node *nxt;
};

template<typename Ops, typename Node, typename D> struct ClosedList {

	enum {
		Defsz = 1024,
		Growfact = 2,
		Fillfact = 3,
	};

	typedef typename D::PackedState PackedState;

	ClosedList(unsigned long szhint) :
			fill(0), ncollide(0), nresize(0), nbins(0), fills(NULL), bins(NULL) {
		resize(szhint);
		nresize = 0;	// don't count the initial resize
	}

	~ClosedList() {
		if (bins)
			delete []bins;
		if (fills)
			delete []fills;
	}

	void init(D &d) { dom = &d; }

	void clear() {
		fill = ncollide = 0;
		nresize = 0;
		for (unsigned int i = 0; i < nbins; i++) {
			bins[i] = NULL;
			fills[i] = 0;
		}
	}

	void add(Node *n) {
		add(n, Ops::key(n).hash(dom));
	}

	void add(Node *n, unsigned long h) {
		if (fill * Fillfact >= nbins)
			resize(nbins == 0 ? Defsz : nbins * Growfact);

		add(bins, fills, nbins, n, h);
		fill++;
	}

	Node *find(PackedState &k) {
		return find(k, k.hash(dom));
	}

	Node *find(PackedState &k, unsigned long h) {
		for (Node *p = bins[h % nbins]; p; p = Ops::closedentry(p).nxt) {
			if (Ops::key(p).eq(dom, k))
				return p;
		}

		return NULL;
	}

	void prstats(FILE *out, const char *prefix) {
		dfpair(out, "closed list type", "%s", "hash table");

		std::string key = prefix + std::string("fill");
		dfpair(out, key.c_str(), "%lu", fill);

		key = prefix + std::string("collisions");
		dfpair(out, key.c_str(), "%lu", ncollide);

		key = prefix + std::string("resizes");
		dfpair(out, key.c_str(), "%lu", nresize);

		key = prefix + std::string("buckets");
		dfpair(out, key.c_str(), "%lu", nbins);

		unsigned int m = 0;
		for (unsigned int i = 0; i < nbins; i++)
			m = std::max(m, fills[i]);
		key = prefix + std::string("max bucket fill");
		dfpair(out, key.c_str(), "%u", m);
	}
	
	Node* remove(PackedState &k) {
		return remove(k, k.hash(dom)); 
	}

	Node* remove(PackedState &k, unsigned long h) {
		unsigned int bindex = h % this->nbins;
		Node* prev = this->bins[bindex];
		Node *p = prev;
		for ( ; p; p = Ops::closedentry(p).nxt) {
			if (Ops::key(p).eq(dom, k)) break;
			prev = p;
		}

		if(!p) return NULL;

		this->fill--;
		if(prev == p) { //head of the list
			this->bins[bindex] = Ops::closedentry(p).nxt;
		}
		else { // middle or tail
			Ops::closedentry(prev).nxt = Ops::closedentry(p).nxt;
		}
		return p;
	}

	class iterator {
	public:
		static iterator begin(Node **bs, unsigned int nb) {
			iterator it(0, NULL);
			it.bins = bs;
			it.nbins = nb;

			if (!it.bins[0])
				it.nextbin();

			if (it.b < it.nbins)
				it.n = it.bins[it.b];

			return it;
		}

		static iterator end(unsigned int nb) {
			return iterator(nb, NULL);
		}

		bool operator==(const iterator &o) const {
			return n == o.n && b == o.b;
		}

		bool operator!=(const iterator &o) const {
			return !(*this == o);
		}

		Node *operator*() const {
			return n;
		}

		Node operator->() const {
			return *n;
		}

		void operator++() {
			if (b >= nbins)
				return;
			
			n = Ops::closedentry(n).nxt;
			if (n != NULL)
				return;

			nextbin();
			if (b < nbins)
				n = bins[b];
		}

	private:
		iterator(unsigned int bin, Node *nd) : n(nd), b(bin) {
		}

		void nextbin() {
			b++;
			while (b < nbins && !bins[b])
				b++;
		}

		Node *n;
		unsigned int b;

		Node **bins;
		unsigned int nbins;
	};

	iterator begin() {
		return iterator::begin(bins, nbins);
	}

	iterator end() {
		return iterator::end(nbins);
	}

	unsigned long getFill() const {
		return this->fill;
	}

	bool empty() const {
		return this->fill == 0;
	}

	void resize(unsigned int sz) {
		Node **b = new Node*[sz];
		unsigned int *f = new unsigned int[sz];

		for (unsigned int i = 0; i < sz; i++) {
			b[i] = NULL;
			f[i] = 0;
		}

		for (unsigned int i = 0; i < nbins; i++) {
			Node *nxt = NULL;
			for (Node *p = bins[i]; p; p = nxt) {
				nxt = Ops::closedentry(p).nxt;
				add(b, f, sz, p, Ops::key(p).hash(dom));
			}
		}

		if (bins)
			delete[] bins;
		if (fills)
			delete []fills;

		bins = b;
		fills = f;
		nbins = sz;
		nresize++;
	}

private:
	friend struct iterator;

	void add(Node *b[], unsigned int f[], unsigned int n, Node *e, unsigned long h) {
		unsigned int i = h % n;
		if (b[i])
			ncollide++;
		Ops::closedentry(e).nxt = b[i];
		b[i] = e;
		f[i]++;
	}

	D *dom;
	unsigned long fill, ncollide;
	unsigned int nresize, nbins;
	unsigned int *fills;
	Node** bins;
};
