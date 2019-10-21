#pragma once

#include "vorbisfile.h"
#include "sound_stream_source.h"

namespace sound
{
	struct vorbis_file_t : public stream_source_i
	{
		bool load_segment(uint8_t* aMemDst, const uint32_t aLength, uint32_t& aCopiedLength) override;
		bool load_wrapped_segment(uint8_t* aMemDst, const uint32_t aLength) override;
		bool rewind(const float aStartPosition) override;
		uint32_t position_bytes() override;
		uint32_t bytes_total() const override;

		static vorbis_file_t* create(const char* aFile);
		~vorbis_file_t();

	private:

		explicit vorbis_file_t();
		bool _init(const char* aFile);

		::FILE* myStream;
		OggVorbis_File myFile;
		uint32_t myBytesTotal;
	};
}
