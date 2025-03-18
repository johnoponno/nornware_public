#include "stdafx.h"

#include "../win32/w32_d3d9_app.h"
#include "../win32/w32_d3d9_state.h"
#include "../plax/plax.h"

//--------------------------------------------------------------------------------------
// specific game implementation - we want 8 bit / palettized pixels
//--------------------------------------------------------------------------------------
struct wmdl_app_t : public w32_d3d9_chunky_app_t
{
	//this is the main "tick" callback from the win32 / d3d9 harness
	bool w32_d3d9_chunky_app_tick(const w32_dsound_container_t& in_sounds) override
	{
		if (!plax_tick(_guts._micron, _game))
			return false;

#if 0
		{//display the number of dropped frames (60hz)
			char slask[16];
			::sprintf_s(slask, "%u", w32_d3d9_state.app->_w32_d3d9_app_frame_drops);
			micron_canvas_atascii_print(_guts._micron, slask, 255, 0, _guts._micron.canvas_width / 2, _guts._micron.canvas_height - 8);
		}//display the number of dropped frames (60hz)
#endif

		for (const uint32_t SP : _guts._micron.sound_plays)
			in_sounds.play(SP, 1.f, 0.f, 1.f, nullptr);
		_guts._micron.sound_plays.clear();

		_guts.handle_music_request(_w32_d3d9_app_frame_moves);

		return true;
	}

	//we pass our time-per-tick (1 / fps) and our framebuffer to be rendered
	explicit wmdl_app_t()
		:w32_d3d9_chunky_app_t(PLAX_SECONDS_PER_TICK, 0)
	{
	}

	//this is our actual game (platform agnostic) that only depends on micron (which is platform agnostic)
	plax_t _game;
};

//--------------------------------------------------------------------------------------
// local variables
//--------------------------------------------------------------------------------------
static wmdl_app_t __app;

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int32_t)
{
	//win32 / d3d9 init (title, windowed?, window width, window height)
#if 1
	if (!__app.init("Plax (c) 2025 nornware AB", true, ::GetSystemMetrics(SM_CXSCREEN) * 3 / 4, ::GetSystemMetrics(SM_CYSCREEN) * 3 / 4))
#else
	if (!__app.init("Plax (c) 2025 nornware AB", false, 0, 0))
#endif
		return -1;

	//application-specific init
	if (!plax_init(__app._guts._micron, __app._game))
		return -1;
	/*
	if (!__app.w32_d3d9_chunky_app_init_audio())
		return -1;
		*/

	//enter main loop
	w32_d3d9_main_loop(&__app);

	//application-specific shutdown?
	plax_shutdown(__app._game);

	//cleanup internal win32 / d3d9 / dsound stuff
	__app._guts.cleanup_audio();

	//return internal exit code
	return w32_d3d9_state.m_exit_code;
}
