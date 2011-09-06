#include <cassert>
#include <cstdlib>

template <class Elm> struct IntpqEnt {
	Elm *nxt, *prev;
	IntpqEnt(void) : nxt(NULL), prev(NULL) {}
};

template <class Ops, class Elm> class Intpq {
public:

	enum { Defsz = 64 };

	Intpq(unsigned int sz = Defsz)
		: fill(0), nresize(0), end(0), nbins(0), bins(NULL) {
		resize(sz);
	}

	~Intpq(void) {
		if (bins)
			free(bins);
	}

	void push(Elm *e, unsigned int prio) {
		if (prio > nbins)
			resize(prio == 0 ? Defsz : (prio + 1) * 1.5);

		IntpqEnt<Elm> &bin = Ops::entry(bins + prio);
		IntpqEnt<Elm> &ent = Ops::entry(e);
		ent.nxt = bin.nxt;
		Ops::entry(bin.nxt).prev = e;
		bin.nxt = e;
		ent.prev = bins + prio;

		if (fill == 0 || prio < end)
			end = prio;

		fill++;
	}

	Elm *pop(void) {
		if (fill == 0)
			return NULL;

		for ( ; empty(end); end++)
			;
		assert(!empty(end));

		Elm *e = Ops::entry(bins + end).nxt;
		rm(e);
		return e;
	}

	void rm(Elm *e) {
		IntpqEnt<Elm> &ent = Ops::entry(e);
		Ops::entry(ent.nxt).prev = ent.prev;
		Ops::entry(ent.prev).nxt = ent.nxt;
		ent.nxt = NULL;
		ent.prev = NULL;

		fill--;
	}

	bool empty(void) {
		return fill == 0;
	}

	bool mem(Elm *e) {
		return Ops::entry(e).nxt == NULL;
	}

private:

	bool empty(unsigned int p) {
		return Ops::entry(bins + p).nxt == bins + p;
	}

	void resize(unsigned int sz) {
		Elm *b = (Elm*) malloc(sz * sizeof(*b));

		for (unsigned int i = 0; i < nbins; i++) {
			if (empty(i)) {
				Ops::entry(b + i).prev = b + i;
				Ops::entry(b + i).nxt = b + i;
			} else {
				IntpqEnt<Elm> &bin = Ops::entry(bins + i);
				IntpqEnt<Elm> &nw = Ops::entry(b + i);
				nw.nxt = bin.nxt;
				Ops::entry(bin.nxt).prev = b + i;
				Ops::entry(bin.prev).nxt = b + i;
				nw.prev = bin.prev;
			}
		}

		for (unsigned int i = nbins; i < sz; i++) {
			Ops::entry(b + i).prev = b + i;
			Ops::entry(b + i).nxt = b + i;
		}

		if (bins)
			free(bins);

		nbins = sz;
		bins = b;
		nresize++;
	}

	friend bool intpq_push_test(void);
	friend bool intpq_pop_test(void);

	unsigned long fill;
	unsigned int nresize, end, nbins;
	Elm *bins;
};