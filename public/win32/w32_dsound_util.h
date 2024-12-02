#pragma once

//OLD general waveform format structure (information common to all formats)
struct w32_wav_format_t
{
	uint16_t format_tag;				// format type 
	uint16_t num_channels;				// number of channels (i.e. mono, stereo, etc.) 
	uint32_t samples_per_second;		// sample rate 
	uint32_t average_bytes_per_second;	// for buffer estimation 
	uint16_t blockAlign;				// block size of data 
};

//extended wave format structure
struct w32_wav_format_ex_t
{
	uint16_t format_tag;				/* format type */
	uint16_t num_channels;				/* number of channels (i.e. mono, stereo...) */
	uint32_t samples_per_second;		/* sample rate */
	uint32_t average_bytes_per_second;	/* for buffer estimation */
	uint16_t blockAlign;				/* block size of data */
	uint16_t bitsPerSample;				/* number of bits per sample of mono data */
	uint16_t extraInfoSize;				/* the count in bytes of the size of */
	/* extra information (after cbSize) */
};

struct w32_dsound_length_t
{
	uint32_t in_bytes;
	float in_seconds;
};

long w32_dsound_linear_to_directx_volume(const float aLinear);

constexpr w32_wav_format_ex_t W32_DSOUND_STEREO_WAV_FORMAT =
{
	1,			//wave format tag (1 = pcm)
	2,			//channels (stereo)
	44100,		//sample rate (44khz)
	176400,		//avg bytes per sec (sample rate * blockalign)
	4,			//block align (channels * bits per sample / 8)
	16,			//bits per sample (16 bit)
	0			//extra bullshit info size
};
