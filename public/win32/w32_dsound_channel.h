#pragma once

struct IDirectSound;
struct IDirectSoundBuffer;

struct w32_dsound_length_t;

struct w32_dsound_channel_t
{
	bool play(const bool aLooped, const float aVolume, const float aPan, const float aFrequency, const void* aHandle);
	bool stop(const void* aHandle);

	bool is_playing() const;
	float volume() const;

	explicit w32_dsound_channel_t();
	~w32_dsound_channel_t();

	const void* handle;
	::IDirectSoundBuffer* buffer;
};

::IDirectSoundBuffer* win32_dsound_load_waveform(
	const char* filename,
	::IDirectSound& directsound, w32_dsound_length_t& l);
