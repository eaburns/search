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

	~ClosedList() {
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
	
	Node* remove(PackedState &k) {
		return remove(k, this->dom->hash(k)); 
	}

	Node* remove(PackedState &k, unsigned long h) {
		unsigned int bindex = h % this->nbins;
		Node* prev = this->bins[bindex];
		Node *p = prev;
		for ( ; p; p = Ops::closedentry(p).nxt) {
			if (Ops::key(p) == k) break;
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

	struct iterator {
		iterator() : done(true), bin(-1), elem(NULL), list(NULL) {}
		iterator(Node* elem, int bin, const ClosedList* list)  : done(false), bin(bin), elem(elem), list(list) {}

		Node* next() {
			if(done ||  bin < 0 || elem == NULL) return NULL;

			Node* retVal = elem;

			Node* p = Ops::closedentry(elem).nxt;
			if(p) {
				elem = p;
				return elem;
			}

			for(unsigned int i = bin+1; i < list->nbins; i++) {
				if(list->bins[i] != NULL) {
					bin = i;
					elem = list->bins[i];
					return retVal;
				}
			}

			done = true;
			elem = NULL;
			return retVal;
		}
	
		private: 
			bool done;
			int bin;
			Node* elem;
			const ClosedList *list;
	};

	

	iterator begin() const {
		for(unsigned int i = 0; i < nbins; i++)
			if(bins[i] != NULL)
				return iterator(bins[i], i, this);
	
		return iterator();
	}

	void advanceIterator(iterator* iter) const {

	}

	void destroyIterator(iterator* iter) const { if(iter) delete iter; }

	unsigned long getFill() const { return this->fill; }

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

private:
	friend struct iterator;

	void add(Node *b[], unsigned int n, Node *e, unsigned long h) {
		unsigned int i = h % n;
		if (b[i])
			ncollide++;
		Ops::closedentry(e).nxt = b[i];
		b[i] = e;
	}

	D *dom;
	unsigned long fill, ncollide;
	unsigned int nresize, nbins;
	Node** bins;
};
