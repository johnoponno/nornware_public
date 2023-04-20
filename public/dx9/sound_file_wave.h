#pragma once

static_assert(sizeof(unsigned long) == sizeof(uint32_t), "sizeof(unsigned long) != sizeof(uint32_t)");

#include "win32_dsound_util.h"

namespace sound
{
	struct file_wave_t
	{
		explicit file_wave_t();
		~file_wave_t();

		win32_wav_format_ex_t header;
		uint32_t size;
		uint8_t* data;
	};

	bool file_wave_load(const char* aFileName, file_wave_t& fws);
}
