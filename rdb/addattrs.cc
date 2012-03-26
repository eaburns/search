// addattrs adds the attributes specified on the command-line
// to the datafile just before the first #pair key in the datafile.

#include "rdb.hpp"
#include "../utils/utils.hpp"
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <string>

static void usage(int);

int main(int argc, const char *argv[]) {
	if (argc < 2)
		usage(1);

	const char *file = argv[1];
	RdbAttrs attrs = attrargs(argc-2, argv+2);
	if (attrs.size() == 0)
		return 0;

	FILE *fin = fopen(file, "r");
	if (!fin)
		fatalx(errno, "Failed to open %s for reading", file);

	std::string outfile = std::string(file) + '~';
	FILE *fout = fopen(outfile.c_str(), "w");
	if (!fout)
		fatalx(errno, "Failed to open %s for writing", outfile.c_str());

	boost::optional<std::string> l = readline(fin);
	bool added = false;
	while (l) {
		if (!added && hasprefix(l->c_str(), "#pair")) {
			added = true;
			while (attrs.size() > 0) {
				std::string key = attrs.front();
				std::string val = attrs.lookup(key);
				attrs.pop_front();
				dfpair(fout, key.c_str(), "%s", val.c_str());
			}
		}
		fputs((*l + '\n').c_str(), fout);
		l = readline(fin);
	}

	fclose(fin);
	fclose(fout);

	if (rename(outfile.c_str(), file) == -1)
		fatalx(errno, "Failed to rename %s to %s", outfile.c_str(), file);
	

	return 0;
}

static void usage(int r) {
	puts("Usage: addattrs <datafile> [<key>=<value>]*");
	exit(r);
}