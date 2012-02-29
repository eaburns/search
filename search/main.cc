#include <cstring>

bool headerfooter(int argc, const char *argv[]) {
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-ocamlrun") == 0)
			return false;
	}
	return true;
}