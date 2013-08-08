// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include <sys/time.h>
#include <ctime>
#include <cerrno>

#include "../utils/utils.hpp"

double walltime() {
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == -1)
		fatalx(errno, "gettimeofday failed");

	return (double) tv.tv_sec + (double) tv.tv_usec / 1000000.0;
}

double cputime() {
	struct timespec ts;
	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) < 0)
		fatalx(errno, "clock_gettime failed");
	return ts.tv_sec + (double) ts.tv_nsec/(double) 1e9;
}
