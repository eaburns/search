#include <cstring>

// hasprefix returns true if the string has the given prefix
bool hasprefix(const char *str, const char *prefix) {
	unsigned int plen = strlen(prefix);
	if (strlen(str) < plen)
		return false;
	for (unsigned int i = 0; i < plen; i++)
		if (str[i] != prefix[i])
			return false;
	return true;
}