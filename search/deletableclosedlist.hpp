#include "../search/search.hpp"
#include "../gridnav/gridnav.hpp"

template<typename Ops, typename Node, typename D> struct DeletableClosedList : 
	public ClosedList<Ops, Node, D> {
	typedef typename D::PackedState PackedState;

	DeletableClosedList(unsigned long szhint) :
		ClosedList<Ops, Node, D>(szhint) {}

	~DeletableClosedList() {}
	
	Node* remove(PackedState &k) {
		return remove(k, this->dom->hash(k)); 
	}

	Node* remove(PackedState &k, unsigned long h) {
		unsigned int bindex = h % this->nbins;
		Node* prev = this->bins[bindex];
		Node *p = prev;
		for ( ; p; p = Ops::closedentry(p).nxt) {
			if (Ops::key(p) == k) break;
			prev = p;
		}

		if(!p) return NULL;

		this->fill--;
		if(prev == p) { //head of the list
			this->bins[bindex] = Ops::closedentry(p).nxt;
		}
		else { // middle or tail
			Ops::closedentry(prev).nxt = Ops::closedentry(p).nxt;
		}
		return p;
	}

	unsigned long getFill() const { return this->fill; }
};

template<typename Ops, typename Node> struct DeletableClosedList<Ops, Node, GridNav> 
	: public ClosedList<Ops, Node, GridNav> {
	typedef typename GridNav::PackedState PackedState;

	DeletableClosedList(unsigned long szhint) : ClosedList<Ops, Node, GridNav>(szhint) {}

	~DeletableClosedList() {}

	Node* remove(PackedState &k) { return remove(k, k.loc); }

	Node* remove(PackedState &k, unsigned long h) {
		if(this->nodes[h] == NULL) return NULL;

		Node* p = this->nodes[h];
		this->nodes[h] = NULL;
		return p;
	}
};