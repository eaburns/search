#include "utils.hpp"
#include <cassert>
#include <cstring>

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

std::string runlenenc(const std::string &data) {
	std::string buf, dst;

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
	return dst;
}

std::string runlendec(const std::string &data) {
	std::string dst;
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
	return dst;
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

std::string ascii85enc(const std::string &data) {
	std::string dst;
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
		return dst;

	assert (data.size() - i < 4);
	word = (unsigned char) data[i] << 24;
	if (data.size() - i > 1)
		word |= (unsigned char) data[i+1] << 16;
	if (data.size() - i > 2)
		word |= (unsigned char) data[i+2] << 8;

	digits85(dst, data.size() - i + 1, word);
	return dst;
}

// Decoding ascii85:
// 	Ignore white-space.
//	A 'z' in the middle of a 5-byte sequence is an error.
//	Values decoding to > 2^32 - 1 are errors.

static const char base64tab[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
	'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
};

std::string base64enc(const std::string &data) {
	assert (sizeof(base64tab) / sizeof(base64tab[0]) == 64);

	std::string enc;

	for (unsigned int i = 0; i < data.size(); i+=3) {
		unsigned long b = (data[i] & 0xFF) << 16;
		if (i+1 < data.size()) b |= (data[i+1] & 0xFF) << 8;
		if (i+2 < data.size()) b |= (data[i+2] & 0xFF);

		char e[5];
		e[4] = '\0';
		for (int j = 3; j >= 0; j--) {
			unsigned int ind = b & 0x3F;
			assert (ind < 64);
			e[j] = base64tab[ind];
			b >>= 6;
		}

		if (data.size() < 3 || i > data.size() - 3) {
			unsigned int pad = i + 3 - data.size();
			assert (pad < 3);
			if (pad > 0) e[3] = '=';
			if (pad > 1) e[2] = '=';
		}

		enc.append(e);
	}

	return enc;
}

std::string base64dec(const std::string &data) {
	assert (sizeof(base64tab) / sizeof(base64tab[0]) == 64);

	// Decoding table.  Initialized onec on call to base64dec.
	static char tab[255];
	if (tab[(int)'B'] != 1) {
		for (unsigned int i = 0; i < 64; i++)
			tab[(int) base64tab[i]] = i;
	}

	std::string dec;

	for (unsigned int i = 0; i < data.size(); i += 4) {
		unsigned long b = tab[(int) data[i]] << 18;
		b |= tab[(int) data[i+1]] << 12;
		if (data[i+2] != '=') b |= tab[(int) data[i+2]] << 6;
		if (data[i+3] != '=') b |= tab[(int) data[i+3]];

		dec.push_back((b & 0xFF0000) >> 16);
		if (data[i+2] != '=')
			dec.push_back((b & 0xFF00) >> 8);
		if (data[i+3] != '=')
			dec.push_back(b & 0xFF);
	}

	return dec;
}
