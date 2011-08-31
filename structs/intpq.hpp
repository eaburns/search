#include <cassert>
#include <vector>

template <class Ops, class Elm> class Intpq {
public:

	Intpq() : min(0), fill(0) { }

	void push(Elm e) {
		unsigned int p = Ops::prio(e);

		bins.reserve(p);
		bins[p].push_back(e);

		if (p < min)
			min = p;
		fill++;
	}

	Elm pop(void) {
		if (fill == 0)
			return NULL;

		for ( ; min < bins.size() && !bins[min].empty(); min++)
			;
		assert (min < bins.size());

		fill--;

		Elm res = bins[min].back();
		bins[min].pop_back();
		return res;
	}

	bool empty(void) {
		return fill == 0;
	}

	bool rm(Elm e) {
		std::vector<Elm> bin = bins[Ops::prio(e)];

		int i;
		for (i = 0; i < bin.size() && bin[i] != e; i++)
			;

		if (bin[i] != e)
			return false;

		if (i < bin.size() - 1)
			bin[i] = bin[bin.size() - 1];
		bin.pop_back();

		return true;
	}

private:
	unsigned int min;
	unsigned long fill;
	std::vector< std::vector<Elm> >bins;
};