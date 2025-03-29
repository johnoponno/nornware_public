#include "stdafx.h"
#include "c_prng.h"

static uint32_t __uint32(
	const uint32_t in_max_plus_one, 
	c_xorshift128_t& prng)
{
	if (in_max_plus_one)
		return prng.generate() % in_max_plus_one;

	return 0;
}

//public
//public
//public
//public

c_xorshift128_t c_xorshift128_t::make(const uint32_t in_seed)
{
	c_xorshift128_t result;
	if (0 == in_seed)
		result._x = 123456789;
	else
		result._x = in_seed;
	result._y = 362436069;
	result._z = 521288629;
	result._w = 88675123;
	return result;
}

uint32_t c_xorshift128_t::generate()
{
	//check that we actually initialized the state (legacy behaviour from the default constructor)
	assert(0 != _x || 0 != _y || 0 != _z || 0 != _w);
	//check that we actually initialized the prng (legacy behaviour from the default constructor)

	const uint32_t T = _x ^ (_x << 11);
	_x = _y;
	_y = _z;
	_z = _w;
	return _w = _w ^ (_w >> 19) ^ T ^ (T >> 8);
}

int32_t c_xorshift128_t::int32(const int32_t in_max_plus_one)
{
	if(in_max_plus_one > 0)
		return __uint32(in_max_plus_one, *this);

	return 0;
}

float c_xorshift128_t::float32(const float in_min, const float in_max)
{
	return __uint32(10000, *this) * .0001f * (in_max - in_min) + in_min;
}

bool c_xorshift128_t::boolean()
{
	return 0 != (this->generate() % 2);
}

uint32_t c_random_uint(uint32_t& prng)
{
	prng = 1664525l * prng + 1013904223l;
	uint32_t rf = 0x3f800000 | (prng & 0x007fffff);
	return rf;
}

float c_random_float(uint32_t& prng)
{
	prng = 1664525l * prng + 1013904223l;
	uint32_t rf = 0x3f800000 | (prng & 0x007fffff);
	return (*(float*)&rf) - 1.f;
}
