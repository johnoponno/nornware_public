#include "stdafx.h"
#include "win32_dsound_container.h"

#include "win32_dsound.h"
#include "win32_dsound_vorbis_file.h"
#include "win32_dsound_engine.h"
#include "sound_stream.h"

//init (object should exist even if disabled, but its behaviour will differ, ie won't create sound objects)
bool win32_dsound_container_t::init()
{
	if (!NUM_SOUNDS)
		return false;

	sounds = new win32_dsound_t * [NUM_SOUNDS];
	if (!sounds)
		return false;

	for (uint32_t i = 0; i < NUM_SOUNDS; ++i)
		sounds[i] = nullptr;

	return true;
}

bool win32_dsound_container_t::add_sound(const win32_dsound_engine_t& engine, const char* aFileName, const uint32_t anId, const uint32_t aNumChannels) const
{
	//only add sound if DirectSound is active
	if (engine.directsound && anId < NUM_SOUNDS && aNumChannels > 0)
	{
		//clear any existing...
		if (sounds[anId])
		{
			delete sounds[anId];
			sounds[anId] = nullptr;
		}

		sounds[anId] = win32_dsound_t::create(aFileName, aNumChannels, *engine.directsound);
		return sounds[anId] != nullptr;
	}

	return true;
}

/*
bool container_play_looped(const bool anEnable, const uint32_t anId, const float aVolume, const float aPan, const float aFrequency, const void* aHandle, const container_t& c)
{
	return anId < c.NUM_SOUNDS && nullptr != c.sounds[anId] && c.sounds[anId]->play_looped(anEnable, aVolume, aPan, aFrequency, aHandle);
}

bool container_stop_handle(const uint32_t anId, const void* aHandle, const container_t& c)
{
	return anId < c.NUM_SOUNDS && c.sounds[anId] && c.sounds[anId]->stop_handle(aHandle);
}

bool container_stop_channel(const uint32_t anId, const uint32_t aChannel, const container_t& c)
{
	return anId < c.NUM_SOUNDS && c.sounds[anId] && c.sounds[anId]->stop_channel(aChannel);
}
*/

bool win32_dsound_container_t::play(const uint32_t anId, const float aVolume, const float aPan, const float aFrequency, const void* aHandle) const
{
	if (anId < NUM_SOUNDS &&
		sounds[anId] &&
		aVolume > 0.f &&
		sounds[anId]->play(false, aVolume, aPan, aFrequency, aHandle))
	{
		return true;
	}

	/*
	if(anId >= NUM_SOUNDS)
	{
	FS_LOG("id %d was out of range %u", anId, NUM_SOUNDS);
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

/*
bool container_playing_eh(const uint32_t anId, const container_t& c)
{
	return anId < c.NUM_SOUNDS && c.sounds[anId] && c.sounds[anId]->playing_eh();
}

bool container_playing_eh(const uint32_t anId, const uint32_t aChannel, const container_t& c)
{
	return anId < c.NUM_SOUNDS && c.sounds[anId] && c.sounds[anId]->playing_eh(aChannel);
}
*/

win32_dsound_container_t::win32_dsound_container_t(const uint32_t aNumSounds)
	:NUM_SOUNDS(aNumSounds)
	, sounds(nullptr)
{
}

#ifdef _DEBUG
win32_dsound_container_t::~win32_dsound_container_t()
{
	assert(nullptr == this->sounds);
}
#endif

void win32_dsound_container_t::cleanup()
{
	if (sounds)
	{
		clear();
		delete[] sounds;
		sounds = nullptr;
	}
}

void win32_dsound_container_t::clear() const
{
	for (uint32_t i = 0; i < NUM_SOUNDS; ++i)
	{
		if (sounds[i])
		{
			delete sounds[i];
			sounds[i] = nullptr;
		}
	}
}

/*
void container_stop_all(const container_t& c)
{
	for (uint32_t i = 0; i < c.NUM_SOUNDS; ++i)
	{
		if (c.sounds[i])
			c.sounds[i]->stop_handle(nullptr);
	}
}
*/

sound::stream_t* win32_dsound_stream_create(const win32_dsound_engine_t& engine, const char* aFile)
{
	sound::stream_t* stream = nullptr;

	if (engine.directsound)
	{
		win32_dsound_stream_source_i* source = win32_dsound_vorbis_file_t::create(aFile);
		if (source)
		{
			stream = sound::stream_create(engine, source);
			if (!stream)
				delete source;
		}
	}

	return stream;
}
