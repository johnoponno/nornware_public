#pragma once

#include "win32_dsound_util.h"

struct IDirectSound;
struct IDirectSoundBuffer;

struct win32_dsound_channel_t;

enum
{
	WIN32_DSOUND_STAT_SELECT_LOWEST_VOLUME,
	WIN32_DSOUND_STAT_SELECT_FAILED,
	WIN32_DSOUND_NUM_STATS,
};

struct win32_dsound_t
{
	static win32_dsound_t* create(const char* aFileName, const uint32_t aNumChannels, ::IDirectSound& aDirectSound);

	bool play(const bool aLooped, const float aVolume, const float aPan, const float aFrequency, const void* aHandle);
	bool play_looped(const bool anEnable, const float aVolume, const float aPan, const float aFrequency, const void* aHandle);
	bool stop_channel(const uint32_t aChannel);
	bool stop_handle(const void* aHandle);

	bool playing_eh(const uint32_t aChannel) const;
	bool playing_eh() const;

	~win32_dsound_t();

	const uint32_t NUM_CHANNELS;
	win32_dsound_channel_t* channels;
	uint32_t stats[WIN32_DSOUND_NUM_STATS];
	win32_dsound_length_t length;

private:

	explicit win32_dsound_t(const uint32_t in_num_channels);
};
