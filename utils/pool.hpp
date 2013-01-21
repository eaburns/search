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

	// Releaseall releases all memory back to the pool.
	// No destructors are called.
	void releaseall() {
		blk = 0;
		nxt = 0;
		freed = NULL;
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
