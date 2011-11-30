#include "pdb.hpp"
#include <cstring>

struct Node;

// Big is true if the number of tiles are greater than the bits in a long.
enum { Big = Tiles::Ntiles > sizeof(unsigned long) * 8 };

static unsigned int patsz;
static Tiles::Tile pattern[Tiles::Ntiles];
static bool sparse;

static void helpmsg(int);
static Pdb *gen(void);
static void putnode(Node**, Node*);
static void freenodes(Node*);

int main(int argc, char *argv[]) {

	if (argc < 3)
		helpmsg(1);

	if (strcmp(argv[1], "true") == 0)
		sparse = true;
	else if (strcmp(argv[1], "false") != 0)
		helpmsg(1);

	printf("Big: %d\n", Big);
	printf("sparse: %d\n", sparse);
	printf("Ntiles: %u\n", Tiles::Ntiles);

	for (int i = 2; i < argc; i++) {
		pattern[patsz] = strtoll(argv[i], NULL, 10);
		if (pattern[patsz] >= Tiles::Ntiles)
			fatal("%u is out of range of the tiles", pattern[patsz]);
		patsz++;
	}

	Pdb *p = gen();
	delete p;

	printf("max virtual memory: %lu KB\n", virtmem());

	return 0;
}

static void helpmsg(int status) {
	puts("Usage: pdbgen <bool> <tile>+");
	puts("The boolean argument is true for a sparse PDB and false for a dense one.");
	exit(status);
}

struct Node {

	static Tiles tiles;

	Node(Tiles::Tile pat[]) : next(NULL) {
		ps = new Tiles::Pos[patsz];
		for (unsigned int i = 0; i < patsz; i++)
			ps[i] = pat[i];
	}

	Node(const Node *o) : next(NULL) {
		ps = new Tiles::Pos[patsz];
		*this = *o;
	}

	~Node(void) { delete[] ps; }

	void operator=(const Node &o) {
		memcpy(ps, o.ps, patsz * sizeof(*ps));
	}

	Node *expand(Node **freelst) const {
		Node *lst = NULL;
		unsigned long blkd = Big ? 0 : blkdmask();

		for (unsigned int i = 0; i < patsz; i++) {
			Tiles::Pos src = ps[i];

			for (unsigned int j= 0; j < tiles.ops[src].n; j++) {
				Tiles::Oper dst = tiles.ops[src].mvs[j];
				if (!mvok(blkd, src, dst))
					continue;

				Node *kid = dup(freelst);
				kid->ps[i] = dst;
				kid->next = lst;
				lst = kid;
			}
		}

		return lst;
	}

	Node *next;
	Tiles::Pos *ps;

private:
	
	Node *dup(Node **nodes) const {
		if (!*nodes)
			return new Node(this);
		Node *d = *nodes;
		*nodes = d->next;
		*d = *this;
		return d;
	}

	unsigned long blkdmask(void) const {
		assert(!Big);
		unsigned long blkd = 0;
		for (unsigned int i = 0; i < patsz; i++)
			blkd |= 1 << ps[i];
		return blkd;
	}

	bool mvok(unsigned long blkd, Tiles::Pos p, Tiles::Oper dst) const {
		if (!Big)
			return !(blkd & 1 << dst);

		for (unsigned int i = 0; i < patsz; i++) {
			if (ps[i] == p)
				continue;
			if (ps[i] == (unsigned int) dst)
				return false;
		}
		return true;
	}
};

Tiles Node::tiles;

static Pdb *gen(void) {
	Pdb *pdb;
	if (sparse)
		pdb = new SparsePdb(patsz, pattern);
	else
		pdb = new CompactPdb(patsz, pattern);

	double start = walltime();
	unsigned int depth = 0;
	unsigned long num = 1;
	Node *current = new Node(pattern), *next = NULL, *free = NULL;
	*pdb->poscost(current->ps) = 0;

	while (current || next) {
		if (!current) {
			printf("depth %u: %lu nodes\n", depth, num);
			current = next;
			next = NULL;
			depth++;
		}

		assert(current);
		Node *n = current;
		current = n->next;

		for (Node *q, *p = n->expand(&free); p; p = q) {
			q = p->next;

			char *c = pdb->poscost(p->ps);
			if (*c >= 0) {
				putnode(&free, p);
				continue;
			}

			num++;
			*c = depth + 1;
			p->next = next;
			next = p;
		}

		putnode(&free, n);
	}

	printf("%lu entries in %g seconds\n", num, walltime() - start);
	printf("maximum depth: %u\n", depth);
	freenodes(free);

	return pdb;
}

static void putnode(Node **nodes, Node *n) {
	n->next = *nodes;
	*nodes = n;
}

static void freenodes(Node *nodes) {
	while (nodes) {
		Node *n = nodes;
		nodes = n->next;
		delete n;
	}
}