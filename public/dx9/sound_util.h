#pragma once

namespace sound
{
	//OLD general waveform format structure (information common to all formats)
	struct wave_format_t
	{
		uint16_t formatTag;				// format type 
		uint16_t numChannels;			// number of channels (i.e. mono, stereo, etc.) 
		uint32_t samplesPerSecond;		// sample rate 
		uint32_t averageBytesPerSecond;	// for buffer estimation 
		uint16_t blockAlign;			// block size of data 
	};

	//extended wave format structure
	struct wave_format_ex_t
	{
		uint16_t formatTag;				/* format type */
		uint16_t numChannels;			/* number of channels (i.e. mono, stereo...) */
		uint32_t samplesPerSecond;		/* sample rate */
		uint32_t averageBytesPerSecond;	/* for buffer estimation */
		uint16_t blockAlign;			/* block size of data */
		uint16_t bitsPerSample;			/* number of bits per sample of mono data */
		uint16_t extraInfoSize;			/* the count in bytes of the size of */
		/* extra information (after cbSize) */
	};

	struct length_t
	{
		uint32_t in_bytes;
		float in_seconds;
	};

	long linear_to_direct_x_volume(const float aLinear);
	float direct_x_to_linear_volume(const long aDirectX);
	long linear_to_direct_x_pan(const float aLinear);
	float direct_x_to_linear_pan(const long aDirectX);

	extern const wave_format_ex_t stereo_wav_format;
}
