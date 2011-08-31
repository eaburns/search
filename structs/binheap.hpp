#include <vector>

template <class Ops, class Elm> class Binheap {
public:

	void push(Elm e) {
		heap.push_back(e);
		pullup(heap.size() - 1);
	}

	Elm pop(void) {
		if (heap.size() == 0)
			return NULL;

		Elm res = heap[0];
		heap[0] = heap[heap.size() - 1];
		pushdown(0);

		return res;
	}

	void update(int i) {
		i = pullup(i);
		pushdown(i);
	}

	bool empty(void) {
		return heap.empty();
	}

private:

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
		int p = parent(i);
		if (p > 0 && Ops::pred(heap[i], heap[p])) {
			swap(i, p);
			return pullup(p);
		}

		return i;
	}

	int pushdown(int i) {
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
