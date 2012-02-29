#include "rdb.hpp"
#include <cstring>
#include <cstdio>

void fatal(const char*, ...);

void usage(void);
int nextpair(int, char *[], std::string&, std::string&);

int main(int argc, char *argv[]) {
	if (argc < 2)
		usage();

	const char *root = argv[1];

	RdbAttrs attrs;
	argc -= 2;
	argv+= 2;
	while (argc > 0) {
		std::string key, value;
		int shift = nextpair(argc, argv, key, value);
		argc -= shift;
		argv += shift;
		attrs.push_back(key.c_str(), value.c_str());
	}

	printf("%s\n", rdbpathfor(root, attrs).c_str());
	return 0;
}

void usage(void) {
	fatal("Usage: pathfor <root> <key>=<value>*\n");
}

int nextpair(int argc, char *argv[], std::string &key, std::string &value) {
	key.clear();
	value.clear();

	for (int i = 0; i < argc; i++) {
		char *vl =strchr(argv[i], '=');
		if (!vl) {
			key += argv[i];
			key.push_back(' ');
			continue;
		}
		vl[0] = '\0';
		key += argv[i];
		value = vl+1;
		return i+1;
	}
	usage();
	return -1;	// unreachable
}