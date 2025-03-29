#include "stdafx.h"
#include "d_dev.h"

#include "../microlib/microlib.h"
#include "../micron.h"
#include "m_work.h"
#include "vc_assets.h"
#include "vc_fatpack.h"
#include "vc_work.h"

const char* w32_vk_name(const int32_t in_vk);

namespace m25
{
	//void d_push_constants();

	void vc_persist(const vc_assets_t& in_assets);

	//public
	//public
	//public
	//public

	void d_dev_t::d_gui_text(const char* in_text)
	{
		_gui.resize(_gui.size() + 1);
		gui_t& g = _gui[_gui.size() - 1];
		::strcpy_s(g.text, in_text);
		g.color = 2;
	}

	bool d_dev_t::d_gui_button(const micron_t& in_micron, const int32_t in_key, const char* in_text)
	{
		_gui.resize(_gui.size() + 1);
		gui_t& g = _gui[_gui.size() - 1];
		::sprintf_s(g.text, "%s-%s", w32_vk_name(in_key), in_text);
		g.color = 4;
		return micron_key_downflank(in_micron, in_key);
	}

	bool d_dev_t::d_gui_radio(const micron_t& in_micron, const bool in_state, const int32_t in_key, const char* in_text)
	{
		_gui.resize(_gui.size() + 1);
		gui_t& g = _gui[_gui.size() - 1];
		::sprintf_s(g.text, "%s-%s", w32_vk_name(in_key), in_text);
		g.color = in_state;
		return micron_key_downflank(in_micron, in_key);
	}

	void d_dev_t::d_gui_draw(micron_t& out_micron)
	{
		int32_t y = 0;
		for (const gui_t& G : _gui)
		{
			vc_canvas_atascii_print(0, y, G.color, G.text, out_micron);
			y += 8;
			if (y >= out_micron.canvas_height)
			{
				_gui.clear();
				return;
			}
		}
		_gui.clear();
	}

	//2024-07-08: all this DEPENDS on static allocation of d_app -> init to zero
	d_dev_t::d_dev_t()
	{
		assert(0 == _paused);
		assert(0 == _tick);

		assert(0 == _bit_dev);
		_bit_maximize_size = 1;

		assert(0 == _edit_tilepage);
		assert(0 == _edit_origin_x);
		assert(0 == _edit_origin_y);

		assert(0 == _edit_anim_index);
		assert(0 == _edit_anim_pending_clear_animations);

		assert(0 == _edit_world_source.x);
		assert(0 == _edit_world_source.y);
		assert(0 == _edit_world_farplanes);

		::memset(&_edit_tiles_brush, 0, sizeof(_edit_tiles_brush));
		assert(0 == _edit_tiles_tile_drag_start.x);
		assert(0 == _edit_tiles_tile_drag_start.y);
		_edit_tiles_brush_aspect = 1;
		assert(0 == _edit_tiles_tile_page_active);
		assert(0 == _edit_tiles_drag);

		//assert(nullptr == _edit_terminal_screen);

		_edit_logic_index = M_LOGIC_CHECKPOINT;

		_edit_gfx_index = VC_GFX_FLOWER_GLOW;

		_mode = d_mode_t::PLAY;

		assert(nullptr == _dev_ambience);
		assert(0 == _dev_ambience_count);
	}

	d_dev_t::~d_dev_t()
	{
		delete[] _dev_ambience;
	}

	c_vec2i_t d_dev_t::render_size(const int32_t in_backbuffer_width, const int32_t in_backbuffer_height) const
	{
		if (_bit_maximize_size)
		{
			int32_t mul = 1;
			while (1)
			{
				if (
					NINJA_VC_SCREEN_WIDTH * mul < in_backbuffer_width &&
					NINJA_VC_SCREEN_HEIGHT * mul < in_backbuffer_height
					)
				{
					++mul;
				}
				else
				{
					if (
						NINJA_VC_SCREEN_WIDTH * mul > in_backbuffer_width ||
						NINJA_VC_SCREEN_HEIGHT * mul > in_backbuffer_height
						)
						--mul;
					assert(
						NINJA_VC_SCREEN_WIDTH * mul <= in_backbuffer_width &&
						NINJA_VC_SCREEN_HEIGHT * mul <= in_backbuffer_height
					);
					return { NINJA_VC_SCREEN_WIDTH * mul, NINJA_VC_SCREEN_HEIGHT * mul };
				}
			}
		}

		return{ NINJA_VC_SCREEN_WIDTH, NINJA_VC_SCREEN_HEIGHT };
	}

	/*
	bool d_dev_t::renders_soft_eh(const m_mutable_t& in_mu) const
	{
		if (
			m_game_active(in_mu) &&
			d_mode_t::MAPS == _mode
			)
			return false;

		return true;
	}
	*/

	bool d_dev_t::show_cursor(const m_mutable_t& in_mu) const
	{
		if (
			m_game_active(in_mu) &&
			d_mode_t::PLAY == _mode
			)
			return false;

		return true;
	}

	c_vec4i_t d_dev_t::layout(const int32_t in_backbuffer_width, const int32_t in_backbuffer_height) const
	{
		c_vec4i_t result;
		{
			const c_vec2i_t SIZE = render_size(in_backbuffer_width, in_backbuffer_height);
			result.width = SIZE.x;
			result.height = SIZE.y;
		}
#if 0
		if (_bit_dev)
		{
			//render top left
			result.x = 0;
			result.y = 0;
		}
		else
		{
			//render centered
			result.x = (in_backbuffer_width - result.width) / 2;
			result.y = (in_backbuffer_height - result.height) / 2;
		}
#else
		//render centered
		result.x = (in_backbuffer_width - result.width) / 2;
		result.y = (in_backbuffer_height - result.height) / 2;
#endif
		return result;
	}

	void d_dev_t::set_mode(
		const m_immutable_t& in_m_im, const vc_assets_t& in_assets, const d_mode_t in_new_value,
		m_mutable_t& out_m_mu, vc_fatpack_t& out_fatpack)
	{
		if (d_mode_t::PLAY == in_new_value)
		{
			//const uint32_t START = ::timeGetTime();

			m_persist(in_m_im);
			vc_persist(in_assets);
			m_spawn(_tick, in_m_im, out_m_mu);
			vc_reset_animations(out_fatpack);
			//d_push_constants();

			//_set_play_mode_milliseconds = ::timeGetTime() - START;
		}
		else
		{
			vc_reset_animations(out_fatpack);
			//d_push_constants();

			//snap to hero screen
			const c_vec2i_t SCREEN = vc_hero_screen(out_m_mu);
			_edit_origin_x = SCREEN.x * NINJA_VC_TILES_ON_SCREEN_X;
			_edit_origin_y = SCREEN.y * NINJA_VC_TILES_ON_SCREEN_Y;
		}

		_mode = in_new_value;
	}
}
