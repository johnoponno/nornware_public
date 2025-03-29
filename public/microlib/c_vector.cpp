#include "stdafx.h"
#include "c_vector.h"

#define LENGTH_2D(x, y) ::sqrtf((x) * (x) + (y) * (y))

c_vec2f_t c_vec2f(const c_vec2i_t& in_vector)
{
	return{ (float)in_vector.x, (float)in_vector.y };
}

float c_length(const c_vec2f_t& in_vector)
{
	return LENGTH_2D(in_vector.x, in_vector.y);
}

c_vec2f_t c_rotate(const float in_angle, const c_vec2f_t& in_vector)
{
	if (0.f == in_angle)
		return in_vector;

	const float SIN = ::sinf(in_angle);
	const float COS = ::cosf(in_angle);

	return
	{
		in_vector.x * COS + in_vector.y * SIN,
		in_vector.y * COS - in_vector.x * SIN
	};
}

c_vec2f_t c_normalize(const c_vec2f_t& in_vector)
{
	const float LENGTH = LENGTH_2D(in_vector.x, in_vector.y);
	if (LENGTH)
	{
		const float OOL = 1.f / LENGTH;
		return { in_vector.x * OOL, in_vector.y * OOL };
	}

	//if no length, don't change the input
	return in_vector;
}

c_vec2i_t c_floor_to_int(const c_vec2f_t& in_vector)
{
	c_vec2i_t i
	{
		(int32_t)in_vector.x,
		(int32_t)in_vector.y
	};

	if (in_vector.x < 0.f)
		--i.x;

	if (in_vector.y < 0.f)
		--i.y;

	return i;
}
