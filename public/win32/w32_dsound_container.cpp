#include "stdafx.h"
#include "w32_dsound_container.h"

#include "w32_dsound.h"
#include "w32_dsound_vorbis_file.h"
#include "w32_dsound_engine.h"
#include "w32_dsound_stream.h"

bool w32_dsound_container_t::init()
{
	if (!NUM_SOUNDS)
		return false;

	_sounds = new w32_dsound_t * [NUM_SOUNDS];
	if (!_sounds)
		return false;

	for (
		uint32_t i = 0;
		i < NUM_SOUNDS;
		++i
		)
	{
		_sounds[i] = nullptr;
	}

	return true;
}

bool w32_dsound_container_t::add_sound(const w32_dsound_engine_t& engine, const char* aFileName, const uint32_t anId, const uint32_t aNumChannels) const
{
	//only add sound if DirectSound is active
	if (engine.directsound && anId < NUM_SOUNDS && aNumChannels > 0)
	{
		//clear any existing...
		if (_sounds[anId])
		{
			delete _sounds[anId];
			_sounds[anId] = nullptr;
		}

		_sounds[anId] = w32_dsound_t::create(aFileName, aNumChannels, *engine.directsound);
		return _sounds[anId] != nullptr;
	}

	return true;
}

bool w32_dsound_container_t::play(const uint32_t anId, const float aVolume, const float aPan, const float aFrequency, const void* aHandle) const
{
	if (anId < NUM_SOUNDS &&
		_sounds[anId] &&
		aVolume > 0.f &&
		_sounds[anId]->play(false, aVolume, aPan, aFrequency, aHandle))
	{
		return true;
	}

	return false;
}

bool w32_dsound_container_t::play_looped(const bool anEnable, const uint32_t anId, const float aVolume, const float aPan, const float aFrequency, const void* aHandle) const
{
	assert(_sounds);
	return
		anId < NUM_SOUNDS &&
		nullptr != _sounds[anId] &&
		_sounds[anId]->play_looped(anEnable, aVolume, aPan, aFrequency, aHandle);
}


w32_dsound_container_t::w32_dsound_container_t(const uint32_t aNumSounds)
	:NUM_SOUNDS(aNumSounds)
{
	_sounds = nullptr;
}

#ifdef _DEBUG
w32_dsound_container_t::~w32_dsound_container_t()
{
	assert(nullptr == _sounds);
}
#endif

void w32_dsound_container_t::cleanup()
{
	if (_sounds)
	{
		clear();
		delete[] _sounds;
		_sounds = nullptr;
	}
}

void w32_dsound_container_t::clear() const
{
	for (
		uint32_t i = 0;
		i < NUM_SOUNDS;
		++i
		)
	{
		if (_sounds[i])
		{
			delete _sounds[i];
			_sounds[i] = nullptr;
		}
	}
}

w32_dsound_stream_t* w32_dsound_stream_create(const w32_dsound_engine_t& engine, const char* aFile)
{
	w32_dsound_stream_t* stream = nullptr;

	if (engine.directsound)
	{
		w32_dsound_stream_source_i* source = w32_dsound_vorbis_file_t::create(aFile);
		if (source)
		{
			stream = w32_dsound_stream_t::create(engine, source);
			if (!stream)
				delete source;
		}
	}

	return stream;
}
