#include "anyprof.hpp"
#include "../utils/utils.hpp"
#include <cmath>
#include <cstring>
#include <limits>

AnyProf::AnyProf(unsigned int cbs, double cm, unsigned int tbs, double tm) :
		cbins(cbs), tbins(tbs), cmax(cm), tmax(tm),
		cwidth(cmax/(cbins-1)), twidth(tmax/(tbins-1)) {

	assert (floor(cmax/cwidth) == cbins-1);
	assert (floor(tmax/twidth) == tbins-1);

	bins.resize(cbins);
	for (unsigned int i = 0; i < cbins; i++) {
		bins[i].resize(tbins);
		for (unsigned int j = 0; j < cbins; j++)
			bins[i][j].resize(cbins, 0);
	}
}

void AnyProf::read(FILE *in) {
	if (fscanf(in, "cmax: %lg\n", &cmax) != 1 ||
		fscanf(in, "tmax: %lg\n", &tmax) != 1 ||
		fscanf(in, "cbins: %u\n", &cbins) != 1 ||
		fscanf(in, "tbins: %u\n", &tbins) != 1)
		fatal("failed to read anytime profile");

	cwidth = cmax/(cbins-1);
	twidth = tmax/(tbins-1);

	bins.resize(cbins);
	for (unsigned int i = 0; i < cbins; i++) {
		bins[i].resize(tbins);
		for (unsigned int j = 0; j < cbins; j++) {
			bins[i][j].resize(cbins);
			for (unsigned int k = 0; k < cbins; k++) {
				if (fscanf(in, " %lg", &bins[i][j][k]) == 1)
					continue;
				fatal("failed to read the anytime profile");
			}
		}
	}
}

void AnyProf::write(FILE *out) const {
	if (fprintf(out, "cmax: %g\n", cmax) < 0 ||
			fprintf(out, "tmax: %g\n", tmax) < 0 ||
			fprintf(out, "cbins: %u\n", cbins) < 0 ||
			fprintf(out, "tbins: %u\n", tbins) < 0)
		fatal("failed to write anytime profile");

	for (unsigned int i = 0; i < cbins; i++) {
	for (unsigned int j = 0; j < tbins; j++) {
	for (unsigned int k = 0; k < cbins; k++) {
		if (fprintf(out, " %g", bins[i][j][k]) < 0)
			fatal("failed to write the anytime profile");
	}
	}
	}
}

MonPolicy::MonPolicy(const AnyProf &p, double wc, double wt) :
		prof(p), wcost(wt), wtime(wt) {

	static const double Inifinity = std::numeric_limits<double>::infinity();

	ents.resize(prof.cbins);
	for (unsigned int ci = 0; ci < prof.cbins; ci++) {
		ents[ci].resize(prof.tbins);
		double time = (prof.tbins-1)*prof.twidth;
		double cost = ci*prof.cwidth;
		double u = -(wcost*cost + wtime*time);
		ents[ci][prof.tbins-1] = Entry(u, 0, true);
	}


	// v(c_i, t_i) = max_{dt, stop} (
	// 	sum_j pr(c_j, | c_i, dt) * u(c_j, t_i + dt)		if stop
	// 	sum_j pr(c_j, | c_i, dt) * v(c_j, t_i + dt) + C	otherwise)
	for (int ti = prof.tbins - 2; ti >= 0; ti--) {
	for (unsigned int ci = 0; ci < prof.cbins; ci++) {
		double max = -Inifinity;
		bool smax = true;
		double dmax = 0;

		for (unsigned int tj = ti+1; tj < prof.tbins; tj++) {
			double time = tj*prof.twidth;
			double dt = time - ti*prof.twidth;
			unsigned int dti = dt/prof.twidth;

			double sum = 0;
			for (unsigned int cj = 0; cj < prof.cbins; cj++) {
				double cost = cj*prof.cwidth;
				double u = -(wcost*cost + wtime*time);
				sum += prof.bins[ci][dti][cj] * u;
			}
			if (sum > max) {
				max = sum;
				smax = true;
				dmax = dti*prof.twidth;
			}

			sum = 0;
			for (unsigned int cj = 0; cj < prof.cbins; cj++) {
				assert (ents[cj][tj].set);
				double v = ents[cj][tj].value;
				sum += prof.bins[ci][dti][cj] * v;
			}
			if (sum > max) {
				max = sum;
				smax = false;
				dmax = dti*prof.twidth;
			}

			ents[ci][ti] = Entry(max, dmax, smax);
		}
	}
	}
}