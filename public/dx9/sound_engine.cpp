#include "stdafx.h"

#include "sound_engine.h"
#include "sound_util.h"

namespace sound
{
#ifdef _DEBUG
	engine_t::~engine_t()
	{
		assert(nullptr == this->primary_buffer);
		assert(nullptr == this->directsound);
	}
#endif

	bool engine_init(const ::HWND window, engine_t& e)
	{
		::HRESULT err;

		//create directsound object
		assert(nullptr == e.directsound);
		err = ::DirectSoundCreate(nullptr, &e.directsound, nullptr);
		if (FAILED(err))
			return false;

		//set cooperative level
		err = e.directsound->SetCooperativeLevel(window, DSSCL_PRIORITY);
		if (FAILED(err))
			return false;

		//set up DSBUFFERDESC structure.for a streaming secondary buffer
		::DSBUFFERDESC dsbd{};	//directsound buffer description
		dsbd.dwSize = sizeof(::DSBUFFERDESC);
		dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
		dsbd.dwBufferBytes = 0;		//must be 0 for primary buffer
		dsbd.lpwfxFormat = nullptr;	//must be nullptr for primary buffer

		//try to create buffer
		assert(nullptr == e.primary_buffer);
		err = e.directsound->CreateSoundBuffer(&dsbd, &e.primary_buffer, nullptr);
		if (FAILED(err))
			return false;

		//check buffer format
		::WAVEFORMATEX wav_format;
		err = e.primary_buffer->GetFormat(&wav_format, sizeof(wav_format), nullptr);
		if (FAILED(err))
			return false;

		//make sure is our desired format
		if (wav_format.wFormatTag != stereo_wav_format.formatTag ||
			wav_format.nChannels != stereo_wav_format.numChannels ||
			wav_format.nSamplesPerSec != stereo_wav_format.samplesPerSecond ||
			wav_format.nAvgBytesPerSec != stereo_wav_format.averageBytesPerSecond ||
			wav_format.nBlockAlign != stereo_wav_format.blockAlign ||
			wav_format.wBitsPerSample != stereo_wav_format.bitsPerSample ||
			wav_format.cbSize != stereo_wav_format.extraInfoSize)
		{
			err = e.primary_buffer->SetFormat((::LPCWAVEFORMATEX)&stereo_wav_format);
			if (FAILED(err))
				return false;
		}

		//start looping playback
		err = e.primary_buffer->Play(0, 0, DSBPLAY_LOOPING);
		if (FAILED(err))
			return false;

		return true;
	}

	void engine_cleanup(engine_t& e)
	{
		if (e.primary_buffer)
			e.primary_buffer->Release();

		if (e.directsound)
			e.directsound->Release();

		e = {};
	}
}
