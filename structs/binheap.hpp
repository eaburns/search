#ifndef _BINHEAP_HPP_
#define _BINHEAP_HPP_

#include <vector>
#include <boost/optional.hpp>

template <class Ops, class Elm> class BinHeap {
public:

	void push(Elm e) {
		heap.push_back(e);
		pullup(heap.size() - 1);
	}

	boost::optional<Elm> pop(void) {
		if (heap.size() == 0)
			return boost::optional<Elm>();

		Elm res = heap[0];
		if (heap.size() > 1) {
			heap[0] = heap.back();
			heap.pop_back();
			pushdown(0);
		} else {
			heap.pop_back();
		}
		Ops::setind(res, -1);

		return boost::optional<Elm>(res);
	}

	boost::optional<Elm> front(void) {
		if (heap.size() == 0)
			return boost::optional<Elm>();
		return boost::optional<Elm>(heap[0]);
	}

	void update(long i) {
		i = pullup(i);
		pushdown(i);
	}

	bool empty(void) {
		return heap.empty();
	}

	void clear(void) {
		heap.clear();
	}

	unsigned long size(void) {
		return heap.size();
	}

	Elm at(unsigned long i) {
		assert (i < heap.size());
		return heap[i];
	}

	// Reinitialize the heap property
	void reinit(void) {
		if (heap.size() <= 0)
			return;

		for (unsigned long i = heap.size() / 2; i >= 0; i--) {
			pushdown(i);
			if (i == 0)
				break;
		}
	}

	void append(const std::vector<Elm> &elms) {
		heap.insert(heap.end(), elms.begin(), elms.end());
		reinit();
	}

private:
	friend bool binheap_push_test(void);
	friend bool binheap_pop_test(void);

	unsigned long parent(unsigned long i) {
		return (i - 1) / 2;
	}

	unsigned long left(unsigned long i) {
		return 2 * i + 1;
	}

	unsigned long right(unsigned long i) {
		return 2 * i + 2;
	}

	unsigned long pullup(unsigned long i) {
		if (i == 0)
			return i;
		long p = parent(i);
		if (Ops::pred(heap[i], heap[p])) {
			swap(i, p);
			return pullup(p);
		}
		return i;
	}

	long pushdown(unsigned long i) {
		unsigned long l = left(i), r = right(i);

		unsigned long sml = i;
		if (l < heap.size() && Ops::pred(heap[l], heap[i]))
			sml = l;
		if (r < heap.size() && Ops::pred(heap[r], heap[sml]))
			sml = r;

		if (sml != i) {
			swap(sml, i);
			return pushdown(sml);
		}

		return i;
	}

	void swap(unsigned long i, unsigned long j) {
		Ops::setind(heap[i], j);
		Ops::setind(heap[j], i);
		Elm tmp = heap[i];
		heap[i] = heap[j];
		heap[j] = tmp;
	}

	std::vector<Elm> heap;
};

#endif	// _BINHEAP_HPP_
