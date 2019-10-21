#include "stdafx.h"
#include "sound.h"

#include "sound_channel.h"

namespace sound
{
	static channel_t* __channel(const void* aHandle, sound_t& s)
	{
		//try to match handle
		if (aHandle)
		{
			for (uint32_t i = 0; i < s.num_channels; ++i)
			{
				if (s.channels[i].handle == aHandle)
					return s.channels + i;
			}
		}

		//try for one that isn't playing
		for (uint32_t i = 0; i < s.num_channels; ++i)
		{
			if (!buffer_is_playing(s.channels[i].buffer))
				return s.channels + i;
		}

		//find the lowest volume
		uint32_t lc = s.num_channels;
		float lv = 2.f;	//1.f is max
		for (uint32_t i = 0; i < s.num_channels; ++i)
		{
			const float V = buffer_volume(s.channels[i].buffer);
			if (V < lv)
			{
				lv = V;
				lc = i;
			}
		}

		//use the lowest volume
		if (lc < s.num_channels)
		{
			++s.stats[sound_t::stat_select_lowest_volume];
			return s.channels + lc;
		}

		//failed to find a suitable channel
		++s.stats[sound_t::stat_select_failed];
		return nullptr;
	}

	static bool __create_channels(::IDirectSound& aDirectSound, ::IDirectSoundBuffer* aBuffer, sound_t& s)
	{
		//create sound channels
		s.channels = new channel_t[s.num_channels];
		if (!s.channels)
			return false;

		//store loaded buffer in first channel
		s.channels[0].buffer.buffer = aBuffer;

		//duplicate buffers
		for (uint32_t i = 1; i < s.num_channels; ++i)
		{
			if (DS_OK != aDirectSound.DuplicateSoundBuffer(aBuffer, &s.channels[i].buffer.buffer))
				return false;
		}

		return true;
	}

	static bool __init(const char* aFileName, ::IDirectSound& aDirectSound, sound_t& s)
	{
		//load main buffer
		::IDirectSoundBuffer* main_buffer = load_waveform(aFileName, aDirectSound, s.length);
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
	sound_t* sound_create(const char* aFileName, const uint32_t aNumChannels, ::IDirectSound& aDirectSound)
	{
		sound_t* sound = nullptr;

		if (aNumChannels >= 1 && aNumChannels <= sound_t::max_channels)
		{
			sound = new sound_t(aNumChannels);

			if (sound && !__init(aFileName, aDirectSound, *sound))
			{
				delete sound;
				sound = nullptr;
			}
		}

		return sound;
	}

	bool sound_stop_handle(const void* aHandle, sound_t& s)
	{
		bool result = true;

		for (uint32_t i = 0; i < s.num_channels; ++i)
			result &= channel_stop(aHandle, s.channels[i]);

		return result;
	}

	bool sound_stop_channel(const uint32_t aChannel, sound_t& s)
	{
		return aChannel < s.num_channels && channel_stop(nullptr, s.channels[aChannel]);
	}

	bool sound_play(const bool aLooped, const float aVolume, const float aPan, const float aFrequency, const void* aHandle, sound_t& s)
	{
		channel_t* c = nullptr;

		c = __channel(aHandle, s);
		if (c)
			return channel_play(aLooped, aVolume, aPan, aFrequency, aHandle, *c);

		return false;
	}

	bool sound_play_looped(const bool anEnable, const float aVolume, const float aPan, const float aFrequency, const void* aHandle, sound_t& s)
	{
		if (anEnable && aVolume > 0.f)
			return sound_play(true, aVolume, aPan, aFrequency, aHandle, s);

		return sound_stop_handle(aHandle, s);
	}

	bool sound_playing_eh(const sound_t& s)
	{
		for (uint32_t i = 0; i < s.num_channels; ++i)
		{
			if (buffer_is_playing(s.channels[i].buffer))
				return true;
		}

		return false;
	}

	bool sound_playing_eh(const uint32_t aChannel, const sound_t& s)
	{
		return aChannel < s.num_channels && buffer_is_playing(s.channels[aChannel].buffer);
	}

	sound_t::sound_t(const uint32_t aNumChannels)
		:num_channels(aNumChannels)
		, channels(nullptr)
	{
		length = {};

		for (uint32_t i = 0; i < num_stats; ++i)
			stats[i] = 0;
	}

	sound_t::~sound_t()
	{
		delete[] channels;
		channels = nullptr;
	}
}
