// Copyright © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.
#pragma once

#include <vector>
#include <cstdlib>

template <class Obj> class Pool {
public:

	Pool(unsigned int sz = 1024) : blksz(sz), nxt(0), freed(0), blk(-1) {
		newblk();
	}

	~Pool() {
		for (unsigned int i = 0; i < blks.size(); i++)
			delete[] blks[i];
	}

	Obj *get() {
		if (freed) {
			Ent *res = freed;
			freed = freed->nxt;
			return (Obj*) res->bytes;
		}

		if (nxt == blksz)
			newblk();

		return (Obj*) blks[blk][nxt++].bytes;
	}

	void put(Obj *o) {
		Ent *e = (Ent*) o;
		e->nxt = freed;
		freed = e;
	}

	Obj *construct() {
		Obj *o = get();
		return new (o) Obj();
	}

	void destruct(Obj *o) {
		o->~Obj();
		put(o);
	}

	// Blocks returns the number of allocated blocks.
	unsigned long blocks() const {
		return blks.size();
	}

private:

	void newblk() {
		nxt = 0;
		blk++;
		if (blks.size() <= (unsigned int) blk) {
			Ent *blk = new Ent[blksz];
			blks.push_back(blk);
		}
	}

	union Ent {
		char bytes[sizeof(Obj)];
		Ent *nxt;
	};

	unsigned int blksz, nxt;
	Ent *freed;
	int blk;
	std::vector<Ent*> blks;
};
