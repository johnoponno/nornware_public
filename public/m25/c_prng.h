#pragma once

struct c_xorshift128_t
{
	static c_xorshift128_t make(const uint32_t in_seed);
	uint32_t generate();
	int32_t int32(const int32_t in_max_plus_one);
	float float32(const float in_min, const float in_max);
	bool boolean();

	uint32_t _x;
	uint32_t _y;
	uint32_t _z;
	uint32_t _w;
};

uint32_t c_random_uint(uint32_t& prng);
float c_random_float(uint32_t& prng);
