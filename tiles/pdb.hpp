#include "../utils/utils.hpp"
#include "tiles.hpp"

struct Pdb {
	Pdb(unsigned int, const Tiles::Tile[]);

	virtual ~Pdb(void) { }

	virtual unsigned char* cost(const Tiles::Pos[]) = 0;

	unsigned long numentries(void) const { return nents; }

protected:
	unsigned int patsz;
	Tiles::Tile pat[Tiles::Ntiles];
	unsigned long nents;
};

struct SparsePdb : public Pdb {
	SparsePdb(unsigned int, const Tiles::Tile[]);

	~SparsePdb(void);

	virtual unsigned char* cost(const Tiles::Pos ps[]) {
		unsigned long ind = 0;
		unsigned int mul = 1;
	
		for (unsigned int i = 0; i < patsz; i++) {
			ind += ps[i] * mul;
			mul *= Tiles::Ntiles;
		}
	
		assert (ind < sz);
		return costs + ind;
	}
	
private:
	unsigned long sz;
	unsigned char *costs;
};

struct CompactPdb : public Pdb {
	CompactPdb(unsigned int, const Tiles::Tile[]);

	~CompactPdb(void);

	virtual unsigned char* cost(const Tiles::Pos[]) {
		fatal("Unimplemented");
		return NULL;
	}
	
private:
	Ranker ranker;
	unsigned char *costs;
};