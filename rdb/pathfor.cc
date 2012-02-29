#include "rdb.hpp"
#include <cstring>
#include <cstdio>

void fatal(const char*, ...);

void usage(void);

int main(int argc, char *argv[]) {
	if (argc < 2)
		usage();

	const char *root = argv[1];

	RdbAttrs attrs;
	for (int i = 2; i < argc; i++) {
		char *key = argv[i];
		char *vl = strchr(key, '=');
		if (!vl)
			usage();
		vl[0] = '\0';
		vl++;
		attrs.push_back(key, vl);
	}

	printf("%s\n", rdbpathfor(root, attrs).c_str());
	return 0;
}

void usage(void) {
	fatal("Usage: pathfor <root> <key>=<value>*\n");
}