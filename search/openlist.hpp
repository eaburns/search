#include "../structs/intpq.hpp"
#include "../structs/binheap.hpp"
#include <boost/optional.hpp>

template <class Ops, class Elm, class Cost> class OpenList {
public:

	enum { InitInd = -1 };

	void push(Elm e) {
		heap.push(e);
	}

	boost::optional<Elm> pop(void) { 
		return heap.pop();
	}

	void pre_update(Elm e) {
	}

	void post_update(Elm e) {
		heap.update(Ops::getind(e));
	}

	bool empty(void) {
		return heap.empty();
	}

	bool mem(Elm e) {
		return Ops::getind(e) >= 0;
	}

private:
	Binheap<Ops, Elm> heap;
};


template <class Ops, class Elm> class OpenList <Ops, Elm, int> {
public:

	enum { InitInd = -1 };

	void push(Elm e) {
		pq.push(e);
	}

	boost::optional<Elm> pop(void) {
		return pq.pop();
	}

	void pre_update(Elm e) {
		pq.rm(e);
	}

	void post_update(Elm e) {
		pq.push(e);
	}

	bool empty(void) {
		return pq.empty();
	}

	bool mem(Elm e) {
		return Ops::getind(e) >= 0;
	}

private:
	Intpq<Ops, Elm> pq;
};