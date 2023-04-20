#pragma once

static_assert(sizeof(unsigned long) == sizeof(uint32_t), "sizeof(unsigned long) != sizeof(uint32_t)");

#include "win32_dsound_util.h"

struct win32_dsound_file_wave_t
{
	bool load(const char* aFileName);

	explicit win32_dsound_file_wave_t();
	~win32_dsound_file_wave_t();

	win32_wav_format_ex_t header;
	uint32_t size;
	uint8_t* data;
};
