#include "stdafx.h"
#include "w32_dsound.h"

#include "w32_dsound_channel.h"

static w32_dsound_channel_t* __channel(
	const void* aHandle,
	w32_dsound_t& s)
{
	//try to match handle
	if (aHandle)
	{
		for (uint32_t i = 0; i < s.NUM_CHANNELS; ++i)
		{
			if (s.channels[i].handle == aHandle)
				return s.channels + i;
		}
	}

	//try for one that isn't playing
	for (uint32_t i = 0; i < s.NUM_CHANNELS; ++i)
	{
		if (!s.channels[i].is_playing())
			return s.channels + i;
	}

	//find the lowest volume
	uint32_t lc = s.NUM_CHANNELS;
	float lv = 2.f;	//1.f is max
	for (uint32_t i = 0; i < s.NUM_CHANNELS; ++i)
	{
		const float V = s.channels[i].volume();
		if (V < lv)
		{
			lv = V;
			lc = i;
		}
	}

	//use the lowest volume
	if (lc < s.NUM_CHANNELS)
	{
		++s.stats[WIN32_DSOUND_STAT_SELECT_LOWEST_VOLUME];
		return s.channels + lc;
	}

	//failed to find a suitable channel
	++s.stats[WIN32_DSOUND_STAT_SELECT_FAILED];
	return nullptr;
}

static bool __create_channels(::IDirectSound& aDirectSound, ::IDirectSoundBuffer* aBuffer, w32_dsound_t& s)
{
	//create sound channels
	s.channels = new w32_dsound_channel_t[s.NUM_CHANNELS];
	if (!s.channels)
		return false;

	//store loaded buffer in first channel
	s.channels[0].buffer = aBuffer;

	//duplicate buffers
	for (uint32_t i = 1; i < s.NUM_CHANNELS; ++i)
	{
		if (DS_OK != aDirectSound.DuplicateSoundBuffer(aBuffer, &s.channels[i].buffer))
			return false;
	}

	return true;
}

static bool __init(
	const char* aFileName, ::IDirectSound& aDirectSound,
	w32_dsound_t& s)
{
	//load main buffer
	::IDirectSoundBuffer* main_buffer = w32_dsound_load_waveform(aFileName, aDirectSound, s.length);
	if (main_buffer)
	{
		if (__create_channels(aDirectSound, main_buffer, s))
			return true;

		//cleanup
#ifdef _DEBUG
		assert(0 == main_buffer->Release());
#else
		main_buffer->Release();
#endif
	}

	return false;
	}

//public
//public
//public
//public
w32_dsound_t* w32_dsound_t::create(const char* aFileName, const uint32_t aNumChannels, ::IDirectSound& aDirectSound)
{
	w32_dsound_t* sound = nullptr;

	if (aNumChannels > 0)
	{
		sound = new w32_dsound_t(aNumChannels);

		if (sound && !__init(aFileName, aDirectSound, *sound))
		{
			delete sound;
			sound = nullptr;
		}
	}

	return sound;
}

bool w32_dsound_t::stop_handle(const void* aHandle)
{
	bool result = true;

	for (uint32_t i = 0; i < NUM_CHANNELS; ++i)
		result &= channels[i].stop(aHandle);

	return result;
}

bool w32_dsound_t::stop_channel(const uint32_t aChannel)
{
	return aChannel < NUM_CHANNELS && channels[aChannel].stop(nullptr);
}

bool w32_dsound_t::play(const bool aLooped, const float aVolume, const float aPan, const float aFrequency, const void* aHandle)
{
	w32_dsound_channel_t* c = nullptr;

	c = __channel(aHandle, *this);
	if (c)
		return c->play(aLooped, aVolume, aPan, aFrequency, aHandle);

	return false;
}

bool w32_dsound_t::play_looped(const bool anEnable, const float aVolume, const float aPan, const float aFrequency, const void* aHandle)
{
	if (anEnable && aVolume > 0.f)
		return play(true, aVolume, aPan, aFrequency, aHandle);

	return stop_handle(aHandle);
}

bool w32_dsound_t::playing_eh() const
{
	for (uint32_t i = 0; i < NUM_CHANNELS; ++i)
	{
		if (channels[i].is_playing())
			return true;
	}

	return false;
}

bool w32_dsound_t::playing_eh(const uint32_t aChannel) const
{
	return aChannel < NUM_CHANNELS && channels[aChannel].is_playing();
}

w32_dsound_t::w32_dsound_t(const uint32_t aNumChannels)
	:NUM_CHANNELS(aNumChannels)
	, channels(nullptr)
{
	length = {};

	for (uint32_t i = 0; i < _countof(stats); ++i)
		stats[i] = 0;
}

w32_dsound_t::~w32_dsound_t()
{
	delete[] channels;
	channels = nullptr;
}
