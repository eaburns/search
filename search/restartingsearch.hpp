#include "../search/search.hpp"

template <class D> struct RestartingSearch : public SearchAlgorithm<D> {

public:
	typedef typename D::State State;

	RestartingSearch(SearchAlgorithm<D> *searchAlg, int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv), searchAlg(searchAlg) {
	}

	void search(D &d, typename D::State &s0) {

		this->start();
		State currentState = s0;
		while(true) {

			searchAlg->search(d, currentState);
			Result<D> solution = searchAlg->res;
			if(solution.ops.size() <= 0) {
				break;
			}
			appendToRes(solution);

			d.act(solution.path.back(), solution.ops.back());
			currentState = solution.path[solution.path.size()-2];

			searchAlg->reset();
		}

		this->finish();

		std::reverse(this->res.ops.begin(), this->res.ops.end());
		std::reverse(this->res.path.begin(), this->res.path.end());
	}

	virtual void reset() {
		SearchAlgorithm<D>::reset();
		searchAlg->reset();
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
	}

private:
	void appendToRes(const Result<D> &solution) {
		this->res.ops.push_back(solution.ops.back());
		this->res.path.push_back(solution.path.back());
	}

	SearchAlgorithm<D> *searchAlg;
};
