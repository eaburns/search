#include "../utils/utils.hpp"
#include "tiles.hpp"
#include <cmath>

struct Pdb {
	Pdb(unsigned long, unsigned int, const Tiles::Tile[]);

	virtual ~Pdb(void) { delete[] costs; }

	unsigned long numentries(void) const { return nents; }

protected:

	virtual unsigned long index(const Tiles::Pos ps[]) = 0;

	unsigned int patsz;
	Tiles::Tile pat[Tiles::Ntiles];

	unsigned long costssz;
	unsigned char *costs;

	unsigned long nents;
};

struct SparsePdb : public Pdb {
	SparsePdb(unsigned int, const Tiles::Tile[]);

	virtual unsigned long index(const Tiles::Pos ps[]) {
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
	CompactPdb(unsigned int, const Tiles::Tile[]);

	virtual unsigned long index(const Tiles::Pos ps[]) {
		return ranker.rank(ps);
	}
	
private:
	Ranker ranker;
};