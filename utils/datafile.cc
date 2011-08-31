#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <cerrno>
#include <cstring>
#include <sys/utsname.h>
#include <unistd.h>
#include "../incl/utils.hpp"

enum { Bufsz = 256 };

static const char *start4 = "#start data file format 4\n";
static const char *end4 = "#end data file format 4\n";

static void machineid(FILE *);

void dfpair(FILE *f, const char *key, const char *fmt, ...) {
	char buf[Bufsz];

	int n = snprintf(buf, Bufsz, "#pair \"%s\"\t\"", key);

	va_list ap;
	va_start(ap, fmt);
	n += vsnprintf(buf+n, Bufsz-n, fmt, ap);
	va_end(ap);

	fprintf(f, "%s\"\n", buf);
}

void dfrowhdr(FILE *f, const char *name, int ncols, ...) {
	char buf[Bufsz];
	int n = snprintf(buf, Bufsz, "#altcols  \"%s\"", name);

	va_list ap;
	va_start(ap, ncols);
	for (int i = 0; i < ncols; i++) {
		char *col = va_arg(ap, char*);
		n += snprintf(buf+n, Bufsz-n, "\t\"%s\"", col);
	}
	va_end(ap);

	fprintf(f, "%s\n", buf);
}

void dfrow(FILE *f, const char *name, const char *colfmt, ...) {
	char buf[Bufsz];
	int n = snprintf(buf, Bufsz, "#altrow  \"%s\"", name);

	va_list ap;
	va_start(ap, colfmt);
	for (unsigned int i = 0; i < strlen(colfmt); i++) {
		double g = 0;
		long d = 0;
		unsigned long u = 0;
		const char *s = "";
		int c = 0;
		switch (colfmt[i]) {
		case 'g':
			g = va_arg(ap, double);
			n += snprintf(buf+n, Bufsz-n, "\t\"%g\"", g);
			break;
		case 'f':
			g = va_arg(ap, double);
			n += snprintf(buf+n, Bufsz-n, "\t\"%lf\"", g);
			break;
		case 'd':
			d = va_arg(ap, long);
			n += snprintf(buf+n, Bufsz-n, "\t\"%ld\"", d);
			break;
		case 'u':
			u = va_arg(ap, unsigned long);
			n += snprintf(buf+n, Bufsz-n, "\t\"%lu\"", u);
			break;
		case 's':
			s = va_arg(ap, const char *);
			n += snprintf(buf+n, Bufsz-n, "\t\"%s\"", s);
			break;
		case 'c':
			c = va_arg(ap, int);
			n += snprintf(buf+n, Bufsz-n, "\t\"%c\"", c);
			break;
		case 'b':
			c = va_arg(ap, int);
			n += snprintf(buf+n, Bufsz-n, "\t\"%s\"", c ? "true" : "false");
			break;
		}
	}
	va_end(ap);

	fprintf(f, "%s\n", buf);
}

void dfheader(FILE *f) {
	fputs(start4, f);

	time_t tm;
	time(&tm);
	char *tstr = ctime(&tm);
	tstr[strlen(tstr)-1] = '\0';
	dfpair(f, "wall start date", "%s", tstr);

	dfpair(f, "wall start time", "%g", walltime());
	machineid(f);
}

void dffooter(FILE *f) {
	dfpair(f, "wall finish time", "%g", walltime());

	time_t tm;
	time(&tm);
	char *tstr = ctime(&tm);
	tstr[strlen(tstr)-1] = '\0';
	dfpair(f, "wall finish date", "%s", tstr);
	fputs(end4, f);
}

static void machineid(FILE *f) {
	char hname[Bufsz];
	memset(hname, '\0', Bufsz);
	if (gethostname(hname, Bufsz) == -1) {
		warnx(errno, "gethostname failed, unable to print machine id\n");
		return;
	}

	struct utsname u;
	if (uname(&u) == -1) {
		warnx(errno, "uname failed\n");
		dfpair(f, "machine id", "%s", hname);
	}

	dfpair(f, "machine id", "%s-%s-%s-%s", hname, u.sysname, u.release, u.machine);
}