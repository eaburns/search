#include "utils.hpp"
#include <cassert>

enum {
	Maxrun = 127,
	Minrun = 3,
};

static unsigned int cntsame(const std::string &s, unsigned int strt) {
	unsigned int i = strt + 1;
	while (i < s.size() && i - strt < Maxrun && s[i] == s[strt])
		i++;
	return i - strt;
}

void runlenenc(std::string &dst, const std::string &data) {
	std::string buf;

	for (unsigned int i = 0; i < data.size(); /* Nothing */) {
		unsigned int nsame = cntsame(data, i);
		if (nsame < Minrun) {	// Save non-run characters
			buf.push_back(data[i]);
			if (buf.size() == Maxrun) {
				dst.push_back(Maxrun - 1);
				dst.append(buf);
				buf.clear();
			}
			i++;
			continue;
		}

		if (buf.size() > 0) {	// Output stored non-run
			// Can't be equal since it will have been output above
			assert(buf.size() < Maxrun);
			dst.push_back(buf.size() - 1);
			dst.append(buf);
			buf.clear();
		}

		assert (nsame <= Maxrun);
		char c = data[i];
		dst.push_back(1 - nsame);
		dst.push_back(c);
		i += nsame;
	}

	if (buf.size() > 0) {	// Output stored non-run
		assert(buf.size() < Maxrun);
		dst.push_back(buf.size() - 1);
		dst.append(buf);
		buf.clear();
	}
}

void runlendec(std::string &dst, const std::string &data) {
	unsigned int i = 0;
	while (i < data.size()) {
		char n = data[i++];

		if (n < 0) {	// A run
			int len = -n + 1;
			assert (len <= Maxrun);
			for (int j = 0; j < len; j++)
				dst.push_back(data[i]);
			i++;
			continue;
		}

		// Not a run
		assert (n + 1 >= 0);
		assert (n + 1 <= Maxrun);
		unsigned int end = i + n + 1;
		for (; i < end; i++)
			dst.push_back(data[i]);
	}
}

static void digits85(std::string &dst, unsigned int n, uint32_t word) {
	if (word == 0 && n == 5) {
		dst.push_back('z');
		return;
	}

	unsigned int offs = dst.size() - 1;
	dst.resize(dst.size() + n);
	for (unsigned int i = 0; i < 5; i++) {
		uint32_t digit = word % 85;
		word /= 85;
		if (offs + 5 - i < dst.size())
			dst[offs + 5 - i] = digit + '!';
	}
	assert (word == 0);
}

void ascii85enc(std::string &dst, const std::string &data) {
	uint32_t word;
	unsigned int i = 0;

	while (data.size() - i >= 4) {
		word = (unsigned char) data[i++] << 24;
		word |= (unsigned char) data[i++] << 16;
		word |= (unsigned char) data[i++] << 8;
		word |= (unsigned char) data[i++];
		digits85(dst, 5, word);
	}

	if (data.size() - i == 0)
		return;

	assert (data.size() - i < 4);
	word = (unsigned char) data[i] << 24;
	if (data.size() - i > 1)
		word |= (unsigned char) data[i+1] << 16;
	if (data.size() - i > 2)
		word |= (unsigned char) data[i+2] << 8;

	digits85(dst, data.size() - i + 1, word);
}

// Decoding ascii85:
// 	Ignore white-space.
//	A 'z' in the middle of a 5-byte sequence is an error.
//	Values decoding to > 2^32 - 1 are errors.