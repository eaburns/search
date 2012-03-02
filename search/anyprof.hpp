#include <vector>
#include <cassert>

// Imp holds information about a solution improvement
// such as the initial cost, the new cost and the time delta
// between them.
struct Imp {
	Imp(double _c0, double _c1, double _dt) : c0(_c0), c1(_c1), dt(_dt) { }
	Imp(void) { }
	double c0, c1, dt;
};

// AnyProf holds an anytime algorithm's performance profile.
struct AnyProf {

	AnyProf(const std::vector<Imp>&, int, int);

	~AnyProf(void);

private:

	void maxes(const std::vector<Imp>&);

	// index returns the index of the probability entry
	// for the change from c0 to c1 in time dt.
	int index(double c0, double c1, double dt) const {
		double c0bin = c0 / cwidth;
		double dtbin = dt / twidth;
		double c1bin = c1 / cwidth;
		int i = c0bin*tbins*cbins + dtbin*cbins + c1bin;
		assert (i < sz);
		assert (i >= 0);
		return i;
	}

	double cmax, tmax;	// maximum cost and delta time
	double cwidth, twidth;	// width of each cost and delta time bin
	int cbins, tbins;	// number of cost and delta time bins
	
	int sz;
	double *prof;
};