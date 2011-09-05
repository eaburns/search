#include <cassert>
#include <cstdlib>

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

		Elm **nxt =Ops::nxt(bins + prio);
		*Ops::nxt(e) = *nxt;
		*Ops::prev(*nxt) = e;
		*Ops::nxt(bins + prio) = e;
		*Ops::prev(e) = bins + prio;

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

		Elm *e = *Ops::nxt(bins + end);
		rm(e);
		return e;
	}

	void rm(Elm *e) {
		Elm **nxt = Ops::nxt(e);
		Elm **prev = Ops::prev(e);
		*Ops::prev(*nxt) = *prev;
		*Ops::nxt(*prev) = *nxt;

		*Ops::nxt(e) = NULL;
		*Ops::prev(e) = NULL;

		fill--;
	}

	bool empty(void) {
		return fill == 0;
	}

	bool mem(Elm *e) {
		Elm *nxt = *Ops::nxt(e);
		return nxt == NULL;
	}

private:

	bool empty(unsigned int p) {
		return *Ops::nxt(bins + p) == bins + p;
	}

	void resize(unsigned int sz) {
		Elm *b = (Elm*) malloc(sz * sizeof(*b));

		for (unsigned int i = 0; i < nbins; i++) {
			if (empty(i)) {
				*Ops::prev(b + i) = b + i;
				*Ops::nxt(b + i) = b + i;
			} else {
				Elm **nxt = Ops::nxt(bins + i);
				Elm **prev = Ops::prev(bins + i);
				*Ops::nxt(b + i) = *nxt;
				*Ops::prev(*nxt) = b + i;
				*Ops::nxt(*prev) = b + i;
				*Ops::prev(b + i) = *prev;
			}
		}

		for (unsigned int i = nbins; i < sz; i++) {
			*Ops::prev(b + i) = b + i;
			*Ops::nxt(b + i) = b + i;
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