#include "plat2d.hpp"
#include "../utils/utils.hpp"
#include <cerrno>
#include <cstdio>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		puts("Usage: makeh <outfile>");
		return 1;
	}

	Plat2d p(stdin);

	FILE *out = fopen(argv[1], "w");
	if (!out)
		fatalx(errno, "Failed to open %s for writing", argv[1]);

	p.vg->output(out);
	fprintf(out, "\n");
	for (unsigned int i = 0; i < p.centers.size(); i++)
		fprintf(out, "%ld\n", p.centers[i]);

	fclose(out);

	return 0;
}
