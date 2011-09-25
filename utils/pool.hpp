#include <vector>
#include <cstdlib>

template <class Obj> class Pool {
public:

	Pool(unsigned int sz = 1024) : blksz(sz), nxt(0), freed(0) {
		newblk();
	}

	~Pool(void) {
		for (unsigned int i = 0; i < blks.size(); i++)
			free(blks[i]);
	}

	Obj *get(void) {
		if (freed) {
			Ent *res = freed;
			freed = freed->nxt;
			return (Obj*) res->bytes;
		}

		if (nxt == blksz)
			newblk();

		return (Obj*) blks.back()[nxt++].bytes;
	}

	void put(Obj *o) {
		Ent *e = (Ent*) o;
		e->nxt = freed;
		freed = e;
	}

	Obj *construct(void) {
		Obj *o = get();
		return new (o) Obj();
	}

	void destruct(Obj *o) {
		o->~Obj();
		put(o);
	}

private:

	void newblk(void) {
		Ent *blk = (Ent*) malloc(blksz * sizeof(*blk));
		blks.push_back(blk);
		nxt = 0;
	}

	union Ent {
		char bytes[sizeof(Obj)];
		Ent *nxt;
	};

	unsigned int blksz, nxt;
	Ent *freed;
	std::vector<Ent*> blks;
};