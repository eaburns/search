// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "pdb.hpp"
#include <cstring>	// memset

Pdb::Pdb(unsigned long csz, unsigned int psz, const Tiles::Tile p[]) :
		patsz(psz), costssz(csz), costs(NULL),
		nents(fallfact(Tiles::Ntiles, patsz)) {
	memcpy(pat, p, psz * sizeof(*p));
	costs = new unsigned char[costssz];
	printf("%lu cost entries\n", costssz);
	memset(costs, 0, costssz * sizeof(*costs));
}

SparsePdb::SparsePdb(unsigned int psz, const Tiles::Tile p[]) :
	Pdb(pow((double) Tiles::Ntiles, psz), psz, p) { }

CompactPdb::CompactPdb(unsigned int psz, const Tiles::Tile p[]) :
	Pdb(fallfact(Tiles::Ntiles, psz), psz, p),
	ranker(Tiles::Ntiles, psz) { }
