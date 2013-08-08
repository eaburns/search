// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cstdlib>

enum { Bufsz = 256 };

void warn(const char *fmt, ...) {
	char mbuf[Bufsz];

	va_list args;
	va_start(args, fmt);
	vsnprintf(mbuf, Bufsz, fmt, args);
	va_end(args);

	fprintf(stderr, "%s\n", mbuf);
}

void warnx(int err, const char *fmt, ...) {
	char mbuf[Bufsz];

	va_list args;
	va_start(args, fmt);
	vsnprintf(mbuf, Bufsz, fmt, args);
	va_end(args);

	warn("%s: %s", mbuf, strerror(err));
}

void fatal(const char *fmt, ...) {
	char mbuf[Bufsz];

	va_list args;
	va_start(args, fmt);
	vsnprintf(mbuf, Bufsz, fmt, args);
	va_end(args);

	fprintf(stderr, "%s\n", mbuf);
	exit(1);
}

void fatalx(int err, const char *fmt, ...) {
	char mbuf[Bufsz];

	va_list args;
	va_start(args, fmt);
	vsnprintf(mbuf, Bufsz, fmt, args);
	va_end(args);

	fatal("%s: %s", mbuf, strerror(err));
}