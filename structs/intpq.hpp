#include <cassert>
#include <vector>
#include <boost/optional.hpp>

template <class Ops, class Elm> class Intpq {
public:

	Intpq() : min(0), fill(0) { }

	void push(Elm e) {
		unsigned int p = Ops::prio(e);

		if (p >= bins.size())
			bins.resize(p+1);

		Ops::setind(e, bins[p].size());
		bins[p].push_back(e);

		if (p < min)
			min = p;
		fill++;
	}

	boost::optional<Elm> pop(void) {
		if (fill == 0)
			return boost::optional<Elm>();

		unsigned int i;
		for (i = 0; i < bins.size() && bins[i].empty(); i++)
			;
		assert (i >= min);
		assert (i < bins.size());
		for ( ; min < bins.size() && bins[min].empty(); min++)
			;
		assert (min < bins.size());

		fill--;

		Elm res = bins[min].back();
		bins[min].pop_back();
		Ops::setind(res, -1);
		return boost::optional<Elm>(res);
	}

	bool empty(void) {
		return fill == 0;
	}

	bool rm(Elm e) {
		std::vector<Elm> bin = bins[Ops::prio(e)];

		unsigned int i = Ops::getind(e);
		if (i < 0) {
			for (i = 0; i < bin.size() && bin[i] != e; i++)
				;
			if (bin[i] != e)
				return false;
		} else {
			assert (bin[i] == e);
		}

		if (i < bin.size() - 1)
			bin[i] = bin[bin.size() - 1];
		bin.pop_back();

		fill--;

		return true;
	}

private:
	friend bool intpq_push_test(void);
	friend bool intpq_pop_test(void);

	unsigned int min;
	unsigned long fill;
	std::vector< std::vector<Elm> >bins;
};