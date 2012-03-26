#include "rdb.hpp"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>

void fatal(const char*, ...);

static void usage(int);

int main(int argc, const char *argv[]) {
	if (argc < 2)
		usage(1);

	std::vector<std::string> files = withattrs(argv[1], attrargs(argc-2, argv+2));
	for (unsigned int i = 0; i < files.size(); i++) {
		printf("%s\n", files[i].c_str());
	}

	return 0;
}

static void usage(int r) {
	puts("Usage: pathfor <root> [<key>=<value>]*\n");
	exit(r);
}