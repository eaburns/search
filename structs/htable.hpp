// Copyright © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.
#pragma once

#include <cstring>
#include <cstdio>

void dfpair(FILE *, const char *key, const char *fmt, ...);

template <class Elm> struct HtableEntry {
	Elm *nxt;
};

template <class Ops, class Key, class Elm,int Fillfact=2> class Htable {
public:

	enum {
		Defsz = 1024,
		Growfact = 2,
	};

	Htable(unsigned int sz = Defsz)
		: fill(0), ncollide(0), nbins(0), bins(NULL) {
		resize(sz);
		nresize = 0;	// don't count the initial resize
	}

	~Htable() {
		if (bins)
			delete[] bins;
	}

	// add adds an element to the hash table.
	void add(Elm *e) {
		add(e, Ops::hash(Ops::key(e)));
	}

	// Just like the above add method, however,
	// it accepts a pre-computed hash value.
	void add(Elm *e, unsigned long h) {
		if (fill * Fillfact >= nbins)
			resize(nbins == 0 ? Defsz : nbins * Growfact);

		add(bins, nbins, e, h);
		fill++;
	}

	// find returns the first element that has the
	// give key, or NULL if no elements with the
	// key are in the table.
	Elm *find(Key k) {
		return find(k, Ops::hash(k));
	}

	// Just like the previous find method, however,
	// it accepts a pre-computed hash value.
	Elm *find(Key k, unsigned long h) {
		for (Elm *p = bins[h % nbins]; p; p = Ops::entry(p).nxt) {
			if (Ops::key(p) == k)
				return p;
		}

		return NULL;
	}

	// rm attempts to remove one an element from
	// the hash table.  If rm returns true then one
	// element with the given key is removed, if rm
	// returns false than there were no items with
	// the key.
	bool rm(Key k) {
		return rm(k, Ops::hash(k));
	}

	// Just like the above rm method, however, it
	// accepts a pre-computed hash value.
	bool rm(Key k, unsigned long h) {
		unsigned long i = h % nbins;

		Elm *q = NULL, *p;
		for (p = bins[i]; p; p = Ops::entry(p).nxt) {
			if (Ops::key(p) == k)
				break;
			q = p;
		}

		if (p == NULL)	// not found
			return false;

		fill--;

		if (q == NULL)	// head of list
			bins[i] = Ops::entry(p).nxt;
		else
			q = Ops::entry(p).nxt;
		return true;
	}

	// prstats prints statistics about the hash table.
	void prstats(FILE *f, const char *prefix) {
		char key[strlen(prefix) + strlen("collisions") + 1];
		strcpy(key, prefix);

		strcat(key+strlen(prefix), "fill");
		dfpair(f, key, "%lu", fill);

		strcpy(key+strlen(prefix), "collisions");
		dfpair(f, key, "%lu", ncollide);

		strcpy(key+strlen(prefix), "resizes");
		dfpair(f, key, "%lu", nresize);
	}

	// clear removes all elements from the hash table
	// but does not free any memory, leaving the
	// hash table ready to be re-used.
	void clear() {
		fill = ncollide = 0;
		nresize = 0;
		for (unsigned int i = 0; i < nbins; i++)
			bins[i] = NULL;
	}

private:

	void add(Elm *b[], unsigned int n, Elm *e, unsigned long h) {
		unsigned int i = h % n;
		if (b[i])
			ncollide++;
		Ops::entry(e).nxt = b[i];
		b[i] = e;
	}

	void resize(unsigned int sz) {
		Elm **b = new Elm*[sz];

		for (unsigned int i = 0; i < sz; i++)
			b[i] = NULL;

		for (unsigned int i = 0; i < nbins; i++) {
			Elm *nxt = NULL;
			for (Elm *p = bins[i]; p; p = nxt) {
				nxt = Ops::entry(p).nxt;
				add(b, sz, p, Ops::hash(Ops::key(p)));
			}
		}

		if (bins)
			delete[] bins;

		bins = b;
		nbins = sz;
		nresize++;
	}

	friend bool htable_add_test();
	friend bool htable_rm_test();

	unsigned long fill, ncollide;
	unsigned int nresize, nbins;
	Elm **bins;
};
