// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include <sys/time.h>
#include <ctime>
#include <cerrno>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include "../utils/utils.hpp"

double walltime() {
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == -1)
		fatalx(errno, "gettimeofday failed");

	return (double) tv.tv_sec + (double) tv.tv_usec / 1000000.0;
}

double cputime() {
	struct timespec ts;

#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	ts.tv_sec = mts.tv_sec;
	ts.tv_nsec = mts.tv_nsec;
#else
	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) < 0)
		fatalx(errno, "clock_gettime failed");
#endif

	return ts.tv_sec + (double) ts.tv_nsec/(double) 1e9;
}
