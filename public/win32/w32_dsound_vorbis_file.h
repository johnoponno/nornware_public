#pragma once

#include "vorbis_file.h"
#include "w32_dsound_stream_source.h"

struct w32_dsound_vorbis_file_t : public w32_dsound_stream_source_i
{
	bool w32_dsound_stream_source_load_segment(uint8_t* aMemDst, const uint32_t aLength, uint32_t& aCopiedLength) override;
	bool w32_dsound_stream_source_load_wrapped_segment(uint8_t* aMemDst, const uint32_t aLength) override;
	bool w32_dsound_stream_source_rewind(const float aStartPosition) override;
	uint32_t w32_dsound_stream_source_position_bytes() override;
	uint32_t w32_dsound_stream_source_bytes_total() const override;

	static w32_dsound_vorbis_file_t* create(const char* aFile);
	~w32_dsound_vorbis_file_t();

private:

	explicit w32_dsound_vorbis_file_t();
	bool _init(const char* aFile);

	::FILE* _stream;
	OggVorbis_File _file;
	uint32_t _bytes_total;
};
