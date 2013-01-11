#include "../search/search.hpp"
#include "../gridnav/gridnav.hpp"

template<typename Ops, typename Node, typename D> struct IterableClosedList : 
	public ClosedList<Ops, Node, D> {

	IterableClosedList(unsigned long szhint) : ClosedList<Ops, Node, D>(szhint) {}

	~IterableClosedList() {}

	struct iterator {
		iterator() : done(false), bin(-1), elem(NULL) {}
		bool done;
		int bin;
		Node* elem;
	};

	iterator* getIterator() const {
		iterator* iter = new iterator();
		for(unsigned int i = 0; i < this->nbins; i++) {
			if(this->bins[i] != NULL) {
				iter->done = false;
				iter->elem = this->bins[i];
				return iter;
			}
		}

		iter->done = true;
		return iter;
	}

	void advanceIterator(iterator* iter) const {
		if(iter->done ||  iter->bin < 0 || iter->elem == NULL) return;

		Node* p = Ops::closedentry(iter->elem).nxt;
		if(p) {
			iter->elem = p;
			return;
		}

		for(unsigned int i = iter->bin+1; i < this->nbins; i++) {
			if(this->bins[i] != NULL) {
				iter->bin = i;
				iter->elem = this->bins[i];
				return;
			}
		}

		iter->done = true;
	}

	void destroyIterator(iterator* iter) const { if(iter) delete iter; }
};

template<typename Ops, typename Node> struct IterableClosedList<Ops, Node, GridNav> 
	: public ClosedList<Ops, Node, GridNav> {

	IterableClosedList(unsigned long szhint) : ClosedList<Ops, Node, GridNav>(szhint) {}

	~IterableClosedList() {}

	struct iterator {
		iterator() : done(false), bin(-1), elem(NULL) {}
		bool done;
		long bin;
		Node* elem;
	};

	iterator* getIterator() const {
		iterator* iter = new iterator();
		for(unsigned int i = 0; i < this->cap; i++) {
			if(this->nodes[i] != NULL) {
				iter->done = false;
				iter->elem = this->nodes[i];
				return iter;
			}
		}

		iter->done = true;
		return iter;
	}

	void advanceIterator(iterator* iter) const {
		if(iter->done ||  iter->bin < 0 || iter->elem == NULL) return;

		for(unsigned int i = iter->bin+1; i < this->cap; i++) {
			if(this->nodes[i] != NULL) {
				iter->elem = this->nodes[i];
				return;
			}
		}

		iter->done = true;
	}

	void destroyIterator(iterator* iter) const { if(iter) delete iter; }
};
