#include "stdafx.h"

#include "win32_dsound_engine.h"
#include "win32_dsound_util.h"

#ifdef _DEBUG
win32_dsound_engine_t::~win32_dsound_engine_t()
{
	assert(nullptr == this->primary_buffer);
	assert(nullptr == this->directsound);
}
#endif

bool win32_dsound_engine_t::init(const ::HWND window)
{
	::HRESULT err;

	//create directsound object
	assert(nullptr == directsound);
	err = ::DirectSoundCreate(nullptr, &directsound, nullptr);
	if (FAILED(err))
		return false;

	//set cooperative level
	err = directsound->SetCooperativeLevel(window, DSSCL_PRIORITY);
	if (FAILED(err))
		return false;

	//set up DSBUFFERDESC structure.for a streaming secondary buffer
	::DSBUFFERDESC dsbd{};	//directsound buffer description
	dsbd.dwSize = sizeof(::DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsbd.dwBufferBytes = 0;		//must be 0 for primary buffer
	dsbd.lpwfxFormat = nullptr;	//must be nullptr for primary buffer

	//try to create buffer
	assert(nullptr == primary_buffer);
	err = directsound->CreateSoundBuffer(&dsbd, &primary_buffer, nullptr);
	if (FAILED(err))
		return false;

	//check buffer format
	::WAVEFORMATEX wav_format;
	err = primary_buffer->GetFormat(&wav_format, sizeof(wav_format), nullptr);
	if (FAILED(err))
		return false;

	//make sure is our desired format
	if (wav_format.wFormatTag != WIN32_DSOUND_STEREO_WAV_FORMAT.format_tag ||
		wav_format.nChannels != WIN32_DSOUND_STEREO_WAV_FORMAT.num_channels ||
		wav_format.nSamplesPerSec != WIN32_DSOUND_STEREO_WAV_FORMAT.samples_per_second ||
		wav_format.nAvgBytesPerSec != WIN32_DSOUND_STEREO_WAV_FORMAT.average_bytes_per_second ||
		wav_format.nBlockAlign != WIN32_DSOUND_STEREO_WAV_FORMAT.blockAlign ||
		wav_format.wBitsPerSample != WIN32_DSOUND_STEREO_WAV_FORMAT.bitsPerSample ||
		wav_format.cbSize != WIN32_DSOUND_STEREO_WAV_FORMAT.extraInfoSize)
	{
		err = primary_buffer->SetFormat((::LPCWAVEFORMATEX)&WIN32_DSOUND_STEREO_WAV_FORMAT);
		if (FAILED(err))
			return false;
	}

	//start looping playback
	err = primary_buffer->Play(0, 0, DSBPLAY_LOOPING);
	if (FAILED(err))
		return false;

	return true;
}

void win32_dsound_engine_t::cleanup()
{
	if (primary_buffer)
		primary_buffer->Release();

	if (directsound)
		directsound->Release();

	directsound = nullptr;
	primary_buffer = nullptr;
}
