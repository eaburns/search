#include <sys/time.h>
#include <ctime>
#include <cerrno>

#include "../incl/utils.hpp"

double walltime(void) {
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == -1)
		fatalx(errno, "gettimeofday failed\n");

	return (double) tv.tv_sec + (double) tv.tv_usec / 1000000.0;
}

double cputime(void) {
	return clock() * CLOCKS_PER_SEC;
}
