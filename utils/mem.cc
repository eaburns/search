#include "utils.hpp"
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

enum { Bufsz = 256 };

unsigned long virtmem(void) {
	static const char *field = "VmPeak:";
	char buf[Bufsz];

	int n = snprintf(buf, Bufsz, "/proc/%d/status", getpid());
	if (n <= 0 || n > Bufsz)
		return 0;

	if (!fileexists(buf))
		return 0;

	FILE *in = fopen(buf, "r");
	unsigned long mem = 0;

	for (;;) {
		if (!fgets(buf, Bufsz, in))
			break;
		if (!strstr(buf, field))
			continue;
		size_t skip = strspn(buf + strlen(field), " \t");
		char *strt = buf + strlen(field) + skip;
		char *end = strchr(strt, ' ');
		*end = '\0';
		mem = strtoul(strt, NULL, 10);
		break;
	}

	fclose(in);

	return mem;
}