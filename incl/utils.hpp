#include <cstdio>

void warn(const char *, ...);
void warnx(int, const char *, ...);
void fatal(const char*, ...);
void fatalx(int, const char*, ...);

double walltime(void);
double cputime(void);

void dfpair(FILE *, const char *key, const char *fmt, ...);
void dfrowhdr(FILE *, const char *name, int ncols, ...);
// colfmt is a string of characters: g, f, d, u, s, c and b with the
// following meaning:
// g is a %g formatted double,
// f is a %f formatted double
// d is a %ld formatted long
// u is a %lu formatted unsigned long
// s is a %s formatted string
// c is a %c formatted char
// b is a "true"/"false" formatted bool
void dfrow(FILE *, const char *name, const char *colfmt, ...);
void dfheader(FILE *);
void dffooter(FILE *);