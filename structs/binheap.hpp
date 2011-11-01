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

	void update(int i) {
		i = pullup(i);
		pushdown(i);
	}

	bool empty(void) {
		return heap.empty();
	}

	void clear(void) {
		heap.clear();
	}

private:
	friend bool binheap_push_test(void);
	friend bool binheap_pop_test(void);

	int parent(int i) {
		return (i - 1) / 2;
	}

	int left(int i) {
		return 2 * i + 1;
	}

	int right(int i) {
		return 2 * i + 2;
	}

	int pullup(int i) {
		if (i == 0)
			return i;
		int p = parent(i);
		if (Ops::pred(heap[i], heap[p])) {
			swap(i, p);
			return pullup(p);
		}
		return i;
	}

	int pushdown(unsigned int i) {
		unsigned int l = left(i), r = right(i);

		unsigned int sml = i;
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

	void swap(int i, int j) {
		Ops::setind(heap[i], j);
		Ops::setind(heap[j], i);
		Elm tmp = heap[i];
		heap[i] = heap[j];
		heap[j] = tmp;
	}

	std::vector<Elm> heap;
};

#endif	// _BINHEAP_HPP_
