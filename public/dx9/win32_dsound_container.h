#pragma once

struct win32_dsound_t;

namespace sound
{
	struct stream_t;
	struct engine_t;
}

struct win32_dsound_container_t
{
	bool init();
	void cleanup();

	void clear() const;
	//bool playing_eh(const uint32_t anId, const uint32_t aChannel) const;
	//bool playing_eh(const uint32_t anId) const;
	bool add_sound(const sound::engine_t& engine, const char* aFileName, const uint32_t anId, const uint32_t aNumChannels) const;
	bool play(const uint32_t anId, const float aVolume, const float aPan, const float aFrequency, const void* aHandle) const;

	explicit win32_dsound_container_t(const uint32_t aNumSounds);
#ifdef _DEBUG
	~win32_dsound_container_t();
#endif

	const uint32_t NUM_SOUNDS;
	win32_dsound_t** sounds;

private:

	explicit win32_dsound_container_t(const win32_dsound_container_t& other) = delete;
};

//void container_stop_all(const container_t& c);
//bool container_stop_channel(const uint32_t anId, const uint32_t aChannel, const container_t& c);
//bool container_stop_handle(const uint32_t anId, const void* aHandle, const container_t& c);
//bool container_play_looped(const bool anEnable, const uint32_t anId, const float aVolume, const float aPan, const float aFrequency, const void* aHandle, const container_t& c);

//FIXME: move this?
sound::stream_t* win32_dsound_stream_create(const sound::engine_t& engine, const char* aFile);
