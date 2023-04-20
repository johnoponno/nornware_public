#include "stdafx.h"
#include "win32_dsound_vorbis_file.h"

//local
static size_t __read(void* ptr, size_t size, size_t nmemb, void* datasource)
{
	/*
	fs::read_file_t* handle = (fs::read_file_t*)datasource;

	return handle->impl->read_report(ptr, size * nmemb);
	*/

	::FILE* handle = (::FILE*)datasource;
	return ::fread(ptr, sizeof(uint8_t), size * nmemb, handle);
}

static int32_t __seek(void* datasource, ogg_int64_t offset, int32_t whence)
{
	/*
	fs::read_file_t* handle = (fs::read_file_t*)datasource;

	if (handle->impl->seek((long)offset, whence))
	{
		return 0;
	}
	else
	{
		return -1;
	}
	*/

	::FILE* handle = (::FILE*)datasource;
	return ::fseek(handle, (long)offset, whence);
}

static int32_t __close(void* datasource)
{
	/*
	fs::read_file_t* handle = (fs::read_file_t*)datasource;

	read_file_close(*handle);

	return 0;
	*/

	::FILE* handle = (::FILE*)datasource;
	::fclose(handle);

	return 0;
}

static long __tell(void* datasource)
{
	/*
	fs::read_file_t* handle = (fs::read_file_t*)datasource;

	return handle->impl->tell();
	*/

	::FILE* handle = (::FILE*)datasource;
	return ::ftell(handle);
}

//IStreamFile implementation
bool win32_dsound_vorbis_file_t::win32_dsound_stream_source_load_wrapped_segment(uint8_t* aMemDst, const uint32_t aLength)
{
	uint32_t decoded = 0;
	int32_t status;
	int32_t currentSection;

	while (decoded < aLength)
	{
		status = ::ov_read(&_file, (char*)(aMemDst + decoded), aLength - decoded, 0, 2, 1, &currentSection);
		if (status == OV_HOLE || status == OV_EBADLINK)	//errors
		{
			return false;
		}
		else if (status == 0)	//eof
		{
			if (!win32_dsound_stream_source_rewind(0.f))
				return false;
		}
		else	//decoded bytes
		{
			assert(status > 0);
			decoded += status;
		}
	}

	return true;
}

bool win32_dsound_vorbis_file_t::win32_dsound_stream_source_load_segment(uint8_t* aMemDst, const uint32_t aLength, uint32_t& aCopiedLength)
{
	int32_t status;
	int32_t currentSection;

	aCopiedLength = 0;

	while (aCopiedLength < aLength)
	{
		status = ::ov_read(&_file, (char*)(aMemDst + aCopiedLength), aLength - aCopiedLength, 0, 2, 1, &currentSection);
		if (status == OV_HOLE || status == OV_EBADLINK)	//errors
		{
			return false;
		}
		else if (status == 0)	//eof
		{
			return true;
		}
		else	//decoded bytes
		{
			assert(status > 0);
			aCopiedLength += status;
		}
	}

	return true;
}

bool win32_dsound_vorbis_file_t::win32_dsound_stream_source_rewind(const float aStartPosition)
{
	const double LENGTH = ::ov_time_total(&_file, -1);
	if (OV_EINVAL == LENGTH)
		return false;	//failed to get length

	//start position modulo length...
	const double SP = ::fmod((double)aStartPosition, LENGTH);
	return ::ov_time_seek(&_file, SP) == 0;
}

uint32_t win32_dsound_vorbis_file_t::win32_dsound_stream_source_position_bytes()
{
	const ogg_int64_t PCM_POS = ov_pcm_tell(&_file);
	if (OV_EINVAL != PCM_POS)
		return (uint32_t)PCM_POS * output_format.blockAlign;

	return 0;
}

uint32_t win32_dsound_vorbis_file_t::win32_dsound_stream_source_bytes_total() const
{
	return _bytes_total;
}

//specific interface
win32_dsound_vorbis_file_t* win32_dsound_vorbis_file_t::create(const char* aFile)
{
	win32_dsound_vorbis_file_t* f = new win32_dsound_vorbis_file_t;

	if (f && !f->_init(aFile))
	{
		delete f;
		f = nullptr;
	}

	return f;
}

win32_dsound_vorbis_file_t::~win32_dsound_vorbis_file_t()
{
	::ov_clear(&_file);
}

//private implementation
win32_dsound_vorbis_file_t::win32_dsound_vorbis_file_t()
	:win32_dsound_stream_source_i(4096)
{
	_stream = nullptr;
	::memset(&_file, 0, sizeof(_file));
}

bool win32_dsound_vorbis_file_t::_init(const char* aFile)
{
	//stream
	/*
	if (!read_file_open_binary(impl, aFile, myStream))
	{
		//FS_LOG("failed to open '%s'", aFile);
		return false;
	}
	*/
	if (0 != ::fopen_s(&_stream, aFile, "rb"))
		return false;

	//ogg vorbis file
	if (::ov_open_callbacks(_stream, &_file, nullptr, 0, { &__read, &__seek, &__close, &__tell, }))
	{
		//FS_LOG("failed to initiliaze with '%s', not an ogg file?", aFile);
		return false;
	}

	//info
	vorbis_info* info = ::ov_info(&_file, -1);

	//output format
	output_format.format_tag = 1;	//pcm

	assert(0xffff >= info->channels);
	output_format.num_channels = (uint16_t)info->channels;

	output_format.samples_per_second = info->rate;
	output_format.bitsPerSample = 16;
	output_format.blockAlign = (output_format.num_channels * output_format.bitsPerSample) / 8;
	output_format.average_bytes_per_second = output_format.samples_per_second * output_format.blockAlign;
	output_format.extraInfoSize = 0;		//should be 0

	//cache total size
	ogg_int64_t PCM_TOTAL = ::ov_pcm_total(&_file, -1);
	if (OV_EINVAL != PCM_TOTAL)
		_bytes_total = (uint32_t)PCM_TOTAL * output_format.blockAlign;

	return true;
}
