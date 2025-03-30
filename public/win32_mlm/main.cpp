#include "stdafx.h"

#include "../win32/w32_d3d9_app.h"
#include "../win32/w32_d3d9_state.h"
#include "../mlm/game.h"
#include "../mlm/m_immutable.h"
#include "../mlm/m_work.h"
#include "../mlm/vc_assets.h"

//--------------------------------------------------------------------------------------
// specific game implementation - we want 8 bit / palettized pixels
//--------------------------------------------------------------------------------------
struct wmdl_app_t : public w32_d3d9_chunky_app_t
{
	//this is the main "tick" callback from the win32 / d3d9 harness
	bool w32_d3d9_chunky_app_tick(const w32_dsound_container_t& in_sounds) override
	{
		if (!_game.tick(_guts._micron))
			return false;

#if 0
		{//display the number of dropped frames (60hz)
			char slask[16];
			::sprintf_s(slask, "%u", w32_d3d9_state.app->_w32_d3d9_app_frame_drops);
			micron_canvas_atascii_print(_micron, slask, 255, 0, _micron.canvas_width / 2, _micron.canvas_height - 8);
		}//display the number of dropped frames (60hz)
#endif

		//hero sounds that aren't handle by the micron interface.. :(
		{
			in_sounds.play_looped(

				m_game_active(_game._mu) &&
				m_character_alive(_game._mu) &&
				_game._mu.hero_air &&
				(m_character_is_solid_left_more(_game._im, _game._mu) || m_character_is_solid_right(_game._im, _game._mu)),

				mlm::VC_SND_SLIDE,
				::fabsf(_game._mu.hero_speed.y) / mlm::M_CHAR_MAX_AIR_SPEED_Y,
				0.f,
				1.f,
				nullptr
			);

			/*
			if (_game._dev._tick == _game._mu.hero_melee_tick)
				in_sounds.play(mlm::VC_SND_MELEE, 1.f, 0.f, 1.f, nullptr);
				*/
		}

		for (const uint32_t SP : _guts._micron.sound_plays)
			in_sounds.play(SP, 1.f, 0.f, 1.f, nullptr);
		_guts._micron.sound_plays.clear();

		_guts.handle_music_request(_w32_d3d9_app_frame_moves);

		return true;
	}

	//we pass our time-per-tick (1 / fps) and our framebuffer to be rendered
	explicit wmdl_app_t()
		:w32_d3d9_chunky_app_t(mlm::M_SECONDS_PER_TICK, mlm::VC_NUM_SOUNDS)
	{
	}

	//this is our actual game (platform agnostic) that only depends on micron (which is platform agnostic)
	mlm::game_t _game;
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
	if (!__app.init("MicroLoMania (c) 2025", true, ::GetSystemMetrics(SM_CXSCREEN) * 3 / 4, ::GetSystemMetrics(SM_CYSCREEN) * 3 / 4))
#else
	if (!__app.init("MicroLoMania (c) 2025", false, 0, 0))
#endif
		return -1;

	//application-specific init
	if (!__app._game.init(__app._guts._micron))
		return -1;
	if (!__app._guts.init_audio())//this assumes that micron is loaded with sound load requests...
		return -1;

	//enter main loop
	w32_d3d9_main_loop(&__app);

	//application-specific shutdown?
	__app._game.shutdown();

	//cleanup internal win32 / d3d9 / dsound stuff
	__app._guts.cleanup_audio();

	//return internal exit code
	return w32_d3d9_state.m_exit_code;
}

namespace mlm
{
	const char* w32_vk_name(const int32_t in_vk)
	{
		switch (in_vk)
		{
		default:return"!!UNKNOWN!!";
		case VK_LBUTTON:return"LMB";
		case VK_RBUTTON:return"RMB";
		case VK_CANCEL:return"CANCEL";
		case VK_MBUTTON:return"MMB";
		case VK_XBUTTON1:return"XBUTTON1";
		case VK_XBUTTON2:return"XBUTTON2";
		case VK_BACK:return"BACKSPACE";
		case VK_TAB:return"TAB";
		case VK_CLEAR:return"CLEAR";
		case VK_RETURN:return"ENTER";
		case VK_SHIFT:return"SHIFT";
		case VK_CONTROL:return"CONTROL";
		case VK_MENU:return"ALT";
		case VK_PAUSE:return"PAUSE";
		case VK_CAPITAL:return"CAPS";
		case VK_KANA:return"KANA";
		case VK_JUNJA:return"JUNJA";
		case VK_FINAL:return"FINAL";
		case VK_HANJA:return"HANJA";
		case VK_ESCAPE:return"ESCAPE";
		case VK_CONVERT:return"CONVERT";
		case VK_NONCONVERT:return"NONCONVERT";
		case VK_ACCEPT:return"ACCEPT";
		case VK_MODECHANGE:return"MODECHANGE";
		case VK_SPACE:return"SPACE";
		case VK_PRIOR:return"PGUP";
		case VK_NEXT:return"PGDN";
		case VK_END:return"END";
		case VK_HOME:return"HOME";
		case VK_LEFT:return"LEFT";
		case VK_UP:return"UP";
		case VK_RIGHT:return"RIGHT";
		case VK_DOWN:return"DOWN";
		case VK_SELECT:return"SELECT";
		case VK_PRINT:return"PRINT";
		case VK_EXECUTE:return"EXECUTE";
		case VK_SNAPSHOT:return"PRNTSCRN";
		case VK_INSERT:return"INSERT";
		case VK_DELETE:return"DELETE";
		case VK_HELP:return"HELP";
		case 0x30:return"0";
		case 0x31:return"1";
		case 0x32:return"2";
		case 0x33:return"3";
		case 0x34:return"4";
		case 0x35:return"5";
		case 0x36:return"6";
		case 0x37:return"7";
		case 0x38:return"8";
		case 0x39:return"9";
		case 0x41:return"A";
		case 0x42:return"B";
		case 0x43:return"C";
		case 0x44:return"D";
		case 0x45:return"E";
		case 0x46:return"F";
		case 0x47:return"G";
		case 0x48:return"H";
		case 0x49:return"I";
		case 0x4A:return"J";
		case 0x4B:return"K";
		case 0x4C:return"L";
		case 0x4D:return"M";
		case 0x4E:return"N";
		case 0x4F:return"O";
		case 0x50:return"P";
		case 0x51:return"Q";
		case 0x52:return"R";
		case 0x53:return"S";
		case 0x54:return"T";
		case 0x55:return"U";
		case 0x56:return"V";
		case 0x57:return"W";
		case 0x58:return"X";
		case 0x59:return"Y";
		case 0x5A:return"Z";
		case VK_LWIN:return"LWIN";
		case VK_RWIN:return"RWIN";
		case VK_APPS:return"APPS";
		case VK_SLEEP:return"SLEEP";
		case VK_NUMPAD0:return"NUMPAD0";
		case VK_NUMPAD1:return"NUMPAD1";
		case VK_NUMPAD2:return"NUMPAD2";
		case VK_NUMPAD3:return"NUMPAD3";
		case VK_NUMPAD4:return"NUMPAD4";
		case VK_NUMPAD5:return"NUMPAD5";
		case VK_NUMPAD6:return"NUMPAD6";
		case VK_NUMPAD7:return"NUMPAD7";
		case VK_NUMPAD8:return"NUMPAD8";
		case VK_NUMPAD9:return"NUMPAD9";
		case VK_MULTIPLY:return"MULTIPLY";
		case VK_ADD:return"ADD";
		case VK_SEPARATOR:return"SEPARATOR";
		case VK_SUBTRACT:return"SUBTRACT";
		case VK_DECIMAL:return"DECIMAL";
		case VK_DIVIDE:return"DIVIDE";
		case VK_F1:return"F1";
		case VK_F2:return"F2";
		case VK_F3:return"F3";
		case VK_F4:return"F4";
		case VK_F5:return"F5";
		case VK_F6:return"F6";
		case VK_F7:return"F7";
		case VK_F8:return"F8";
		case VK_F9:return"F9";
		case VK_F10:return"F10";
		case VK_F11:return"F11";
		case VK_F12:return"F12";
		case VK_F13:return"F13";
		case VK_F14:return"F14";
		case VK_F15:return"F15";
		case VK_F16:return"F16";
		case VK_F17:return"F17";
		case VK_F18:return"F18";
		case VK_F19:return"F19";
		case VK_F20:return"F20";
		case VK_F21:return"F21";
		case VK_F22:return"F22";
		case VK_F23:return"F23";
		case VK_F24:return"F24";
		case VK_NUMLOCK:return"NUMLOCK";
		case VK_SCROLL:return"SCROLL";
		case VK_LSHIFT:return"LSHIFT";
		case VK_RSHIFT:return"RSHIFT";
		case VK_LCONTROL:return"LCONTROL";
		case VK_RCONTROL:return"RCONTROL";
		case VK_LMENU:return"LMENU";
		case VK_RMENU:return"RMENU";
		case VK_BROWSER_BACK:return"BROWSER_BACK";
		case VK_BROWSER_FORWARD:return"BROWSER_FORWARD";
		case VK_BROWSER_REFRESH:return"BROWSER_REFRESH";
		case VK_BROWSER_STOP:return"BROWSER_STOP";
		case VK_BROWSER_SEARCH:return"BROWSER_SEARCH";
		case VK_BROWSER_FAVORITES:return"BROWSER_FAVORITES";
		case VK_BROWSER_HOME:return"BROWSER_HOME";
		case VK_VOLUME_MUTE:return"VOLUME_MUTE";
		case VK_VOLUME_DOWN:return"VOLUME_DOWN";
		case VK_VOLUME_UP:return"VOLUME_UP";
		case VK_MEDIA_NEXT_TRACK:return"MEDIA_NEXT_TRACK";
		case VK_MEDIA_PREV_TRACK:return"MEDIA_PREV_TRACK";
		case VK_MEDIA_STOP:return"MEDIA_STOP";
		case VK_MEDIA_PLAY_PAUSE:return"MEDIA_PLAY_PAUSE";
		case VK_LAUNCH_MAIL:return"LAUNCH_MAIL";
		case VK_LAUNCH_MEDIA_SELECT:return"LAUNCH_MEDIA_SELECT";
		case VK_LAUNCH_APP1:return"LAUNCH_APP1";
		case VK_LAUNCH_APP2:return"LAUNCH_APP2";
		case VK_OEM_1:return";:";
		case VK_OEM_PLUS:return"+";
		case VK_OEM_COMMA:return",";
		case VK_OEM_MINUS:return"-";
		case VK_OEM_PERIOD:return".";
		case VK_OEM_2:return"/?";
		case VK_OEM_3:return"`~";
		case VK_OEM_4:return"[{";
		case VK_OEM_5:return"\\|";
		case VK_OEM_6:return"]}";
		case VK_OEM_7:return"'\"";
		case VK_OEM_8:return"OEM8";
		case VK_OEM_102:return"OEM102";
		case VK_PROCESSKEY:return"PROCESS";
		case VK_ATTN:return"ATTN";
		case VK_CRSEL:return"CRSEL";
		case VK_EXSEL:return"EXSEL";
		case VK_EREOF:return"EREOF";
		case VK_PLAY:return"PLAY";
		case VK_ZOOM:return"ZOOM";
		case VK_PA1:return"PA1";
		case VK_OEM_CLEAR:return"CLEAR";
		}
	}
}
