#include "pdb.hpp"
#include <cstring>	// memset

// 'x to the n falling'
static unsigned long fallfact(unsigned int x, unsigned int n) {
	unsigned long f = x;
	for (unsigned int i = 1; i < n; i++)
		f *= x - i;
	return f;
}

Pdb::Pdb(unsigned long csz, unsigned int psz, const Tiles::Tile p[]) :
		patsz(psz), costssz(csz), costs(NULL),
		nents(fallfact(Tiles::Ntiles, patsz)) {
	memcpy(pat, p, psz * sizeof(*p));
	costs = new char[costssz];
	memset(costs, -1, costssz * sizeof(*costs));
}

SparsePdb::SparsePdb(unsigned int psz, const Tiles::Tile p[]) :
	Pdb(pow(Tiles::Ntiles, psz), psz, p) { }

CompactPdb::CompactPdb(unsigned int psz, const Tiles::Tile p[]) :
	Pdb(fallfact(Tiles::Ntiles, psz), psz, p),
	ranker(Tiles::Ntiles, psz) { }