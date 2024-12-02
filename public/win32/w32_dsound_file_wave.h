#pragma once

#include "w32_dsound_util.h"

struct w32_dsound_file_wave_t
{
	bool load(const char* aFileName);

	explicit w32_dsound_file_wave_t();
	~w32_dsound_file_wave_t();

	w32_wav_format_ex_t header;
	uint32_t size;
	uint8_t* data;
};
