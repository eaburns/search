// The Djset type is a disjoint forrest implementation of disjoint sets.
struct Djset {

	// Djset creates a new set.
	Djset(void) : aux(NULL), rank(0) { parent = this; }

	// clear resets the set as though it has just been freshly
	// created.
	void clear(void) {
		rank = 0;
		parent = this;
	}

	// find returns the canonical representation of this set.
	Djset *find(void) {
		if (parent == this)
			return parent;
		parent = parent->find();
		return parent;
	}

	// join unions two sets so that performing a find on either
	// will return the same canonical set.
	void join(Djset &o) {
		Djset *root = find();
		Djset *oroot = o.find();
		if (root == oroot)
			return;
		if (root->rank < oroot->rank) {
			root->parent = oroot;
		} else {
			oroot->parent = root;
			if (root->rank == oroot->rank)
				rank++;
		}
	}

	// aux is initialized to NULL and is never again touched by
	// this code.  This may be used however you please.
	void *aux;

private:
	Djset *parent;
	unsigned int rank;
};