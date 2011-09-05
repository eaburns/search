#include <cassert>
#include <vector>
#include <boost/optional.hpp>

template <class Elm> class Intpqmin {
public:

	Intpqmin() : min(0), fill(0) { }

	int add(Elm e, unsigned int p) {
		if (p >= bins.size())
			bins.resize(p*2);

		Bin &bin = bins[p];
		bin.push_back(e);

		if (fill == 0 || p < min)
			min = p;
		fill++;

		return bin.size() - 1;
 	}

	boost::optional<Elm> pop(void) {
		if (fill == 0)
			return boost::optional<Elm>();

		for ( ; min < bins.size() && bins[min].empty(); min++)
			;
		assert (min < bins.size());

		fill--;

		Bin &bin = bins[min];
		Elm res = bin.back();
		bin.pop_back();

		return boost::optional<Elm>(res);
	}

	bool empty(void) {
		return fill == 0;
	}

	boost::optional<Elm> rm(Elm e, unsigned int p, int ind = -1) {
		boost::optional<Elm> res;
		std::vector<Elm> &bin = bins[p];

		if (ind < 0) {
			for (ind = 0; ind < (int) bin.size() && bin[ind] != e; ind++)
				;
			if (bin[ind] != e)
				return res;
		} else {
			assert (bin[ind] == e);
		}

		if (ind < (int) bin.size() - 1) {
			bin[ind] = bin[bin.size() - 1];
			res = bin[ind];
		}
		bin.pop_back();

		fill--;

		return res;
	}

private:
	friend bool intpqmin_push_test(void);
	friend bool intpqmin_pop_test(void);

	unsigned int min;
	unsigned long fill;
	typedef std::vector<Elm> Bin;
	std::vector<Bin> bins;
};

template <class Elm> class Intpqmax {
public:

	Intpqmax() : max(0), fill(0) { }

	int add(Elm e, unsigned int p) {
		if (p >= bins.size())
			bins.resize(p*2);

		Bin &bin = bins[p];
		bin.push_back(e);

		if (fill == 0 || p > max)
			max = p;
		fill++;

		return bin.size() - 1;
 	}

	boost::optional<Elm> pop(void) {
		if (fill == 0)
			return boost::optional<Elm>();

		for ( ; max > 0 && bins[max].empty(); max--)
			;

		fill--;

		Bin &bin = bins[max];
		Elm res = bin.back();
		bin.pop_back();

		return boost::optional<Elm>(res);
	}

	bool empty(void) {
		return fill == 0;
	}

	boost::optional<Elm> rm(Elm e, unsigned int p, int ind = -1) {
		boost::optional<Elm> res;
		std::vector<Elm> &bin = bins[p];

		if (ind < 0) {
			for (ind = 0; ind < (int) bin.size() && bin[ind] != e; ind++)
				;
			if (bin[ind] != e)
				return res;
		} else {
			assert (bin[ind] == e);
		}

		if (ind < (int) bin.size() - 1) {
			bin[ind] = bin[bin.size() - 1];
			res = bin[ind];
		}
		bin.pop_back();

		fill--;

		return res;
	}

private:
	friend bool intpqmax_push_test(void);
	friend bool intpqmax_pop_test(void);

	unsigned int max;
	unsigned long fill;
	typedef std::vector<Elm> Bin;
	std::vector<Bin> bins;
};