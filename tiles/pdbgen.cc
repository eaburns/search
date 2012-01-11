#include "pdb.hpp"
#include <cstring>

struct Node;

// Big is true if the number of tiles are greater than the bits in a long.
enum { Big = Tiles::Ntiles > sizeof(unsigned long) * 8 };

#ifndef PATTERN
//#define PATTERN 1, 4, 5, 8, 9, 12, 13
#define PATTERN 1,2,3
//#define PATTERN 2, 3, 6, 7, 10, 11, 14, 15
#endif

static const Tiles::Tile pattern[] = { PATTERN };
enum { Patsz = sizeof(pattern) / sizeof(pattern[0]) };

static bool sparse;

static void helpmsg(int);
static Pdb *gen(void);
static Pdb *newpdb(void);
static Node *expand(Node**, const Node*);
static Node *dupnode(Node**, const Node*);
static void putnode(Node**, Node*);
static unsigned long freenodes(Node*);

int main(int argc, char *argv[]) {

	if (argc > 2)
		helpmsg(1);

	if (argc == 2 && strcmp(argv[1], "-sparse") == 0)
		sparse = true;
	else if (argc == 2)
		helpmsg(1);

	printf("Big: %d\n", Big);
	printf("sparse: %d\n", sparse);
	printf("Ntiles: %u\n", Tiles::Ntiles);
	printf("Pattern:");
	for (unsigned int i = 0; i < Patsz; i++)
		printf(" %d", pattern[i]);
	printf("\n");

	Pdb *p = gen();
	delete p;

	printf("max virtual memory: %lu KB\n", virtmem());

	return 0;
}

static void helpmsg(int status) {
	puts("Usage: pdbgen [-sparse]");
	exit(status);
}

struct Node {

	Node(const Tiles::Tile pat[]) : next(NULL) {
		for (unsigned int i = 0; i < Patsz; i++)
			ps[i] = pat[i];
	}

	Node(const Node *o) : next(NULL) {
		*this = *o;
	}

	void operator=(const Node &o) {
		memcpy(ps, o.ps, Patsz * sizeof(*ps));
	}

	unsigned long blkdmask(void) const {
		assert(!Big);
		unsigned long blkd = 0;
		for (unsigned int i = 0; i < Patsz; i++)
			blkd |= 1 << ps[i];
		return blkd;
	}

	bool mvok(unsigned long blkd, Tiles::Pos p, Tiles::Oper dst) const {
		if (!Big)
			return !(blkd & 1 << dst);

		for (unsigned int i = 0; i < Patsz; i++) {
			if (ps[i] == p)
				continue;
			if (ps[i] == (unsigned int) dst)
				return false;
		}
		return true;
	}

	Node *next;
	unsigned char ps[Patsz];
};

static Pdb *gen(void) {
	Pdb *pdb = newpdb();

	double start = walltime();
	unsigned int depth = 0;
	unsigned long num = 1, cur = 0;
	Node *current = new Node(pattern), *next = NULL, *free = NULL;
	unsigned char *goalcost = pdb->poscost(current->ps);
       	*goalcost = 0;

	while (current || next) {
		if (!current) {
			printf("depth %u:\t%lu nodes\t%lu total\t%lu MB\n", depth + 1,
				cur, num, virtmem() / 1024);
			current = next;
			next = NULL;
			depth++;
			cur = 0;
		}

		assert(current);
		Node *n = current;
		current = n->next;

		for (Node *q, *p = expand(&free, n); p; p = q) {
			q = p->next;

			unsigned char *c = pdb->poscost(p->ps);
			if (*c > 0 || c == goalcost) {
				putnode(&free, p);
				continue;
			}

			num++;
			cur++;
			*c = depth + 1;
			p->next = next;
			next = p;
		}

		putnode(&free, n);
	}

	unsigned long nodes = freenodes(free);
	unsigned int nodesz = sizeof(Node);
	printf("%lu entries in %g seconds\n", num, walltime() - start);
	printf("%lu nodes allocated\n", nodes);
	printf("%lu bytes for nodes, %u each\n", nodes * nodesz, nodesz);
	printf("maximum depth: %u\n", depth);

	return pdb;
}

static Pdb *newpdb(void) {
	if (sparse)
		return new SparsePdb(Patsz, pattern);
	return new CompactPdb(Patsz, pattern);
}

static Node *expand(Node **nodes, const Node *n) {
	static Tiles tiles;
	Node *lst = NULL;
	unsigned long blkd = Big ? 0 : n->blkdmask();

	for (unsigned int i = 0; i < Patsz; i++) {
		Tiles::Pos src = n->ps[i];

		for (unsigned int j= 0; j < tiles.ops[src].n; j++) {
			Tiles::Oper dst = tiles.ops[src].mvs[j];
			if (!n->mvok(blkd, src, dst))
				continue;

			Node *kid = dupnode(nodes, n);
			kid->ps[i] = dst;
			kid->next = lst;
			lst = kid;
		}
	}

	return lst;
}

static Node *dupnode(Node **nodes, const Node *n) {
	Node *d = *nodes;
	if (!d)
		return new Node(n);
	*nodes = d->next;
	*d = *n;
	return d;
}

static void putnode(Node **nodes, Node *n) {
	n->next = *nodes;
	*nodes = n;
}

static unsigned long freenodes(Node *nodes) {
	unsigned long count = 0;
	while (nodes) {
		Node *n = nodes;
		nodes = n->next;
		delete n;
		count++;
	}
	return count;
}
