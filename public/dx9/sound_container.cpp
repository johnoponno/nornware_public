#include "stdafx.h"
#include "sound_container.h"

#include "win32_dsound.h"
#include "sound_vorbis_file.h"
#include "sound_engine.h"
#include "sound_stream.h"

namespace sound
{
	//init (object should exist even if disabled, but its behaviour will differ, ie won't create sound objects)
	bool container_init(container_t& c)
	{
		if (!c.num_sounds)
			return false;

		c.sounds = new win32_dsound_t *[c.num_sounds];
		if (!c.sounds)
			return false;

		for (uint32_t i = 0; i < c.num_sounds; ++i)
			c.sounds[i] = nullptr;

		return true;
	}

	bool container_add_sound(const engine_t& engine, const char* aFileName, const uint32_t anId, const uint32_t aNumChannels, const container_t& c)
	{
		//only add sound if DirectSound is active
		if (engine.directsound && anId < c.num_sounds && aNumChannels > 0)
		{
			//clear any existing...
			if (c.sounds[anId])
			{
				delete c.sounds[anId];
				c.sounds[anId] = nullptr;
			}

			c.sounds[anId] = win32_dsound_t::create(aFileName, aNumChannels, *engine.directsound);
			return c.sounds[anId] != nullptr;
		}

		return true;
	}

	bool container_play_looped(const bool anEnable, const uint32_t anId, const float aVolume, const float aPan, const float aFrequency, const void* aHandle, const container_t& c)
	{
		return anId < c.num_sounds && nullptr != c.sounds[anId] && c.sounds[anId]->play_looped(anEnable, aVolume, aPan, aFrequency, aHandle);
	}

	bool container_stop_handle(const uint32_t anId, const void* aHandle, const container_t& c)
	{
		return anId < c.num_sounds && c.sounds[anId] && c.sounds[anId]->stop_handle(aHandle);
	}

	bool container_stop_channel(const uint32_t anId, const uint32_t aChannel, const container_t& c)
	{
		return anId < c.num_sounds && c.sounds[anId] && c.sounds[anId]->stop_channel(aChannel);
	}

	bool container_play(const uint32_t anId, const float aVolume, const float aPan, const float aFrequency, const void* aHandle, const container_t& c)
	{
		if (anId < c.num_sounds &&
			c.sounds[anId] &&
			aVolume > 0.f &&
			c.sounds[anId]->play(false, aVolume, aPan, aFrequency, aHandle))
		{
			return true;
		}

		/*
		if(anId >= num_sounds)
		{
		FS_LOG("id %d was out of range %u", anId, num_sounds);
		}
		else
		{
		FS_LOG("failed to play id %u, volume %f, pan %f, frequency %f, handle %d", anId, aVolume, aPan, aFrequency, aHandle);
		if(sounds[anId])
		{
		uint32_t i;
		const Sound& S(*sounds[anId]);

		FS_LOG("numChannels = %u", S.myNumChannels);
		for(i = 0; i < S.myNumChannels; ++i)
		{
		const Channel& C(S.myChannels[i]);
		FS_LOG("channel %u", i);
		FS_LOG("\tisPlaying = %u", C.is_playing() ? 1 : 0);
		FS_LOG("\tvolume = %f", C.volume());
		//FS_LOG("\tpan = %f", C.pan());
		//FS_LOG("\tfrequency = %f", C.frequency());
		}
		}
		else
		{
		FS_LOG("sound is not loaded");
		}
		}
		*/

		return false;
	}

	bool container_playing_eh(const uint32_t anId, const container_t& c)
	{
		return anId < c.num_sounds && c.sounds[anId] && c.sounds[anId]->playing_eh();
	}

	bool container_playing_eh(const uint32_t anId, const uint32_t aChannel, const container_t& c)
	{
		return anId < c.num_sounds && c.sounds[anId] && c.sounds[anId]->playing_eh(aChannel);
	}

	container_t::container_t(const uint32_t aNumSounds)
		:num_sounds(aNumSounds)
		, sounds(nullptr)
	{
	}

#ifdef _DEBUG
	container_t::~container_t()
	{
		assert(nullptr == this->sounds);
	}
#endif

	void container_cleanup(container_t& c)
	{
		if (c.sounds)
		{
			container_clear(c);
			delete[] c.sounds;
			c.sounds = nullptr;
		}
	}

	void container_clear(const container_t& c)
	{
		for (uint32_t i = 0; i < c.num_sounds; ++i)
		{
			if (c.sounds[i])
			{
				delete c.sounds[i];
				c.sounds[i] = nullptr;
			}
		}
	}

	stream_t* stream_create(const engine_t& engine, const char* aFile)
	{
		stream_t* stream = nullptr;

		if (engine.directsound)
		{
			stream_source_i* source = nullptr;

			source = vorbis_file_t::create(aFile);

			if (source)
			{
				stream = stream_create(engine, source);
				if (!stream)
					delete source;
			}
		}

		return stream;
	}

	void container_stop_all(const container_t& c)
	{
		for (uint32_t i = 0; i < c.num_sounds; ++i)
		{
			if (c.sounds[i])
				c.sounds[i]->stop_handle(nullptr);
		}
	}
}
