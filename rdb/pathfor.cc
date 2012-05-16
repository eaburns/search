#include "rdb.hpp"
#include <cstring>
#include <cstdio>

void fatal(const char*, ...);

void usage();
int nextpair(int, char *[], std::string&, std::string&);

int main(int argc, const char *argv[]) {
	if (argc < 2)
		usage();

	const char *root = argv[1];
	RdbAttrs attrs = attrargs(argc-2, argv+2);

	printf("%s\n", pathfor(root, attrs).c_str());
	return 0;
}

void usage() {
	fatal("Usage: pathfor <root> <key>=<value>*\n");
}