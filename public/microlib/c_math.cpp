#include "stdafx.h"

#include "c_vector.h"

static bool __float_in_range(const float in_value, const float in_min, const float in_max)
{
	return in_min <= in_value && in_value <= in_max;
}

//public
//public
//public

uint32_t c_frame(const float aNow, const float anFps, const uint32_t aNumFrames)
{
	if (anFps == 0.f)
		return 0;

	return (uint32_t)(aNow * anFps) % aNumFrames;
}

bool c_inside_rectf(const c_vec2f_t& aPosition, const c_vec2f_t& aMin, const c_vec2f_t& aMax)
{
	return
		__float_in_range(aPosition.x, __min(aMin.x, aMax.x), __max(aMin.x, aMax.x)) &&
		__float_in_range(aPosition.y, __min(aMin.y, aMax.y), __max(aMin.y, aMax.y));
}

void c_int32_min_max(int32_t& aMin, int32_t& aMax)
{
	if (aMin > aMax)
	{
		const int32_t TEMP = aMin;
		aMin = aMax;
		aMax = TEMP;
	}
}
