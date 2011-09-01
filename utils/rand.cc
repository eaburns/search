#include "../incl/utils.hpp"
#include <stdint.h>

static const uint64_t Mul = 2685821657736338717LL;
static const uint64_t Vini = 4101842887655102017LL;
static const double Fl = 5.42101086242752217e-20;

Rand::Rand(unsigned long seed) {
	v = Vini ^ seed;
	v = bits();
}

unsigned long Rand::bits(void)
{
	v ^= v >> 21;
	v ^= v << 35;
	v ^= v >> 4;
	v *= Mul;
	return v;
}

long Rand::integer(long min, long max)
{
	if (min == max)
		return min;
	
	return bits() % (max - min) + min;
}