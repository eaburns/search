#include "../utils/utils.hpp"
#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <cerrno>
#include <cstring>
#include <cassert>
#include <sys/utsname.h>
#include <unistd.h>
#include <sys/stat.h>

enum { Bufsz = 256 };

static const char *start4 = "#start data file format 4\n";
static const char *end4 = "#end data file format 4\n";

static void dfpair_sz(FILE*, unsigned int, const char*, const char*, va_list);
static void machineid(FILE*);
static void tryprocstatus(FILE*);
static char *gettoken(unsigned int, char*);

void dfpair(FILE *f, const char *key, const char *fmt, ...) {
	char buf[Bufsz];
	int n = snprintf(buf, Bufsz, "#pair  \"%s\"\t\"", key);
	if (n > Bufsz)
		fatal("dfrowhdr: buffer is too small\n");

	va_list ap;
	va_start(ap, fmt);
	unsigned int m = vsnprintf(buf+n, Bufsz-n, fmt, ap);
	va_end(ap);

	if (m > (unsigned int) Bufsz-n) {	// Err, overflow
		va_start(ap, fmt);
		dfpair_sz(f, m + n + 1, key, fmt, ap);
		va_end(ap);
		return;
	}

	fprintf(f, "%s\"\n", buf);
	fflush(f);
}

static void dfpair_sz(FILE *f, unsigned int sz, const char *key, const char *fmt, va_list ap) {
	char *buf = (char*) malloc(sz * sizeof(*buf));

	unsigned int n = snprintf(buf, sz, "#pair  \"%s\"\t\"", key);
	assert (n <= sz);
	vsnprintf(buf+n, sz-n, fmt, ap);

	fprintf(f, "%s\"\n", buf);
	free(buf);
}

void dfrowhdr(FILE *f, const char *name, int ncols, ...) {
	char buf[Bufsz];
	int n = snprintf(buf, Bufsz, "#altcols  \"%s\"", name);
	if (n > Bufsz)
		fatal("dfrowhdr: buffer is too small\n");

	va_list ap;
	va_start(ap, ncols);
	for (int i = 0; i < ncols; i++) {
		char *col = va_arg(ap, char*);
		unsigned int m = snprintf(buf+n, Bufsz-n, "\t\"%s\"", col);
		if (m > (unsigned int) Bufsz - n)
			fatal("dfrowhdr: buffer is too small\n");
		n += m;
	}
	va_end(ap);

	fprintf(f, "%s\n", buf);
}

void dfrow(FILE *f, const char *name, const char *colfmt, ...) {
	char buf[Bufsz];
	int n = snprintf(buf, Bufsz, "#altrow  \"%s\"", name);
	if (n > Bufsz)
		fatal("dfrowhdr: buffer is too small\n");

	va_list ap;
	va_start(ap, colfmt);
	for (unsigned int i = 0; i < strlen(colfmt); i++) {
		double g = 0;
		long d = 0;
		unsigned long u = 0;
		unsigned int m = 0;
		switch (colfmt[i]) {
		case 'g':
			g = va_arg(ap, double);
			m = snprintf(buf+n, Bufsz-n, "\t%g", g);
			break;
		case 'f':
			g = va_arg(ap, double);
			m = snprintf(buf+n, Bufsz-n, "\t%lf", g);
			break;
		case 'd':
			d = va_arg(ap, long);
			m = snprintf(buf+n, Bufsz-n, "\t%ld", d);
			break;
		case 'u':
			u = va_arg(ap, unsigned long);
			m = snprintf(buf+n, Bufsz-n, "\t%lu", u);
			break;
		}
		if (m > (unsigned int) Bufsz-n)
			fatal("dfrow: buffer is too small\n");
		n += m;
	}
	va_end(ap);

	fprintf(f, "%s\n", buf);
	fflush(f);
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
	tryprocstatus(f);
	fputs(end4, f);
}

void dfprocstatus(FILE *f) {
	tryprocstatus(f);
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

static void tryprocstatus(FILE *out)
{
	static const char *field = "VmPeak:";
	static const char *key = "max virtual kilobytes";
	char buf[Bufsz];

	int n = snprintf(buf, Bufsz, "/proc/%d/status", getpid());
	if (n <= 0 || n > Bufsz)
		return;

	struct stat sb;
	if (stat(buf, &sb) < 0)
		return;

	FILE *in = fopen(buf, "r");

	for (;;) {
		if (!fgets(buf, Bufsz, in))
			break;
		if (!strstr(buf, field))
			continue;
		size_t skip = strspn(buf + strlen(field), " \t");
		char *strt = buf + strlen(field) + skip;
		char *end = strchr(strt, ' ');
		*end = '\0';
		dfpair(out, key, "%s", strt);
		break;
	}

	fclose(in);
}

void dfread(FILE *in, Dfhandler seeline, void *priv, bool echo) {
	unsigned int lineno = 1;
	unsigned int sz = 0;
	char *linebuf = NULL;
	std::vector<const char*> toks;
	boost::optional<std::string> line = readline(in, echo);

	while (line) {
		if (sz < line->size() + 1) {
			if (linebuf)
				free(linebuf);
			linebuf = strdup(line->c_str());
			sz = line->size() + 1;
		} else {
			strcpy(linebuf, line->c_str());
		}

		toks.clear();
		if (hasprefix(linebuf, "#pair"))
			toks.push_back("#pair");

		else if (hasprefix(linebuf, "#altcols"))
			toks.push_back("#altcols");

		else if (hasprefix(linebuf, "#altrow"))
			toks.push_back("#altrow");

		if (toks.size() > 0) {
			const char *end = linebuf + strlen(linebuf);
			char *left = linebuf + strlen(toks[0]);
			while (left < end) {
				char *vl = gettoken(lineno, left);
				if (!vl)
					break;
				toks.push_back(vl);
				left = vl + strlen(vl) + 1;
			}
			seeline(toks, priv);
		}
		lineno++;
		line = readline(in, echo);
	}

	free(linebuf);
	return;
}

// gettoken returns the first whitespace delimited or quoted
// token from the string.  This routine  modifies the given
// string, returns NULL if end of line is encountered before a
// token.
static char *gettoken(unsigned int lineno, char *str) {
	unsigned int i;
	for (i = 0; i < strlen(str) && isspace(str[i]); i++)
		;
	if (i >= strlen(str))
		return NULL;

	if (str[i] == '"') {
		char *strt = str + i + 1;
		for (i = 0; i < strlen(strt) && strt[i] != '"'; i++)
			;
		if (i == strlen(strt) || strt[i] != '"')
			fatal("line %u: No closing quote\n", lineno);
		strt[i] = '\0';
		return strt;
	}

	char *strt = str + i;
	for (i = 0; i < strlen(strt) && !isspace(strt[i]); i++)
		;
	strt[i] = '\0';

	return strt;	
}