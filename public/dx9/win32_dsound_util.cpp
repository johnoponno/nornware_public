#include "stdafx.h"
#include "win32_dsound_util.h"

long win32_dsound_linear_to_directx_volume(const float aLinear)
{
	if (aLinear <= 0.f)
		return -10000;

	if (aLinear >= 1.f)
		return 0;

	return (int32_t)::floorf(2000.0f * ::log10f(aLinear) + .5f);
}

const win32_wav_format_ex_t WIN32_DSOUND_STEREO_WAV_FORMAT =
{
	1,			//wave format tag (1 = pcm)
	2,			//channels (stereo)
	44100,		//sample rate (44khz)
	176400,		//avg bytes per sec (sample rate * blockalign)
	4,			//block align (channels * bits per sample / 8)
	16,			//bits per sample (16 bit)
	0			//extra bullshit info size
};
