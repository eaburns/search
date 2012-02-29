// Prints a string of the different attributes
// for the given path.

#include "rdb.hpp"
#include "../utils/utils.hpp"

int main(int argc, const char *argv[]) {
	if (argc < 2)
		fatal("Requires a path");
	RdbAttrs attrs = pathattrs(argv[1]);
	puts(attrs.string().c_str());
	return 0;
}