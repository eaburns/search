#include "../structs/intpq.hpp"
#include "../structs/binheap.hpp"

template <class Ops, class Ent> class OpenList {
public:
	void push(Ent) = 0;
	Ent pop(void) = 0;
	void pre_update(Ent) = 0;
	void post_update(Ent) = 0;
	bool empty(void) = 0;
};

template <class Ops, class Ent> class IntOpenList : public OpenList<Ops, Ent> {
public:

	void push(Ent e) {
		pq.push(e);
		Ops::setindex(e, 1);
	}

	Ent pop(void) {
		Ent &e = pq.pop();
		Ops::setindex(e, 0);
		return e;
	}

	void pre_update(Ent e) {
		pq.rm(e);
	}

	void post_update(Ent e) {
		pq.push(e);
	}

	bool empty(void) {
		return pq.empty();
	}

private:
	Intpq<Ops, Ent> pq;
};

template <class Ops, class Ent> class FloatOpenList : public OpenList<Ops, Ent> {
public:

	void push(Ent e) {
		heap.push(e);
	}

	Ent pop(void) { 
		return heap.pop();
	}

	void pre_update(Ent e) {
	}

	void post_update(Ent e) {
		heap.update(Ops::getind(e));
	}

	bool empty(void) {
		return heap.empty();
	}

private:
	Binheap<Ops, Ent> heap;
};