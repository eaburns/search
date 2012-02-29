#ifndef _INTPQ_HPP_
#define _INTPQ_HPP_

#include <cassert>
#include <cstdlib>

template<class Elm> struct IntpqEntry {
	Elm *nxt, *prev;
	IntpqEntry(void) : prev(NULL) {}
};

template <class Ops, class Elm> class Intpq {
public:

	enum { Initsz = 1024 };

	Intpq(unsigned int sz = Initsz)
		: fill(0), nresize(0), end(0), nbins(0), bins(NULL) {
		resize(sz);
	}

	~Intpq(void) {
		if (bins)
			delete[] bins;
	}

	void push(Elm *e, unsigned int prio) {
		if (prio >= nbins)
			resize(prio == 0 ? Initsz : (prio + 1) * 1.5);
		if (bins[prio])
			Ops::entry(bins[prio]).prev = e;
		Ops::entry(e).nxt = bins[prio];
		Ops::entry(e).prev = e;
		bins[prio] = e;

		if (fill == 0 || prio < end)
			end = prio;

		fill++;
	}

	Elm *pop(void) {
		if (fill == 0)
			return NULL;

		for ( ; !bins[end]; end++)
			;
		assert(bins[end]);

		Elm *e = bins[end];
		IntpqEntry<Elm> &ent = Ops::entry(e);
		if (ent.nxt)
			Ops::entry(ent.nxt).prev = ent.nxt;
		bins[end] = ent.nxt;
		ent.prev = NULL;

		fill--;

		return e;
	}

	void rm(Elm *e, unsigned int prio) {
		IntpqEntry<Elm> &ent = Ops::entry(e);
		assert(mem(e));
		if (fst(e)) {
			if (ent.nxt)
				Ops::entry(ent.nxt).prev = ent.nxt;
			bins[prio] = ent.nxt;
		} else {
			assert (ent.prev != e);
			if (ent.nxt)
				Ops::entry(ent.nxt).prev = ent.prev;
			Ops::entry(ent.prev).nxt = ent.nxt;
		}
		ent.prev = NULL;
		fill--;
	}

	bool empty(void) {
		return fill == 0;
	}

	bool mem(Elm *e) {
		return Ops::entry(e).prev != NULL;
	}

	void clear(void) {
		fill = 0;
		nresize = end = 0;
		for (unsigned int i = 0; i < nbins; i++)
			bins[i] = NULL;
	}

private:

	bool fst(Elm *e) {
		return Ops::entry(e).prev == e;
	}

	void resize(unsigned int sz) {
		Elm **b = new Elm*[sz];

		for (unsigned int i = 0; i < nbins; i++)
			b[i] = bins[i];

		for (unsigned int i = nbins; i < sz; i++)
			b[i] = NULL;

		if (bins)
			delete[] bins;

		nbins = sz;
		bins = b;
		nresize++;
	}

	friend bool intpq_push_test(void);
	friend bool intpq_pop_test(void);

	unsigned long fill;
	unsigned int nresize, end, nbins;
	Elm **bins;
};

#endif	// _INTPQ_HPP_