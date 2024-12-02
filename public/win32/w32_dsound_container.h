#pragma once

struct w32_dsound_t;
struct w32_dsound_engine_t;
struct w32_dsound_stream_t;

struct w32_dsound_container_t
{
	bool init();
	void cleanup();

	void clear() const;
	bool add_sound(const w32_dsound_engine_t& engine, const char* aFileName, const uint32_t anId, const uint32_t aNumChannels) const;
	bool play(const uint32_t anId, const float aVolume, const float aPan, const float aFrequency, const void* aHandle) const;

	explicit w32_dsound_container_t(const uint32_t aNumSounds);
#ifdef _DEBUG
	~w32_dsound_container_t();
#endif

	const uint32_t NUM_SOUNDS;
	w32_dsound_t** sounds;

private:

	explicit w32_dsound_container_t(const w32_dsound_container_t& other) = delete;
};

//FIXME: move this?
w32_dsound_stream_t* win32_dsound_stream_create(const w32_dsound_engine_t& engine, const char* aFile);
