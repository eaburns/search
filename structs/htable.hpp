#include <cstring>
#include <cstdio>

void dfpair(FILE *, const char *key, const char *fmt, ...);

template <class Ops, class Key, class Elm> class Htable {
public:

	enum {
		Defsz = 1024,
		Growfact = 2,
	};

	static const float Fillfact = 1.5;

	Htable(unsigned int sz = Defsz)
		: fill(0), ncollide(0), nresize(0), nbins(0), bins(NULL) {
		resize(sz);
	}

	~Htable(void) {
		if (bins)
			delete[] bins;
	}

	void add(Elm *e) {
		add(e, Ops::hash(Ops::key(e)));
	}

	void add(Elm *e, unsigned long h) {
		if (fill * Fillfact >= nbins)
			resize(nbins == 0 ? Defsz : nbins * Growfact);

		add(bins, nbins, e, h);
		fill++;
	}

	Elm *find(Key k) {
		return find(k, Ops::hash(k));
	}

	Elm *find(Key k, unsigned long h) {
		unsigned int i = h % nbins;

		for (Elm *p = bins[i]; p; p = *Ops::nxt(p)) {
			if (Ops::eq(Ops::key(p), k))
				return p;
		}

		return NULL;
	}

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

private:

	void add(Elm *b[], unsigned int n, Elm *e, unsigned long h) {
		unsigned int i = h % n;
		if (b[i])
			ncollide++;
		*Ops::nxt(e) = b[i];
		b[i] = e;
	}

	void resize(unsigned int sz) {
		Elm **b = new Elm*[sz];

		for (unsigned int i = 0; i < sz; i++)
			b[i] = NULL;

		for (unsigned int i = 0; i < nbins; i++) {
		for (Elm *p = bins[i]; p; p = *Ops::nxt(p))
			add(b, sz, p, Ops::hash(Ops::key(p)));
		}

		if (bins)
			delete[] bins;

		bins = b;
		nbins = sz;
		nresize++;
	}

	friend bool htable_add_test(void);

	unsigned long fill, ncollide;
	unsigned int nresize, nbins;
	Elm **bins;
};