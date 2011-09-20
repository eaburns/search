#include "mdlearn.hpp"
#include <cstdlib>
#include <cstring>

TilesMDLearn::TilesMDLearn(FILE *in) : TilesMdist(in) {
	initops();
}

void TilesMDLearn::initops(void) {
	for (int i = 0; i < Ntiles; i++) {
		ops[i].n = 0;
		memset(ops[i].dests, 0, sizeof(ops[i].dests));
		if (i >= Width)
			ops[i].mvs[ops[i].n++].b = i - Width;
		if (i % Width > 0)
			ops[i].mvs[ops[i].n++].b = i - 1;
		if (i % Width < Width - 1)
			ops[i].mvs[ops[i].n++].b = i + 1;
		if (i < Ntiles - Width)
			ops[i].mvs[ops[i].n++].b = i + Width;
	}
	initdests();
}

void TilesMDLearn::initdests(void) {
	for (unsigned int i = 0; i < Ntiles; i++) {
		for (unsigned int j = 0; j < ops[i].n; j++) {
			Pos b = ops[i].mvs[j].b;
			ops[i].dests[b] = ops[i].mvs + j;
		}
	}
}

void TilesMDLearn::resetprobs(void) {
	for (unsigned int i = 0; i < Ntiles; i++) {
		for (unsigned int j = 0; j < ops[i].n; j++) {
			ops[i].mvs[j].nused = 0;
			ops[i].mvs[j].ndec = 0;
		}
	}
}

int cmpmv(const void *_a, const void *_b) {
	const TilesMDLearn::Opinfo *a = (const TilesMDLearn::Opinfo *) _a;
	const TilesMDLearn::Opinfo *b = (const TilesMDLearn::Opinfo *) _b;
	double aprob = (double) a->ndec / a->nused;
	double bprob = (double) b->ndec / b->nused;
	if (aprob > bprob)
		return -1;
	else if (aprob < bprob)
		return 1;
	return 0;
}

void TilesMDLearn::sortops(void) {
	for (int i = 0; i < Ntiles; i++)
		qsort(ops[i].mvs, ops[i].n, sizeof(ops[i].mvs[0]), cmpmv);
}