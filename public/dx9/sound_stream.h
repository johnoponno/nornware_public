#pragma once

struct IDirectSoundBuffer;

struct win32_dsound_stream_source_i;

namespace sound
{
	struct engine_t;

	struct stream_t
	{
		explicit stream_t(win32_dsound_stream_source_i* aSource);
		~stream_t();

		win32_dsound_stream_source_i* source;
		::IDirectSoundBuffer* buffer;
		float fade_start;
		float fade_duration;
		float fade_delta;
		uint32_t last_play_position;
		uint32_t buffer_size;
		uint32_t total_position;
		uint32_t end_of_sample_position;
		unsigned fading_eh : 1;
		unsigned playing_eh : 1;
		unsigned looping_eh : 1;
		unsigned stop_pending_eh : 1;
	};

	stream_t* stream_create(const engine_t& anEngine, win32_dsound_stream_source_i* aSource);
	bool stream_play(const bool aLoopFlag, const float aStartPosition, const float aMaxVolume, stream_t& s);
	bool stream_fade_in_play(const bool aLoopFlag, const float aCurrentTime, const float aDuration, const float aStartPosition, stream_t& s);
	bool stream_fade_out_stop(const float aCurrentTime, const float aDuration, stream_t& s);
	void stream_stop_playback(stream_t& s);
	bool stream_update(const float aCurrentTime, const float aMaxVolume, stream_t& s);
	float stream_position_seconds(const stream_t& s);
	void stream_position_bytes(uint32_t& aPosition, uint32_t& aTotal, const stream_t& s);
}
