#include "../structs/intpq.hpp"
#include "../structs/binheap.hpp"

template <class Ops, class Ent, class Cost> class OpenList {
public:

	enum { InitInd = -1 };

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

	bool mem(Ent e) {
		return Ops::getind(e) >= 0;
	}

private:
	Binheap<Ops, Ent> heap;
};


template <class Ops, class Ent> class OpenList <Ops, Ent, int> {
public:

	enum { InitInd = -1 };

	void push(Ent e) {
		pq.push(e);
		Ops::setind(e, 1);
	}

	Ent pop(void) {
		Ent e = pq.pop();
		Ops::setind(e, -1);
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

	bool mem(Ent e) {
		return Ops::getind(e) >= 0;
	}

private:
	Intpq<Ops, Ent> pq;
};