#pragma once

#include "vorbis_file.h"
#include "win32_dsound_stream_source.h"

struct win32_dsound_vorbis_file_t : public win32_dsound_stream_source_i
{
	bool win32_dsound_stream_source_load_segment(uint8_t* aMemDst, const uint32_t aLength, uint32_t& aCopiedLength) override;
	bool win32_dsound_stream_source_load_wrapped_segment(uint8_t* aMemDst, const uint32_t aLength) override;
	bool win32_dsound_stream_source_rewind(const float aStartPosition) override;
	uint32_t win32_dsound_stream_source_position_bytes() override;
	uint32_t win32_dsound_stream_source_bytes_total() const override;

	static win32_dsound_vorbis_file_t* create(const char* aFile);
	~win32_dsound_vorbis_file_t();

private:

	explicit win32_dsound_vorbis_file_t();
	bool _init(const char* aFile);

	::FILE* _stream;
	OggVorbis_File _file;
	uint32_t _bytes_total;
};
