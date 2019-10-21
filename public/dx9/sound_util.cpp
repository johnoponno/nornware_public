#include "stdafx.h"
#include "sound_util.h"

namespace sound
{
	//#define LEGACY

	long linear_to_direct_x_volume(const float aLinear)
	{
#ifdef LEGACY
		if (aLinear <= 0.1f)
			return -10000;

		if (aLinear >= 1.f)
			return 0;

		const long RET((long)(-10000.f * ::log10f(1.f / aLinear)));
		assert(RET <= 0 && RET >= -10000);
		return RET;
#else
		if (aLinear <= 0.f)
			return -10000;

		if (aLinear >= 1.f)
			return 0;

		return (int32_t)::floorf(2000.0f * ::log10f(aLinear) + .5f);
#endif
	}

	float direct_x_to_linear_volume(const long aDirectX)
	{
#ifdef LEGACY
		if (aLogVolume <= MINVOLUME)
			return 0.1f;

		if (aLogVolume > 0)
			return 1.f;

		const float RET(1.f / ::pow(10.f, aLogVolume / MINVOLUME));
		assert(RET >= 0.f && RET <= 1.f);
		return RET;
#else
		if (aDirectX <= -10000)
			return 0.f;

		if (aDirectX >= 0)
			return 1.f;

		return ::powf(10.0f, (float)aDirectX / 2000.f);
#endif
	}

	long linear_to_direct_x_pan(const float aLinear)
	{
		float p = aLinear;

		if (p < -1.f)
			p = -1.f;
		else if (p > 1.f)
			p = 1.f;

		//exponential
		p = ::powf(p, 3.f);

		return (long)(p * 10000.f);
	}

	float direct_x_to_linear_pan(const long aDirectX)
	{
		if (aDirectX <= -10000)
			return -1.f;

		if (aDirectX >= 10000)
			return 1.f;

		const float LINEAR(::powf((float)::abs(aDirectX) / 10000.f, 1.f / 3.f));
		if (aDirectX < 0)
			return -LINEAR;

		return LINEAR;

	}

	const wave_format_ex_t stereo_wav_format =
	{
		1,			//wave format tag (1 = pcm)
		2,			//channels (stereo)
		44100,		//sample rate (44khz)
		176400,		//avg bytes per sec (sample rate * blockalign)
		4,			//block align (channels * bits per sample / 8)
		16,			//bits per sample (16 bit)
		0			//extra bullshit info size
	};
}
