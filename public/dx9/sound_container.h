#pragma once

namespace sound
{
	struct sound_t;
	struct stream_t;
	struct engine_t;

	struct container_t
	{
		explicit container_t(const uint32_t aNumSounds);
#ifdef _DEBUG
		~container_t();
#endif

		const uint32_t num_sounds;
		sound_t** sounds;

	private:

		explicit container_t(const container_t& aContainer);
	};

	bool container_init(container_t& c);
	void container_cleanup(container_t& c);
	void container_clear(const container_t& c);

	//FIXME: move this!
	stream_t* stream_create(const engine_t& engine, const char* aFile);

	bool container_playing_eh(const uint32_t anId, const uint32_t aChannel, const container_t& c);
	bool container_playing_eh(const uint32_t anId, const container_t& c);
	void container_stop_all(const container_t& c);
	bool container_stop_channel(const uint32_t anId, const uint32_t aChannel, const container_t& c);
	bool container_stop_handle(const uint32_t anId, const void* aHandle, const container_t& c);
	bool container_play_looped(const bool anEnable, const uint32_t anId, const float aVolume, const float aPan, const float aFrequency, const void* aHandle, const container_t& c);
	bool container_play(const uint32_t anId, const float aVolume, const float aPan, const float aFrequency, const void* aHandle, const container_t& c);
	bool container_add_sound(const engine_t& engine, const char* aFileName, const uint32_t anId, const uint32_t aNumChannels, const container_t& c);
}
