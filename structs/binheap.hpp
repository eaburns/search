#pragma once

#include <vector>
#include <boost/optional.hpp>

template <class Ops, class Elm> class BinHeap {
public:

	// push pushes a new element into the heap in
	// O(lg n) time.
	void push(Elm e) {
		heap.push_back(e);
		Ops::setind(e, heap.size() - 1);
		pullup(heap.size() - 1);
	}

	// pop pops the front element from the heap and
	// returns it in O(lg n) time.
	boost::optional<Elm> pop() {
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

	// front returns the front element of the heap if it is
	// not empty and an empty option if the heap is empty.
	// O(1) time.
	boost::optional<Elm> front() {
		if (heap.size() == 0)
			return boost::optional<Elm>();
		return boost::optional<Elm>(heap[0]);
	}

	// update updates the element in the heap at the given
	// index.  This should be called whenever the priority
	// of an element changes.  O(lg n) time.
	void update(long i) {
		i = pullup(i);
		pushdown(i);
	}

	// pushupdate either pushes the element or updates it's
	// position in the queue, given the element and i, it's
	// index value.  Note that the index value must be tracked
	// properly, i.e., i < 0 means that the element is not in
	// the priority queue and i  >= 0 means that the element
	// is at the given index. O(lg n) time.
	void pushupdate(Elm e, long i) {
		if (i  < 0)
			push(e);
		else
			update(i);
	}

	// empty returns true if the heap is empty and
	// false otherwise.
	bool empty() { return heap.empty(); }

	// clear clears all of the elements from the heap
	// leaving it empty.
	void clear() { heap.clear(); }

 	// size returns the number of entries in the heap.
	long size() { return heap.size(); }

	// at returns the element of the heap at the given
	// index.
	Elm at(long i) {
		assert (i < size());
		return heap[i];
	}

	// data returns the raw vector used to back the
	// heap.  If you mess with this then you must
	// reinit the heap afterwards to ensure that the
	// heap property still holds.
	std::vector<Elm> &data() { return heap; }

	// reinit reinitialize the heap property in O(n) time.
	void reinit() {
		if (heap.size() <= 0)
			return;

		for (long i = heap.size() / 2; ; i--) {
			pushdown(i);
			if (i == 0)
				break;
		}
	}

	// append appends the vector of elements to the heap
	// and ensures that the heap property holds after.
	void append(const std::vector<Elm> &elms) {
		heap.insert(heap.end(), elms.begin(), elms.end());
		reinit();
	}

private:
	friend bool binheap_push_test();
	friend bool binheap_pop_test();

	long parent(long i) { return (i - 1) / 2; }

	long left(long i) { return 2 * i + 1; }

	long right(long i) { return 2 * i + 2; }

	long pullup(long i) {
		if (i == 0)
			return i;
		long p = parent(i);
		if (Ops::pred(heap[i], heap[p])) {
			swap(i, p);
			return pullup(p);
		}
		return i;
	}

	long pushdown(long i) {
		long l = left(i), r = right(i);

		long sml = i;
		if (l < size() && Ops::pred(heap[l], heap[i]))
			sml = l;
		if (r < size() && Ops::pred(heap[r], heap[sml]))
			sml = r;

		if (sml != i) {
			swap(sml, i);
			return pushdown(sml);
		}

		return i;
	}

	void swap(long i, long j) {
		Ops::setind(heap[i], j);
		Ops::setind(heap[j], i);
		Elm tmp = heap[i];
		heap[i] = heap[j];
		heap[j] = tmp;
	}

	std::vector<Elm> heap;
};
