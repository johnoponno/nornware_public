#include "stdafx.h"

#include "win32_dsound_channel.h"
#include "sound_file_wave.h"

static float __directx_to_linear_volume(const long aDirectX)
{
	if (aDirectX <= -10000)
		return 0.f;

	if (aDirectX >= 0)
		return 1.f;

	return ::powf(10.0f, (float)aDirectX / 2000.f);
}

/*
static float __directx_to_linear_pan(const long aDirectX)
{
	if (aDirectX <= -10000)
		return -1.f;

	if (aDirectX >= 10000)
		return 1.f;

	const float LINEAR(::powf((float)::abs(aDirectX) / 10000.f, 1.f / 3.f));
	if (aDirectX < 0)
		return -LINEAR;

	return LINEAR;

}
*/

static long __linear_to_directx_pan(const float aLinear)
{
	float p = aLinear;

	if (p < -1.f)
		p = -1.f;
	else if (p > 1.f)
		p = 1.f;

	//exponential
	p = ::powf(p, 3.f);

	return (long)(p * 10000.f);
}

static bool __buffer_is_playing(::IDirectSoundBuffer* buffer)
{
	::DWORD status;

	assert(buffer);
	buffer->GetStatus(&status);

	return (status != DSBSTATUS_BUFFERLOST) && (status & DSBSTATUS_PLAYING) || (status & DSBSTATUS_LOOPING);
}

static bool __buffer_play_looped(
	const float volume, const float pan, const float frequency,
	::IDirectSoundBuffer* buffer)
{
	if (__buffer_is_playing(buffer))
	{
		return
			buffer->SetVolume(win32_dsound_linear_to_directx_volume(volume)) == DS_OK &&
			buffer->SetPan(__linear_to_directx_pan(pan)) == DS_OK &&
			buffer->SetFrequency((uint32_t)(44100.f * frequency)) == DS_OK;
	}

	return
		buffer->SetCurrentPosition(0) == DS_OK &&
		buffer->SetVolume(win32_dsound_linear_to_directx_volume(volume)) == DS_OK &&
		buffer->SetPan(__linear_to_directx_pan(pan)) == DS_OK &&
		buffer->SetFrequency((uint32_t)(44100.f * frequency)) == DS_OK &&
		buffer->Play(0, 0, DSBPLAY_LOOPING) == DS_OK;
}

//only play if the volume is audible
//DO NOT STOP otherwise; this should avoid choking out playing sounds that ARE audible
static bool __buffer_play(
	const float volume, const float pan, const float frequency,
	::IDirectSoundBuffer* buffer)
{
	//do we want to check incoming calls?
	assert(volume > 0.f && volume <= 1.f);

	if (volume > 0.f)
	{
		return
			DS_OK == buffer->Stop() &&
			DS_OK == buffer->SetCurrentPosition(0) &&
			DS_OK == buffer->SetVolume(win32_dsound_linear_to_directx_volume(volume)) &&
			DS_OK == buffer->SetPan(__linear_to_directx_pan(pan)) &&
			DS_OK == buffer->SetFrequency((uint32_t)(44100.f * frequency)) &&
			DS_OK == buffer->Play(0, 0, 0);
	}

	return true;
}

//public
//public
//public
//public
::IDirectSoundBuffer* win32_dsound_load_waveform(
	const char* filename,
	::IDirectSound& directsound, win32_dsound_length_t& l)
{
	//load a wav file
	sound::file_wave_t wave;
	if (!file_wave_load(filename, wave))
		return nullptr;

	l.in_bytes = wave.size;
	l.in_seconds = (float)l.in_bytes / (float)wave.header.average_bytes_per_second;

	//set up DSBUFFERDESC structure for a secondary buffer
	::DSBUFFERDESC bd{};
	bd.dwSize = sizeof(bd);
	bd.dwFlags = DSBCAPS_STATIC | DSBCAPS_LOCSOFTWARE | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
	bd.dwBufferBytes = wave.size;
	bd.lpwfxFormat = (::LPWAVEFORMATEX)&wave.header;

	//try to create buffer.
	::IDirectSoundBuffer* new_buffer = nullptr;
	const ::HRESULT err = directsound.CreateSoundBuffer(&bd, &new_buffer, nullptr);
	if (DS_OK != err)
	{
		switch (err)
		{
		case DSERR_ALLOCATED:
			//The request failed because resources, such as a priority level, were already in use by another caller.  
			break;

		case DSERR_BADFORMAT:
			//The specified wave format is not supported.  
			break;

		case DSERR_INVALIDPARAM:
			//An invalid parameter was passed to the returning function.
			break;

		case DSERR_NOAGGREGATION:
			//The object does not support aggregation.  
			break;

		case DSERR_OUTOFMEMORY:
			//The DirectSound subsystem could not allocate sufficient memory to complete the caller's request.  
			break;

		case DSERR_UNINITIALIZED:
			//The IDirectSound::Initialize method has not been called or has not been called successfully before other methods were called.
			break;

		case DSERR_UNSUPPORTED:
			//The function called is not supported at this time.
			break;
		}

		return nullptr;
	}

	//copy data to buffer
	struct
	{
		void* data;
		::DWORD length;
	} data1{}, data2{};
	if (new_buffer->Lock(
		0,					//write offset
		wave.size,			//size of mem to lock
		&data1.data,		//pointer to first block of locked mem
		&data1.length,		//length of first block
		&data2.data,		//pointer to second block of locked mem
		&data2.length,		//length of second block
		0L) == DS_OK)		//flags
	{
		//copy chunk 1
		if (data1.length)
		{
			if (8 == wave.header.bitsPerSample)
				::memset(data1.data, 127, data1.length);
			else if (16 == wave.header.bitsPerSample)
				::memset(data1.data, 0, data1.length);

			::memcpy(data1.data, wave.data, data1.length);
		}

		//copy chunk 2
		if (data2.length)
		{
			if (8 == wave.header.bitsPerSample)
				::memset(data2.data, 127, data2.length);
			else if (16 == wave.header.bitsPerSample)
				::memset(data2.data, 0, data2.length);

			::memcpy(data2.data, wave.data + data1.length, data2.length);
		}

		//unlock
		if (DS_OK != new_buffer->Unlock(data1.data, data1.length, data2.data, data2.length))
			return nullptr;
	}
	else
	{
#ifdef _DEBUG
		assert(0 == new_buffer->Release());
#else
		new_buffer->Release();
#endif
		return nullptr;
	}

	return new_buffer;
}

bool win32_dsound_channel_t::is_playing() const
{
	return __buffer_is_playing(buffer);
}

float win32_dsound_channel_t::volume() const
{
	long v = 0;

	assert(buffer);
	buffer->GetVolume(&v);

	return __directx_to_linear_volume(v);
}

win32_dsound_channel_t::win32_dsound_channel_t()
{
	handle = nullptr;
	buffer = nullptr;
}

win32_dsound_channel_t::~win32_dsound_channel_t()
{
	if (buffer)
	{
		buffer->Release();
		buffer = nullptr;
	}
}

/*
float buffer_pan(const buffer_t& b)
{
	long p = 0;

	if (b.buffer)
		b.buffer->GetPan(&p);

	return __directx_to_linear_pan(p);
}

uint32_t buffer_position(const buffer_t& b)
{
	::DWORD position;
	if (SUCCEEDED(b.buffer->GetCurrentPosition(&position, nullptr)))
		return position;

	return 0;
}
*/

/*
*/

/*
bool buffer_play_looped(const bool anEnable, const float aVolume, const float aPan, const float aFrequency, const buffer_t& b)
{
	if (anEnable && aVolume > 0.f)
		return buffer_play_looped(aVolume, aPan, aFrequency, b);

	return buffer_stop(b);
}
*/

bool win32_dsound_channel_t::play(const bool in_looped, const float in_volume, const float in_pan, const float in_frequency, const void* in_handle)
{
	bool result;

	if (in_looped)
		result = __buffer_play_looped(in_volume, in_pan, in_frequency, buffer);
	else
		result = __buffer_play(in_volume, in_pan, in_frequency, buffer);

	if (result)
		handle = in_handle;

	return result;
}

bool win32_dsound_channel_t::stop(const void* in_handle)
{
	if ((in_handle && handle == in_handle) || !in_handle)
	{
		handle = nullptr;
		return DS_OK == buffer->Stop();
	}

	return false;
}
