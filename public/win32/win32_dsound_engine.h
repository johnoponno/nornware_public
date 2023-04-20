#pragma once

struct IDirectSound;
struct IDirectSoundBuffer;

struct win32_dsound_engine_t
{
	bool init(const ::HWND window);
	void cleanup();

#ifdef _DEBUG
	~win32_dsound_engine_t();
#endif
	::IDirectSound* directsound;
	::IDirectSoundBuffer* primary_buffer;
};
