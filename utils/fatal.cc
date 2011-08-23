#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cstdlib>

void fatal(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(1);
}

enum { Bufsz = 256 };

void fatalx(int err, const char *fmt, ...)
{
	char mbuf[Bufsz];

	va_list args;
	va_start(args, fmt);
	vsnprintf(mbuf, Bufsz, fmt, args);
	va_end(args);

	fatal("%s: %s", mbuf, strerror(err));
}