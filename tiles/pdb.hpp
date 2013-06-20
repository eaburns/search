// Copyright © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.
#include "../utils/utils.hpp"
#include "tiles.hpp"
#include <cmath>

struct Pdb {
	Pdb(unsigned long, unsigned int, const Tiles::Pos[]);

	virtual ~Pdb() { delete[] costs; }

	unsigned long numentries() const { return nents; }

	// poscost gets a pointer to the cost entry for the given
	// of pattern tile positions.
	unsigned char *poscost(const Tiles::Pos ps[]) {
		unsigned long ind = posind(ps);
		assert (ind < costssz);
		return costs + ind;
	}

	unsigned char *poscost(const unsigned char ps[]) {
		// convert character sized elements into Tiles::Pos.
		Tiles::Pos psbig[Tiles::Ntiles];
		for (unsigned int i = 0; i < patsz; i++)
			psbig[i] = ps[i];
		return poscost(psbig);
	}

protected:

	// posind computes an index into the costs array for
	// the given positions of the pattern tiles.
	virtual unsigned long posind(const Tiles::Pos ps[]) = 0;

	unsigned int patsz;
	Tiles::Tile pat[Tiles::Ntiles];

	unsigned long costssz;
	unsigned char *costs;

	unsigned long nents;
};

struct SparsePdb : public Pdb {
	SparsePdb(unsigned int, const Tiles::Pos[]);

	virtual unsigned long posind(const Tiles::Pos ps[]) {
		unsigned long ind = ps[0];
		unsigned int mul = Tiles::Ntiles;
	
		for (unsigned int i = 1; i < patsz; i++) {
			ind += ps[i] * mul;
			mul *= Tiles::Ntiles;
		}

		return ind;
	}
};

struct CompactPdb : public Pdb {
	CompactPdb(unsigned int, const Tiles::Pos[]);

	virtual unsigned long posind(const Tiles::Pos ps[]) {
		return ranker.rank(ps);
	}
	
private:
	Ranker ranker;
};
