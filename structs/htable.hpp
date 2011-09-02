#include <vector>
#include <list>
#include <cstring>
#include <cstdio>
#include <boost/optional.hpp>

void dfpair(FILE *, const char *key, const char *fmt, ...);

template <class Ops, class Key, class Val> class Htable {
public:

	Htable(unsigned int sz = 1024) : fill(0), collides(0) {
		bkts.resize(sz);
	}

	void add(Key k, Val v) {
		if (fill * 3 >= bkts.size())
			grow();

		unsigned int ind = Ops::hash(k) % bkts.size();
		if (!bkts[ind].empty())
			collides++;
		bkts[ind].push_back(std::pair<Key,Val>(k, v));
		fill++;
	}

	boost::optional<Val>  repl(Key k, Val v) {
		unsigned int ind = Ops::hash(k) % bkts.size();
		Bucket &bkt = bkts[ind];

		for (typename Bucket::iterator it = bkt.begin(); it != bkt.end(); it++) {
			std::pair<Key, Val> &e = *it;
			if (Ops::eq(k, e.first)) {
				Val old = e.second;
				e.second = e.v;
				return boost::optional<Val>(old);
			}
		}

		bkts[ind].push_back(pair(k, v));
		fill++;

		return boost::optional<Val>();
	}

	bool mem(Key k) {
		unsigned int ind = Ops::hash(k) % bkts.size();
		Bucket &bkt = bkts[ind];

		for (typename Bucket::iterator it = bkt.begin(); it != bkt.end(); it++) {
			std::pair<Key, Val> &e = *it;
			if (Ops::eq(k, e.first))
				return true;
		}

		return false;
	}

	boost::optional<Val> find(Key k) {
		unsigned int ind = Ops::hash(k) % bkts.size();
		Bucket &bkt = bkts[ind];

		for (typename Bucket::iterator it = bkt.begin(); it != bkt.end(); it++) {
			std::pair<Key, Val> &e = *it;
			if (Ops::eq(k, e.first))
				return boost::optional<Val>(e.second);
		}

		return boost::optional<Val>();
	}

	Val rm(Key k) {
		unsigned int ind = Ops::hash(k) % bkts.size();
		Bucket &bkt = bkts[ind];

		for (typename Bucket::iterator it = bkt.begin(); it != bkt.end(); it++) {
			std::pair<Key, Val> &e = *it;

			if (Ops::eq(k, e.first)) {
				Val res = e.second;

				bkt.erase(it);
				bkt.pop_back();
				fill--;

				return boost::optional<Val>(res);
			};
		}

		return boost::optional<Val>();
	}

	void prstats(FILE *f, const char *prefix) {
		char key[strlen(prefix) + strlen("collisions") + 1];
		strcpy(key, prefix);

		strcat(key+strlen(prefix), "fill");
		dfpair(f, key, "%lu", fill);

		strcat(key+strlen(prefix), "collisions");
		dfpair(f, key, "%lu", collides);
	}

private:
	friend bool htable_add_test(void);

	typedef std::list< std::pair<Key, Val> > Bucket;
	typedef std::vector<Bucket> Buckets;

	void grow(void) {
		unsigned int newsz = bkts.size() * 2;
		Buckets b(newsz);

		unsigned int oldsz = bkts.size();
		for (unsigned int i = 0; i < oldsz; i++) {
			Bucket &bkt = bkts[i];
			for (typename Bucket::iterator it = bkt.begin(); it != bkt.end(); it++) {
				std::pair<Key, Val> &e = *it;
				unsigned int ind = Ops::hash(e.first) % newsz;
				b[ind].push_back(e);
			}
		}

		bkts.swap(b);
	}

	unsigned long fill, collides;
	Buckets bkts;
};