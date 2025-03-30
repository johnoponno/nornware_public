#pragma once

#include "../microlib/c_fixed_string.h"
#include "../microlib/c_vector.h"

struct micron_t;

#define MLM_D_BRUSH_MAX_ASPECT 8

namespace mlm
{
	struct m_immutable_t;
	struct m_mutable_t;

	struct vc_assets_t;
	struct vc_fatpack_t;

	enum struct d_mode_t
	{
		PLAY,
		WORLD,
		TILE,
		TYPE,
		ANIM,
		LOGIC,
		GFX,
		//TERMINAL,
		//MAPS,
		WANG,
	};

	struct d_gui_i
	{
		virtual void d_gui_text(const char* in_text) = 0;
		virtual bool d_gui_button(const micron_t& in_micron, const int32_t in_key, const char* in_text) = 0;
		virtual bool d_gui_radio(const micron_t& in_micron, const bool in_state, const int32_t in_key, const char* in_text) = 0;
		virtual void d_gui_draw(micron_t& out_micron) = 0;
	};

	struct d_dev_t : public d_gui_i
	{
		void d_gui_text(const char* in_text) override;
		bool d_gui_button(const micron_t& in_micron, const int32_t in_key, const char* in_text) override;
		bool d_gui_radio(const micron_t& in_micron, const bool in_state, const int32_t in_key, const char* in_text) override;
		void d_gui_draw(micron_t& out_micron) override;

		explicit d_dev_t();
		~d_dev_t();

		void set_mode(
			const m_immutable_t& in_m_im, const vc_assets_t& in_assets, const d_mode_t in_new_value,
			m_mutable_t& out_m_mu, vc_fatpack_t& out_fatpack);

		c_vec2i_t render_size(const int32_t in_backbuffer_width, const int32_t in_backbuffer_height) const;
		c_vec4i_t layout(const int32_t in_backbuffer_width, const int32_t in_backbuffer_height) const;
		bool show_cursor(const m_mutable_t& in_mu) const;
		//bool renders_soft_eh(const m_mutable_t& in_mu) const;

		//this is just dev stuff, as at runtime we just read the straight up path from the data
		c_path_t* _dev_ambience;
		uint32_t _dev_ambience_count;

		int32_t _edit_origin_x;
		int32_t _edit_origin_y;
		uint32_t _edit_tilepage;
		c_vec2i_t _edit_world_source;
		//uint32_t _edit_world_farplanes;
		int32_t _edit_tiles_brush[MLM_D_BRUSH_MAX_ASPECT * MLM_D_BRUSH_MAX_ASPECT];
		c_vec2i_t _edit_tiles_tile_drag_start;
		int32_t _edit_tiles_brush_aspect;
		uint32_t _edit_anim_index;
		uint32_t _edit_logic_index;
		uint32_t _edit_gfx_index;
		//const vc_named_bitmap_t* _edit_terminal_screen;
		uint32_t _edit_tiles_tile_page_active : 1;
		uint32_t _edit_tiles_drag : 1;
		uint32_t _edit_anim_pending_clear_animations : 1;

		d_mode_t _mode;
		uint32_t _set_play_mode_milliseconds;
		uint32_t _paused;
		uint32_t _tick;
		uint32_t _bit_dev : 1;
		uint32_t _bit_maximize_size : 1;
		uint32_t _bit_do_draw_gui : 1;

		struct string_t
		{
			char text[256];
			uint8_t color;
		};
		std::vector<string_t> _strings;
	};
}
