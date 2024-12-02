#pragma once

struct IDirectSoundBuffer;

struct w32_dsound_stream_source_i;
struct w32_dsound_engine_t;

struct w32_dsound_stream_t
{
	static w32_dsound_stream_t* create(const w32_dsound_engine_t& anEngine, w32_dsound_stream_source_i* aSource);

	bool play(const bool aLoopFlag, const float aStartPosition, const float aMaxVolume);
	bool update(const float aCurrentTime, const float aMaxVolume);

	~w32_dsound_stream_t();

	w32_dsound_stream_source_i* source;
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

	explicit w32_dsound_stream_t(w32_dsound_stream_source_i* aSource);
};
