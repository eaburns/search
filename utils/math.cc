#include <boost/cstdint.hpp>

unsigned int ilog2(boost::uint32_t v) {
	if (!v)
		return 0;

	unsigned int lg = 0;

	if (v & 0xFFFF0000) {
		lg += 16;
		v >>= 16;
	}
	if (v & 0xFF00) {
		lg += 8;
		v >>= 8;
	}
	if (v & 0xF0) {
		lg += 4;
		v >>= 4;
	}
	if (v & 0xC) {
		lg += 2;
		v >>= 2;
	}
	if (v & 0x2) {
		lg += 1;
		v >>= 1;
	}
	return lg + 1;
}

unsigned long ipow(unsigned int b, unsigned int e) {
	unsigned long r = 1;
	for (unsigned int i = 0; i < e; i++)
		r *= b;
	return r;
}