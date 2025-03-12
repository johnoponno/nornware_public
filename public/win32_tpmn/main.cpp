#include "stdafx.h"

#include "../win32/w32_d3d9_app.h"
#include "../win32/w32_d3d9_state.h"
#include "../tpmn/tpmn_game.h"

//--------------------------------------------------------------------------------------
// specific game implementation - we want 16 bit pixels
//--------------------------------------------------------------------------------------
struct tpmn_app_t : public w32_d3d9_softdraw_app_t
{
	//this is the main "tick" callback from the win32 / d3d9 harness
	bool w32_d3d9_softdraw_app_tick(const micron_t& in_micron, const w32_dsound_container_t& in_sounds) override
	{
		_sound_plays.clear();
		const char* music_request = nullptr;

		if (!tpmn_game_tick(in_micron, _game, _sound_plays, music_request))
			return false;

		for (const uint32_t SP : _sound_plays)
			in_sounds.play(SP, 1.f, 0.f, 1.f, nullptr);
		w32_d3d9_softdraw_app_handle_music_request(music_request);

		return true;
	}

	//we pass our time-per-tick (1 / fps) and our framebuffer to be rendered
	explicit tpmn_app_t()
		:w32_d3d9_softdraw_app_t(TPMN_SECONDS_PER_TICK, _game.controller.canvas, TPMN_NUM_SOUNDS)
	{
	}

	//this is our actual game (platform agnostic) that only depends on minyin (which is platform agnostic)
	tpmn_game_t _game;

	//this is kept around so as to not thrash the heap
	std::vector<uint32_t> _sound_plays;
};

//--------------------------------------------------------------------------------------
// local variables
//--------------------------------------------------------------------------------------
static tpmn_app_t __app;

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int32_t)
{
	//win32 / d3d9 init (title, windowed?, window width, window height)
#if 1
	if (!__app.init("Twisted Pair Man Nightmare (c) 2012-2024 nornware AB", true, ::GetSystemMetrics(SM_CXSCREEN) * 3 / 4, ::GetSystemMetrics(SM_CYSCREEN) * 3 / 4))
#else
	if (!__app.init("Twisted Pair Man Nightmare (c) 2012-2024 nornware AB", false, 0, 0))
#endif
		return -1;

	//application-specific init
	{
		std::vector<micron_sound_request_t> sound_requests;
		if (!tpmn_game_init(__app._game, sound_requests))
			return -1;
		if (!__app.w32_d3d9_softdraw_app_init_audio(sound_requests))
			return -1;
	}

	//enter main loop
	w32_d3d9_main_loop(&__app);

	//application-specific shutdown?
	//tpmn_game_shutdown(__app._game);

	//cleanup internal win32 / d3d9 / dsound stuff
	__app.w32_d3d9_softdraw_app_cleanup_audio();

	//return internal exit code
	return w32_d3d9_state.m_exit_code;
}
