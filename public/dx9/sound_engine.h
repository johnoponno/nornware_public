#pragma once

struct IDirectSound;
struct IDirectSoundBuffer;

namespace sound
{
	struct engine_t
	{
#ifdef _DEBUG
		~engine_t();
#endif
		::IDirectSound* directsound;
		::IDirectSoundBuffer* primary_buffer;
	};

	bool engine_init(const ::HWND window, engine_t& e);
	void engine_cleanup(engine_t& e);
}
