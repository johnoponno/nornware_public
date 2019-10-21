#pragma once

#include "sound_util.h"

namespace sound
{
	struct stream_source_i
	{
		virtual ~stream_source_i()
		{
		}

		virtual bool load_segment(uint8_t* aMemDst, const uint32_t aLength, uint32_t& aCopiedLength) = 0;
		virtual bool load_wrapped_segment(uint8_t* aMemDst, const uint32_t aLength) = 0;
		virtual bool rewind(const float aStartPosition) = 0;
		virtual uint32_t position_bytes() = 0;
		virtual uint32_t bytes_total() const = 0;

		const uint32_t frame_size;
		wave_format_ex_t output_format;

	protected:

		explicit stream_source_i(const uint32_t aFrameSize)
			:frame_size(aFrameSize)
		{
			output_format = {};
		}
	};
}
