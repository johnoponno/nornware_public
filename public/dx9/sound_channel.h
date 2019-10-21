#pragma once

struct IDirectSound;
struct IDirectSoundBuffer;

namespace sound
{
	struct length_t;

	struct buffer_t
	{
		explicit buffer_t();
		~buffer_t();

		::IDirectSoundBuffer* buffer;
	};

	struct channel_t
	{
		explicit channel_t();

		const void* handle;
		buffer_t buffer;
	};

	::IDirectSoundBuffer* load_waveform(const char* filename, ::IDirectSound& directsound, length_t& l);
	bool buffer_init(const char* aFileName, ::IDirectSound& aDirectSound, buffer_t& b);
	bool buffer_init(const char* filename, ::IDirectSound& directsound, buffer_t& b, length_t& l);
	void buffer_cleanup(buffer_t& b);
	uint32_t buffer_position(const buffer_t& b);
	float buffer_pan(const buffer_t& b);
	float buffer_volume(const buffer_t& b);
	bool buffer_is_playing(const buffer_t& b);
	bool buffer_stop(const buffer_t& b);
	bool buffer_play(const float aVolume, const float aPan, const float aFrequency, const buffer_t& b);
	bool buffer_play_looped(const float aVolume, const float aPan, const float aFrequency, const buffer_t& b);
	bool buffer_play_looped(const bool anEnable, const float aVolume, const float aPan, const float aFrequency, const buffer_t& b);

	bool channel_stop(const void* aHandle, channel_t& c);
	bool channel_play(const bool aLooped, const float aVolume, const float aPan, const float aFrequency, const void* aHandle, channel_t& c);
}
