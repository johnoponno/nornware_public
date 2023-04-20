#pragma once

#include "win32_dsound_util.h"

struct IDirectSound;
struct IDirectSoundBuffer;

namespace sound
{
	struct channel_t;

	struct sound_t
	{
		enum
		{
			max_channels = 8,
		};

		explicit sound_t(const uint32_t aNumChannels);
		~sound_t();

		enum
		{
			stat_select_lowest_volume,
			stat_select_failed,
			num_stats,
		};

		const uint32_t num_channels;
		channel_t* channels;
		uint32_t stats[num_stats];
		win32_dsound_length_t length;
	};

	sound_t* sound_create(const char* aFileName, const uint32_t aNumChannels, ::IDirectSound& aDirectSound);

	bool sound_play(const bool aLooped, const float aVolume, const float aPan, const float aFrequency, const void* aHandle, sound_t& s);
	bool sound_play_looped(const bool anEnable, const float aVolume, const float aPan, const float aFrequency, const void* aHandle, sound_t& s);
	bool sound_stop_channel(const uint32_t aChannel, sound_t& s);
	bool sound_stop_handle(const void* aHandle, sound_t& s);

	bool sound_playing_eh(const uint32_t aChannel, const sound_t& s);
	bool sound_playing_eh(const sound_t& s);
}
