#include "stdafx.h"

#include "../micron.h"
#include "m_work.h"
#include "vc_fatpack.h"
#include "vc_assets.h"
#include "vc_octamap.h"
#include "vc_work.h"

//#define ASSET_IDLE_MUSIC "ambience/ToobMenu01.ogg"
#define ASSET_IDLE_MUSIC "ambience/oga_japanese_hacks.ogg"

#define CREDITS_X 210
#define SCORES_X 190
#define DONE_X 230
#define MAX_NAME 9
#define NAME_INPUT_WAIT 2.f
#define SCORES_ON_SCREEN 12
#define MENU_Y NINJA_VC_OCTAFONT_HEIGHT

namespace mlm
{
	static bool __main(
		const micron_t& in_micron, const m_immutable_t& in_im, const uint32_t in_tick,
		m_mutable_t& out_m_mu, vc_fatpack_t& out_fatpack)
	{
		if (MLM_VC_GUI_LEFT & vc_gui_big_text(in_micron, in_micron.canvas_width / 2, MENU_Y + NINJA_VC_OCTAFONT_HEIGHT * 2, -1, "Credits", out_fatpack))
			out_fatpack.idle_screen = VC_SCREEN_CREDITS;

		if (MLM_VC_GUI_LEFT & vc_gui_big_text(in_micron, in_micron.canvas_width / 2, MENU_Y + NINJA_VC_OCTAFONT_HEIGHT * 4, -1, "Exit", out_fatpack))
			return false;

		if (MLM_VC_GUI_LEFT & vc_gui_big_text(in_micron, in_micron.canvas_width / 2, MENU_Y, -1, "New Game", out_fatpack))
		{
			m_restart(in_tick, m_mode_t::PLAY, in_im, out_m_mu);
			out_fatpack.prng = c_xorshift128_t::make();
			vc_calculate_flowers(in_im, out_m_mu, out_fatpack);
		}

		return true;
	}

	static void __credits(
		const micron_t& in_micron,
		vc_fatpack_t& out_fatpack)
	{
		const char* CREDITS[] =
		{
			"CODE",
			"Johannes 'johno' Norneby",
			nullptr,
			"WORLD",
			"Mats Persson",
			nullptr,
			"ART",
			"Mats Persson",
			"Johannes 'johno' Norneby",
			nullptr,
			"MUSIC",
			"Tobias Carlsson",
			nullptr,
			"www.nornware.com"
		};

		{
			int32_t y = NINJA_VC_OCTAFONT_HEIGHT;
			for (
				uint32_t i = 0;
				i < _countof(CREDITS);
				++i
				)
			{
				if (CREDITS[i])
				{
					vc_gui_big_text(in_micron, in_micron.canvas_width / 2, y, -1, CREDITS[i], out_fatpack);
					y += NINJA_VC_OCTAFONT_HEIGHT;
				}
				else
				{
					y += NINJA_VC_OCTAFONT_HEIGHT / 2;
				}
			}
		}

		if (MLM_VC_GUI_LEFT & vc_gui_big_text(in_micron, in_micron.canvas_width / 2, in_micron.canvas_height - NINJA_VC_OCTAFONT_HEIGHT / 2, -1, "Back", out_fatpack))
			out_fatpack.idle_screen = VC_SCREEN_MAIN;
	}

	//public
	//public
	//public
	//public

	bool vc_idle_input(
		const micron_t& in_micron, const m_immutable_t& in_im, const uint32_t in_tick,
		m_mutable_t& out_m_mu, vc_fatpack_t& out_fatpack)
	{
		switch (out_fatpack.idle_screen)
		{
		case VC_SCREEN_MAIN:
			if (!__main(in_micron, in_im, in_tick, out_m_mu, out_fatpack))
				return false;
			break;

		case VC_SCREEN_CREDITS:
			__credits(in_micron, out_fatpack);
			break;
		}

		return true;
	}

	void vc_idle_output(
		const m_immutable_t& in_im, const uint32_t in_tick, const m_mutable_t& in_m_mu, const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack, micron_t& out_micron)
	{
		vc_draw_plax(in_tick, in_im, in_m_mu, in_assets, {}, out_fatpack.prng, out_micron);

		//vc_do_ambience(in_tick, ASSET_IDLE_MUSIC, 1.f, in_assets, out_fatpack);
		/*
		out_fatpack.request_ambience_file = ASSET_IDLE_MUSIC;
		out_fatpack.request_ambience_volume = 1.f;
		*/
		out_micron.music = ASSET_IDLE_MUSIC;

		//vc_do_hero_sound(in_tick, in_im, in_m_mu, in_assets);

#if 1
		{//palette vis
			constexpr int32_t MAG = 4;
			for (uint32_t i = 0; i < 256; ++i)
				for (int32_t y = 0; y < MAG; ++y)
					for (int32_t x = 0; x < MAG; ++x)
						out_micron.canvas[(MAG * (i % 16) + x) + (MAG * (i / 16) + y) * out_micron.canvas_width] = (uint8_t)i;
		}//palette vis
#endif

#if 1
		vc_canvas_atascii_print(0, out_micron.canvas_height - 8, 0, "the xp was here...", out_micron);
#endif
	}
}
