#pragma once

struct IDirectSound;
struct IDirectSoundBuffer;

struct win32_dsound_length_t;

struct win32_dsound_buffer_t
{
	explicit win32_dsound_buffer_t();
	~win32_dsound_buffer_t();

	::IDirectSoundBuffer* buffer;
};

struct win32_dsound_channel_t
{
	bool play(const bool aLooped, const float aVolume, const float aPan, const float aFrequency, const void* aHandle);
	bool stop(const void* aHandle);

	bool is_playing() const;
	float volume() const;

	explicit win32_dsound_channel_t();

	const void* handle;
	win32_dsound_buffer_t buffer;
};

::IDirectSoundBuffer* win32_dsound_load_waveform(
	const char* filename,
	::IDirectSound& directsound, win32_dsound_length_t& l);
