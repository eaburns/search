// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "traffic.hpp"
#include "../search/main.hpp"
#include <cstdio>
#include <cerrno>

static SearchAlgorithm<Traffic> *get(int, const char*[]);

int main(int argc, const char *argv[]) {
	dfheader(stdout);

	FILE *inst = stdin;
	const char *instpath = "";
	for (int i = 0; i < argc; i++) {
		if (i < argc - 1 && strcmp(argv[i], "-lvl") == 0)
			instpath = argv[++i];
	}

	if (instpath[0] != '\0') {
		inst = fopen(instpath, "r");
		if (!inst)
			fatalx(errno, "Failed to open %s for reading", instpath);
	}

	Traffic d(inst);

	if (instpath[0] != '\0') {
		dfpair(stdout, "level", "%s", instpath);
		fclose(inst);
 	}

	searchGet<Traffic>(get, d, argc, argv);

	dffooter(stdout);

	return 0;
}

static SearchAlgorithm<Traffic> *get(int argc, const char *argv[]) {
	return getsearch<Traffic>(argc, argv);
}
