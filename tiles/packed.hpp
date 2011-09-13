#include <cassert>
#include <boost/cstdint.hpp>
#include <boost/integer/static_log2.hpp>

class Tiles;

template<int Ntiles> class PackedTiles {
	friend class Tiles;
	static const unsigned int nbits = boost::static_log2<Ntiles>::value;
	static const unsigned int nbytes = (int) ((nbits / 8.0) * Ntiles);

	unsigned char bytes[nbytes];
public:
	void pack(Tiles::Tile ts[]) {
		assert("Unimplemented" == 0);
	}

	Tiles::Pos unpack(Tiles::Tile ts[]) {
		assert("Unimplemented" == 0);
	}

	Tiles::Pos unpack_md(unsigned int md[][Ntiles], Tiles::Tile ts[], Tiles::Cost *h) {
		assert("Unimplemented" == 0);
	}

	unsigned long hash(void) {
		return hashbytes(bytes, sizeof(bytes));
	}

	bool eq(PackedTiles &other) const {
		return memcmp(bytes, other.bytes, sizeof(bytes)) == 0;
	}
};

template<> class PackedTiles<16> {
	friend class Tiles;
	boost::uint64_t word;
	enum { Ntiles = 16 };
public:
	void pack(Tiles::Tile ts[]) {
		for (int i = 0; i < Ntiles; i++) {
			word <<= 4;
			word |= ts[i];
		}
	}

	Tiles::Pos unpack(Tiles::Tile ts[]) {
		int b = -1;
		boost::uint64_t w = word;
		for (int i = Ntiles - 1; i >= 0; i--) {
			Tiles::Tile t = w & 0xF;
			w >>= 4;
			ts[i] = t;
			if (t == 0)
				b = i;
		}
		return b;
	}

	Tiles::Pos unpack_md(unsigned int md[][Ntiles], Tiles::Tile ts[], Tiles::Cost *hp) {
		int b = -1;
		Tiles::Cost h = 0;
		boost::uint64_t w = word;
		for (int i = Ntiles - 1; i >= 0; i--) {
			Tiles::Tile t = w & 0xF;
			w >>= 4;
			ts[i] = t;
			if (t == 0)
				b = i;
			else
				h += md[t][i];
		}
		*hp = h;
		return b;
	}

	unsigned long hash(void) {
		return word;
	}

	bool eq(PackedTiles &other) const {
		return word == other.word;
	}
};