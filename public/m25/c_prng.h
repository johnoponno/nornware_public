#pragma once

struct c_xorshift128_t
{
	static c_xorshift128_t make();
	uint32_t generate();
	int32_t int32(const int32_t in_max_plus_one);
	float float32(const float in_min, const float in_max);
	bool boolean();

	uint32_t _x;
	uint32_t _y;
	uint32_t _z;
	uint32_t _w;
};
