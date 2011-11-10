#include "pdb.hpp"
#include <cmath>
#include <cstring>	// memset

Pdb::Pdb(unsigned int psz, const Tiles::Tile p[]) : patsz(psz), nents(0) {
	memcpy(pat, p, psz * sizeof(*p));
	for (unsigned int i = 0; i < psz; i++)
		nents *= Tiles::Ntiles - i;
}

SparsePdb::SparsePdb(unsigned int psz, const Tiles::Tile p[]) : Pdb(psz, p) {
	sz = pow(Tiles::Ntiles, patsz);
	costs = new unsigned char[sz];
	memset(costs, 0, sz * sizeof(*costs));
}

SparsePdb::~SparsePdb(void) {
	delete[] costs;
}

CompactPdb::CompactPdb(unsigned int psz, const Tiles::Tile p[]) :
		Pdb(psz, p), ranker(Tiles::Ntiles, psz) {
	costs = new unsigned char[nents];
	memset(costs, 0, nents * sizeof(*costs));
}

CompactPdb::~CompactPdb(void) {
	delete[] costs;
}