#pragma once

#include "win32_dsound_util.h"

struct win32_dsound_stream_source_i
{
	virtual ~win32_dsound_stream_source_i()
	{
	}

	virtual bool win32_dsound_stream_source_load_segment(uint8_t* aMemDst, const uint32_t aLength, uint32_t& aCopiedLength) = 0;
	virtual bool win32_dsound_stream_source_load_wrapped_segment(uint8_t* aMemDst, const uint32_t aLength) = 0;
	virtual bool win32_dsound_stream_source_rewind(const float aStartPosition) = 0;
	virtual uint32_t win32_dsound_stream_source_position_bytes() = 0;
	virtual uint32_t win32_dsound_stream_source_bytes_total() const = 0;

	const uint32_t FRAME_SIZE;
	win32_wav_format_ex_t output_format;

protected:

	explicit win32_dsound_stream_source_i(const uint32_t in_frame_size)
		:FRAME_SIZE(in_frame_size)
	{
		output_format = {};
	}
};
