#include "stdafx.h"

#include "../micron.h"
#include "m_work.h"
#include "vc_fatpack.h"
#include "vc_assets.h"
#include "vc_octamap.h"
#include "vc_work.h"

//#define ASSET_IDLE_MUSIC "oga_japanese_hacks.ogg"

namespace mlm
{
	static bool __main(
		const micron_t& in_micron, const m_immutable_t& in_im, const uint32_t in_tick,
		m_mutable_t& out_m_mu, vc_fatpack_t& out_fatpack)
	{
		int32_t x;
		int32_t y;

		vc_gui_big_text(in_micron, x = in_micron.canvas_width / 8, y = 16, -1, "MicroLoMania", out_fatpack);

		if (MLM_VC_GUI_LEFT & vc_gui_big_text(in_micron, x, y += 16, 'N', "New Game", out_fatpack))
		{
			m_restart(in_tick, m_mode_t::PLAY, in_im, out_m_mu);
			out_fatpack.prng = c_xorshift128_t::make();
			vc_calculate_flowers(in_im, out_m_mu, out_fatpack);
		}

		if (MLM_VC_GUI_LEFT & vc_gui_big_text(in_micron, x, y += 8, 'C', "Credits", out_fatpack))
			out_fatpack.idle_screen = VC_SCREEN_CREDITS;

		if (MLM_VC_GUI_LEFT & vc_gui_big_text(in_micron, x, y += 8, 'X', "Exit", out_fatpack))
			return false;

		return true;
	}

	static void __credits(
		const micron_t& in_micron,
		vc_fatpack_t& out_fatpack)
	{
		const char* CREDITS[] =
		{
			"CODE",
			"Philip Von Schwerin",
			"Johannes 'johno' Norneby",
			nullptr,
			"WORLD",
			"Elias",
			nullptr,
			"ART",
			"Mathilde",
			"Elias",
			nullptr,
			"MUSIC / SOUND",
			"Isabel",
			nullptr,
		};

		{
			const int32_t X = in_micron.canvas_width / 8;
			int32_t y = 8;
			for (
				uint32_t i = 0;
				i < _countof(CREDITS);
				++i
				)
			{
				if (CREDITS[i])
				{
					vc_gui_big_text(in_micron, X, y, -1, CREDITS[i], out_fatpack);
					y += 8;
				}
				else
				{
					y += 8 / 2;
				}
			}
		}

		if (MLM_VC_GUI_LEFT & vc_gui_big_text(in_micron, in_micron.canvas_width / 2, in_micron.canvas_height - 16, 'B', "Back", out_fatpack))
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

		out_micron.music = ASSET_AMBIENCE;

#if 0
		{//palette vis
			constexpr int32_t MAG = 4;
			for (uint32_t i = 0; i < 256; ++i)
				for (int32_t y = 0; y < MAG; ++y)
					for (int32_t x = 0; x < MAG; ++x)
						out_micron.canvas[(MAG * (i % 16) + x) + (MAG * (i / 16) + y) * out_micron.canvas_width] = (uint8_t)i;
		}//palette vis
#endif

#if 0
		vc_canvas_atascii_print(0, out_micron.canvas_height - 8, 0, "the xp was here...", out_micron);
#endif
	}
}
