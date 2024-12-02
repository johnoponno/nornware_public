#pragma once

struct IDirectSoundBuffer;

struct win32_dsound_stream_source_i;
struct win32_dsound_engine_t;

struct win32_dsound_stream_t
{
	static win32_dsound_stream_t* create(const win32_dsound_engine_t& anEngine, win32_dsound_stream_source_i* aSource);

	bool play(const bool aLoopFlag, const float aStartPosition, const float aMaxVolume);
	bool update(const float aCurrentTime, const float aMaxVolume);

	~win32_dsound_stream_t();

	win32_dsound_stream_source_i* source;
	::IDirectSoundBuffer* buffer;
	float fade_start;
	float fade_duration;
	float fade_delta;
	uint32_t last_play_position;
	uint32_t buffer_size;
	uint32_t total_position;
	uint32_t end_of_sample_position;
	uint32_t fading_eh : 1;
	uint32_t playing_eh : 1;
	uint32_t looping_eh : 1;
	uint32_t stop_pending_eh : 1;

private:

	explicit win32_dsound_stream_t(win32_dsound_stream_source_i* aSource);
};

//bool stream_fade_in_play(const bool aLoopFlag, const float aCurrentTime, const float aDuration, const float aStartPosition, stream_t& s);
//bool stream_fade_out_stop(const float aCurrentTime, const float aDuration, stream_t& s);
//void stream_stop_playback(stream_t& s);
//float stream_position_seconds(const stream_t& s);
//void stream_position_bytes(uint32_t& aPosition, uint32_t& aTotal, const stream_t& s);
