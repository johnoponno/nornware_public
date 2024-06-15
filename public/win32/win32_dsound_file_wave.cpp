#include "stdafx.h"
#include "win32_dsound_file_wave.h"

#include "../minyin/fs.h"

static void __destroy(win32_dsound_file_wave_t& fws)
{
	//clear header
	::memset(&fws.header, 0, sizeof(fws.header));

	//clear wave data
	if (fws.data)
	{
		delete[] fws.data;
		fws.data = nullptr;
	}

	//clear size
	fws.size = 0;
}

static bool __parse_wave_data(
	const uint8_t* aFileData,
	win32_dsound_file_wave_t& fws)
{
	//set up pointer to the start of the chunk of memory.
	uint32_t* mem_ptr = (uint32_t*)aFileData;

	//get the type and length of the chunk of memory
	uint32_t riff = *mem_ptr++;
	uint32_t length = *mem_ptr++;
	uint32_t type = *mem_ptr++;

	//using the mmioFOURCC macro (part of Windows SDK), ensure
	//that this is a RIFF WAVE chunk of memory
	if (riff != mmioFOURCC('R', 'I', 'F', 'F'))
		return false;      // not even RIFF

	if (type != mmioFOURCC('W', 'A', 'V', 'E'))
		return false;      // not a WAV

	//find the pointer to the end of the chunk of memory
	uint32_t* mem_end = (uint32_t*)((uint8_t*)mem_ptr + length - 4);

	//run through the bytes looking for the tags
	while (mem_ptr < mem_end)
	{
		type = *mem_ptr++;
		length = *mem_ptr++;

		switch (type)
		{
			//found the format part
		case mmioFOURCC('f', 'm', 't', ' '):
			//something's wrong! Not a WAV
			if (length < sizeof(win32_wav_format_t))
				return false;

			//copy header
			::memcpy(&fws.header, mem_ptr, sizeof(win32_wav_format_ex_t));

			//cbSize should always be 0 for PCM sounds,
			//according to Patrik Larsson at DECAM and the DirectSound debug DLLs
			fws.header.extraInfoSize = 0;

			//check to see if the other two items have been
			//filled out yet (the bits and the size of the
			//bits). If so, then this chunk of memory has
			//been parsed out and we can exit
			if (fws.data && fws.size)
				return true;
			break;

			// Found the samples
		case mmioFOURCC('d', 'a', 't', 'a'):
			//allocate memory
			fws.data = new uint8_t[length];
			assert(fws.data);
			if (!fws.data)
				return false;

			//copy samples
			::memcpy(fws.data, mem_ptr, length);

			//set the size of the wave
			fws.size = length;

			//make sure we have our header pointer set up. if we do, we can exit
			if (fws.data && fws.size)
				return true;
			break;
		}	//end switch

		// Move the pointer through the chunk of memory
		mem_ptr = (uint32_t*)((uint8_t*)mem_ptr + ((length + 1) & ~1));
	}

	//failed, did not get all the pieces of the wav
	return false;
}

win32_dsound_file_wave_t::win32_dsound_file_wave_t()
{
	header = {};
	data = nullptr;
	size = 0;
}

win32_dsound_file_wave_t::~win32_dsound_file_wave_t()
{
	__destroy(*this);
}

bool win32_dsound_file_wave_t::load(const char* aFileName)
{
	//cleanup
	__destroy(*this);

	fs_blob_t contents = fs_file_contents(aFileName);
	if (!contents.data)
		return false;

	//parse wave data
	const bool result = __parse_wave_data((uint8_t*)contents.data, *this);

	//delete memory
	delete[] contents.data;

	return result;
}
