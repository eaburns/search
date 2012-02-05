#include "utils.hpp"
#include <cstring>
#include <ctype.h>

bool hasprefix(const char *str, const char *prefix) {
	unsigned int plen = strlen(prefix);
	if (strlen(str) < plen)
		return false;
	for (unsigned int i = 0; i < plen; i++)
		if (str[i] != prefix[i])
			return false;
	return true;
}

char *gettoken(char *str, unsigned int lineno) {
	unsigned int i;
	for (i = 0; i < strlen(str) && isspace(str[i]); i++)
		;
	if (i >= strlen(str))
		return NULL;

	if (str[i] == '"') {
		char *strt = str + i + 1;
		for (i = 0; i < strlen(strt) && strt[i] != '"'; i++)
			;
		if (i == strlen(strt) || strt[i] != '"') {
			if (lineno > 0)
				fatal("line %u: No closing quote", lineno);
			fatal("No closing quote");
		}
		strt[i] = '\0';
		return strt;
	}

	char *strt = str + i;
	for (i = 0; i < strlen(strt) && !isspace(strt[i]); i++)
		;
	strt[i] = '\0';

	return strt;	
}