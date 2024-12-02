#include "stdafx.h"
#include "w32_dsound_util.h"

long w32_dsound_linear_to_directx_volume(const float aLinear)
{
	if (aLinear <= 0.f)
		return -10000;

	if (aLinear >= 1.f)
		return 0;

	return (int32_t)::floorf(2000.0f * ::log10f(aLinear) + .5f);
}
