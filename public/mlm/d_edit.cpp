#include "stdafx.h"

#include "../microlib/c_math.h"
#include "../microlib/microlib.h"
#include "../micron.h"
#include "m_work.h"
#include "m_mutable.h"
#include "vc_assets.h"
#include "vc_fatpack.h"
#include "vc_octamap.h"
#include "vc_work.h"
#include "d_dev.h"

#define ASSET_TEMP_AMBIENCE "hero_immortal.ogg"

namespace mlm
{
	const char* vc_ambience(const c_vec2i_t& aScreen, const vc_assets_t& in_assets);
	bool vc_restart(const c_vec2i_t& aScreen, const vc_assets_t& in_assets);
	uint32_t vc_gfx_index_of(const uint32_t aValue, const vc_assets_t& in_assets);
	uint32_t vc_wang_blob_flags(const int32_t x, const int32_t y, const uint32_t index, const m_immutable_t& m_im);

	static vc_screen_t& __screen(
		const c_vec2i_t& in_screen,
		vc_assets_t& out_assets)
	{
		const uint32_t OFFSET = in_screen.x + in_screen.y * NINJA_VC_SCREENS_IN_WORLD_X;
		assert(OFFSET < (NINJA_VC_SCREENS_IN_WORLD_X * NINJA_VC_SCREENS_IN_WORLD_Y));
		return out_assets.screens[OFFSET];
	}

	static void __set_logic(
		const uint32_t in_index, const uint32_t in_value,
		m_immutable_t& out_im)
	{
		if (in_index < M_NUM_LOGIC)
			out_im.logic_indices[in_index] = in_value;
	}

	static void __set_tile_type(
		const c_vec2i_t& in_position, const uint32_t in_type,
		m_immutable_t& out_im)
	{
		if (m_in_world(in_position.x, in_position.y))
		{
			const uint16_t TILE = m_tile(in_position.x, in_position.y, out_im);
			out_im.tile_type[TILE] = in_type;
			out_im.tile_type[TILE] %= M_NUM_TYPE;
		}
	}

	static void __set_tile_area(
		const int32_t in_start_x, const int32_t in_start_y, const int32_t in_end_x, const int32_t in_end_y, const uint16_t in_index,
		m_immutable_t& out_im)
	{
		if (m_in_world(in_start_x, in_start_y) && m_in_world(in_end_x, in_end_y))
		{
			const int32_t SX = __min(in_start_x, in_end_x);
			const int32_t SY = __min(in_start_y, in_end_y);
			const int32_t EX = __max(in_start_x, in_end_x);
			const int32_t EY = __max(in_start_y, in_end_y);

			for (
				int32_t y = SY;
				y < EY + 1;
				++y
				)
				for (
					int32_t x = SX;
					x < EX + 1;
					++x
					)
					out_im.world_tiles[x + y * MLM_M_WORLD_WIDTH] = in_index;
		}
	}

	static void __set_tile(
		const c_vec2i_t& in_position, const uint16_t in_index,
		m_immutable_t& out_im)
	{
		if (m_in_world(in_position.x, in_position.y))
			out_im.world_tiles[in_position.x + in_position.y * MLM_M_WORLD_WIDTH] = in_index;
	}

	static void __bound_edit_origin(d_dev_t& out_dev)
	{
		if (out_dev._edit_origin_x < 0)
			out_dev._edit_origin_x = 0;

		if (out_dev._edit_origin_y < 0)
			out_dev._edit_origin_y = 0;

		if (out_dev._edit_origin_x > MLM_M_WORLD_WIDTH - NINJA_VC_TILES_ON_SCREEN_X)
			out_dev._edit_origin_x = MLM_M_WORLD_WIDTH - NINJA_VC_TILES_ON_SCREEN_X;

		if (out_dev._edit_origin_y > MLM_M_WORLD_HEIGHT - NINJA_VC_TILES_ON_SCREEN_Y)
			out_dev._edit_origin_y = MLM_M_WORLD_HEIGHT - NINJA_VC_TILES_ON_SCREEN_Y;
	}

	static c_vec2i_t __camera(const d_dev_t& in_dev)
	{
		return { in_dev._edit_origin_x * MLM_M_TILE_ASPECT, in_dev._edit_origin_y * MLM_M_TILE_ASPECT };
	}

	static void __tile_page_navigation(
		const micron_t& in_micron, const c_blob_t& in_tiles,
		d_dev_t& out_dev)
	{
		uint32_t num_tile_pages = 0;
		{
			int32_t height = FS_TGA_HEADER(in_tiles)->image_spec_height;
			while (height > 0)
			{
				height -= in_micron.canvas_height;
				++num_tile_pages;
			}
		}

		if (out_dev.d_gui_button(in_micron, 'Y', "TilePage--"))
		{
			if (out_dev._edit_tilepage)
				--out_dev._edit_tilepage;
			else
				out_dev._edit_tilepage = num_tile_pages - 1;
		}

		//if (out_hard_gui_fucker.button(out_pos.x, out_pos.y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, 'U', "TilePage++"))
		if (out_dev.d_gui_button(in_micron, 'U', "TilePage++"))
		{
			++out_dev._edit_tilepage;
			out_dev._edit_tilepage %= num_tile_pages;
		}
	}

	c_vec2i_t __cursor_tile_space(const micron_t& in_micron)
	{
		return c_vec2i_t{ in_micron.canvas_cursor_x, in_micron.canvas_cursor_y } / MLM_M_TILE_ASPECT;
	}

	static uint32_t __edit_cursor_index(const micron_t& in_micron, const c_blob_t& in_tiles, const d_dev_t& in_dev)
	{
		const int32_t TILES_ON_TILEPAGE_X = FS_TGA_HEADER(in_tiles)->image_spec_width / MLM_M_TILE_ASPECT;
		const int32_t PAGE_Y_OFFSET = in_dev._edit_tilepage * TILES_ON_TILEPAGE_X * NINJA_VC_TILES_ON_SCREEN_Y;
		const c_vec2i_t MP = __cursor_tile_space(in_micron);
		const uint32_t INDEX(MP.x + MP.y * TILES_ON_TILEPAGE_X + PAGE_Y_OFFSET);
		return INDEX;
	}

	static void __anim_input(
		const micron_t& in_micron,
		vc_assets_t& out_assets, vc_fatpack_t& out_fatpack, d_dev_t& out_dev)
	{
		//constexpr hg_widget_t WIDGET{ NINJA_DEV_BUTTON_WIDTH, NINJA_DEV_BUTTON_HEIGHT, HG_TEXT_CENTER };
		//const c_vec2i_t CS = out_dev.render_size(in_backbuffer_width, in_backbuffer_height);
		//constexpr hg_text_style_t STYLE{ HG_TEXT_SHADOW, UINT32_MAX };

		if (out_dev._edit_anim_pending_clear_animations)
		{
			out_dev.d_gui_text("are you sure you want");
			out_dev.d_gui_text("to clear all animations?");

			if (out_dev.d_gui_button(in_micron, -1, "Yes"))
			{
				vc_tiles_clear_animations(out_assets, out_fatpack);
				out_dev._edit_anim_pending_clear_animations = 0;
			}

			if (out_dev.d_gui_button(in_micron, -1, "No"))
				out_dev._edit_anim_pending_clear_animations = 0;
		}
		else
		{
			if (out_dev.d_gui_button(in_micron, -1, "Clear All Animations"))
				out_dev._edit_anim_pending_clear_animations = 1;

			out_dev.d_gui_text("<LMB>from");
			out_dev.d_gui_text("<RMB>to");

			__tile_page_navigation(in_micron, out_assets.tiles, out_dev);

			{
				//pick from
				if (micron_key_downflank(in_micron, MICRON_KEY_LMB))
					out_dev._edit_anim_index = __edit_cursor_index(in_micron, out_assets.tiles, out_dev);

				//pick to
				if (micron_key_downflank(in_micron, MICRON_KEY_RMB))
				{
					const uint32_t TO = __edit_cursor_index(in_micron, out_assets.tiles, out_dev);
					out_assets.tile_info[out_dev._edit_anim_index].anim_target = TO;

					//set from to to (could be optional)
					out_dev._edit_anim_index = TO;
				}
			}
		}
	}

	static void __edit_draw_tile_page(
		const c_blob_t& in_tiles, const d_dev_t& in_dev,
		micron_t& out_micron)
	{
		vc_octamap_blit_clip(0, (int32_t)in_dev._edit_tilepage * -out_micron.canvas_height, 0, 0, 0, 0, in_tiles, out_micron);
	}

	static void __draw_index_bitmap(
		const c_blob_t& in_tiles, const c_blob_t& in_icons, const uint32_t in_tile, const uint32_t in_icon, const d_dev_t& in_dev,
		micron_t& out_micron)
	{
		const int32_t TILES_ON_TILEPAGE_X = FS_TGA_HEADER(in_tiles)->image_spec_width / MLM_M_TILE_ASPECT;
		c_vec2i_t dst;

		dst.x = in_tile % TILES_ON_TILEPAGE_X;
		dst.y = (in_tile / TILES_ON_TILEPAGE_X) - NINJA_VC_TILES_ON_SCREEN_Y * in_dev._edit_tilepage;
		dst.x *= MLM_M_TILE_ASPECT;
		dst.y *= MLM_M_TILE_ASPECT;

		vc_octamap_blit_key_clip(
			dst.x,
			dst.y,
			FS_TGA_HEADER(in_icons)->image_spec_width,
			FS_TGA_HEADER(in_icons)->image_spec_width,
			0,
			FS_TGA_HEADER(in_icons)->image_spec_width * in_icon,
			in_icons,
			out_micron
		);
	}

	static void __gfx_input(
		const micron_t& in_micron,
		vc_assets_t& out_assets, d_dev_t& out_dev)
	{
		//const c_vec2i_t CS = out_dev.render_size(in_backbuffer_width, in_backbuffer_height);

		out_dev.d_gui_text("<LMB>set gfx to tile");
		__tile_page_navigation(in_micron, out_assets.tiles, out_dev);

		{
			//constexpr hg_widget_t BIG{ NINJA_DEV_BUTTON_WIDTH, NINJA_DEV_BUTTON_HEIGHT, HG_TEXT_CENTER };
			for (
				uint32_t i = 0;
				i < VC_NUM_GFX;
				++i
				)
			{
				if (out_dev.d_gui_radio(in_micron, (uint32_t)out_dev._edit_gfx_index == i, '1' + i, VC_GFX_NAME_DEFAULT[i].name))
					out_dev._edit_gfx_index = i;
			}
		}

		if (micron_key_upflank(in_micron, MICRON_KEY_LMB))
		{
			if (out_dev._edit_gfx_index < VC_NUM_GFX)
				out_assets.gfx[out_dev._edit_gfx_index] = __edit_cursor_index(in_micron, out_assets.tiles, out_dev);
		}
	}

	static void __logic_input(
		const micron_t& in_micron, const vc_assets_t& in_assets,
		m_immutable_t& out_im, d_dev_t& out_dev)
	{
		//const c_vec2i_t CS = out_dev.render_size(in_backbuffer_width, in_backbuffer_height);

		out_dev.d_gui_text("<LMB>set logic to tile");
		__tile_page_navigation(in_micron, in_assets.tiles, out_dev);

		{
			//constexpr hg_widget_t WIDGET{ NINJA_DEV_BUTTON_WIDTH, NINJA_DEV_BUTTON_HEIGHT, HG_TEXT_CENTER };
			for (
				uint32_t i = 0;
				i < M_NUM_LOGIC;
				++i
				)
			{
				if (out_dev.d_gui_radio(in_micron, (uint32_t)out_dev._edit_logic_index == i, '1' + i, M_LOGIC_INFO[i].name))
					out_dev._edit_logic_index = i;
			}
		}

		{
			if (micron_key_upflank(in_micron, MICRON_KEY_LMB))
				__set_logic(out_dev._edit_logic_index, __edit_cursor_index(in_micron, in_assets.tiles, out_dev), out_im);
		}
	}

	static c_vec2i_t __edit_cursor_tile_space_screen(const micron_t& in_micron, const d_dev_t& in_dev)
	{
		return (c_vec2i_t{ in_micron.canvas_cursor_x, in_micron.canvas_cursor_y } + __camera(in_dev)) / MLM_M_TILE_ASPECT;
	}

	static void __change_ambience(
		const d_dev_t& in_dev, const bool in_state,
		vc_assets_t& out_assets)
	{
		const c_vec2i_t SCREEN{ in_dev._edit_origin_x / NINJA_VC_TILES_ON_SCREEN_X, in_dev._edit_origin_y / NINJA_VC_TILES_ON_SCREEN_Y };

		if (
			SCREEN.x >= 0 &&
			SCREEN.x < NINJA_VC_SCREENS_IN_WORLD_X &&
			SCREEN.y >= 0 &&
			SCREEN.y < NINJA_VC_SCREENS_IN_WORLD_Y
			)
		{
			vc_screen_t& s = __screen(SCREEN, out_assets);
			for (
				uint32_t i = 0;
				i < in_dev._dev_ambience_count;
				++i
				)
			{
				if (in_dev._dev_ambience[i] == s.ambience)
				{
					if (in_state)
					{
						s.ambience = in_dev._dev_ambience[(i + 1) % in_dev._dev_ambience_count];
					}
					else
					{
						if (i)
							s.ambience = in_dev._dev_ambience[i - 1];
						else
							s.ambience = in_dev._dev_ambience[in_dev._dev_ambience_count - 1];
					}

					return;
				}
			}

			//search failed, assign first
			s.ambience = in_dev._dev_ambience[0];
		}
	}

	static void __change_farplane(
		const d_dev_t& in_dev, const bool in_state,
		vc_assets_t& out_assets)
	{
		const c_vec2i_t SCREEN{ in_dev._edit_origin_x / NINJA_VC_TILES_ON_SCREEN_X, in_dev._edit_origin_y / NINJA_VC_TILES_ON_SCREEN_Y };

		if (
			SCREEN.x >= 0 &&
			SCREEN.x < NINJA_VC_SCREENS_IN_WORLD_X &&
			SCREEN.y >= 0 &&
			SCREEN.y < NINJA_VC_SCREENS_IN_WORLD_Y
			)
		{
			vc_screen_t& s = __screen(SCREEN, out_assets);
			if (in_state)
			{
				++s.plax;
				s.plax %= VC_PLAX_NUM_STYLES;
			}
			else
			{
				if (s.plax)
				{
					--s.plax;
				}
				else
				{
					s.plax = VC_PLAX_NUM_STYLES - 1;
				}
			}
		}
	}


	static void __edit_common_input(
		const micron_t& in_micron,
		m_immutable_t& out_im, m_mutable_t& out_mu, vc_assets_t& out_assets, d_dev_t& out_dev)
	{
		//constexpr hg_widget_t WIDGET{ NINJA_DEV_BUTTON_WIDTH, NINJA_DEV_BUTTON_HEIGHT, HG_TEXT_CENTER };
		c_path_t slask;

		//if inside world
		const c_vec2i_t MP = __edit_cursor_tile_space_screen(in_micron, out_dev);
		if (m_in_world(MP.x, MP.y))
		{
			{
				const uint16_t TILE_INDEX = m_tile(MP.x, MP.y, out_im);
				//constexpr hg_text_style_t STYLE{ 0, UINT32_MAX };
				out_dev.d_gui_text(C_U32(slask, TILE_INDEX));
				out_dev.d_gui_text(C_U32(slask, vc_wang_blob_flags(MP.x, MP.y, TILE_INDEX, out_im)));
			}

			//level info
			if (out_dev.d_gui_button(in_micron, 'Z', "Set Start"))
				m_set_start(MP, out_im);

			//place hero
			if (out_dev.d_gui_button(in_micron, 'H', "Place Hero"))
				out_mu.hero_position = m_grid_to_world(MP.x, MP.y);
		}

		//constexpr hg_text_style_t SHADOW{ HG_TEXT_SHADOW, UINT32_MAX };

		//shift origin
		out_dev.d_gui_text(slask.format("edit_origin %d,%d (SHIFT modifies steps)", out_dev._edit_origin_x, out_dev._edit_origin_y));

		if (micron_key_is_down(in_micron, MICRON_KEY_SHIFT))
		{
			if (out_dev.d_gui_button(in_micron, MICRON_KEY_UP, "edit_origin_y -= 1"))
				--out_dev._edit_origin_y;

			if (out_dev.d_gui_button(in_micron, MICRON_KEY_LEFT, "edit_origin_x -= 1"))
				--out_dev._edit_origin_x;

			if (out_dev.d_gui_button(in_micron, MICRON_KEY_DOWN, "edit_origin_y += 1"))
				++out_dev._edit_origin_y;

			if (out_dev.d_gui_button(in_micron, MICRON_KEY_RIGHT, "edit_origin_x += 1"))
				++out_dev._edit_origin_x;
		}
		else
		{
			if (out_dev.d_gui_button(in_micron, MICRON_KEY_UP, slask.format("edit_origin_y -= %u", NINJA_VC_TILES_ON_SCREEN_Y)))
			{
				--out_dev._edit_origin_y;
				out_dev._edit_origin_y /= NINJA_VC_TILES_ON_SCREEN_Y;
				out_dev._edit_origin_y *= NINJA_VC_TILES_ON_SCREEN_Y;
			}

			if (out_dev.d_gui_button(in_micron, MICRON_KEY_LEFT, slask.format("edit_origin_x -= %u", NINJA_VC_TILES_ON_SCREEN_X)))
			{
				--out_dev._edit_origin_x;
				out_dev._edit_origin_x /= NINJA_VC_TILES_ON_SCREEN_X;
				out_dev._edit_origin_x *= NINJA_VC_TILES_ON_SCREEN_X;
			}

			if (out_dev.d_gui_button(in_micron, MICRON_KEY_DOWN, slask.format("edit_origin_y += %u", NINJA_VC_TILES_ON_SCREEN_Y)))
			{
				out_dev._edit_origin_y += NINJA_VC_TILES_ON_SCREEN_Y;
				out_dev._edit_origin_y /= NINJA_VC_TILES_ON_SCREEN_Y;
				out_dev._edit_origin_y *= NINJA_VC_TILES_ON_SCREEN_Y;
			}

			if (out_dev.d_gui_button(in_micron, MICRON_KEY_RIGHT, slask.format("edit_origin_x += %u", NINJA_VC_TILES_ON_SCREEN_X)))
			{
				out_dev._edit_origin_x += NINJA_VC_TILES_ON_SCREEN_X;
				out_dev._edit_origin_x /= NINJA_VC_TILES_ON_SCREEN_X;
				out_dev._edit_origin_x *= NINJA_VC_TILES_ON_SCREEN_X;
			}
		}
		__bound_edit_origin(out_dev);	//catch-all

		{
			const c_vec2i_t SCREEN{ out_dev._edit_origin_x / NINJA_VC_TILES_ON_SCREEN_X, out_dev._edit_origin_y / NINJA_VC_TILES_ON_SCREEN_Y };

			//farplanes
			if (out_dev.d_gui_button(in_micron, MICRON_KEY_PRIOR, "Prev bkg"))
				__change_farplane(out_dev, false, out_assets);

			if (out_dev.d_gui_button(in_micron, MICRON_KEY_NEXT, "Next bkg"))
				__change_farplane(out_dev, true, out_assets);

			//ambience
			out_dev.d_gui_text(C_STR(slask, vc_ambience(SCREEN, out_assets)));

			if (out_dev.d_gui_button(in_micron, MICRON_KEY_HOME, "Prev amb"))
				__change_ambience(out_dev, false, out_assets);

			if (out_dev.d_gui_button(in_micron, MICRON_KEY_END, "Next amb"))
				__change_ambience(out_dev, true, out_assets);

			if (out_dev.d_gui_button(in_micron, MICRON_KEY_INSERT, vc_restart(SCREEN, out_assets) ? "don't restart" : "restart"))
				__screen(SCREEN, out_assets).restart ^= 1;
		}

		//FIXME: how to support this?
		//out_dev.d_gui_text(slask.format("%.1f / %.1f", out_fatpack.ambience.current_position_seconds(), out_fatpack.ambience.pending_position_seconds()));

		//out_pos.y += NINJA_DEV_BUTTON_HEIGHT * 2;
	}

	static void __tiles_do_screen_interaction(
		const micron_t& in_micron, const c_blob_t& in_tiles,
		m_immutable_t& out_im, d_dev_t& out_dev)
	{
		const int32_t TILES_ON_TILEPAGE_X = FS_TGA_HEADER(in_tiles)->image_spec_width / MLM_M_TILE_ASPECT;
		const int32_t PAGE_Y_OFFSET = out_dev._edit_tilepage * TILES_ON_TILEPAGE_X * NINJA_VC_TILES_ON_SCREEN_Y;

		//on tilepage
		if (out_dev._edit_tiles_tile_page_active)
		{
			const c_vec2i_t MP = __cursor_tile_space(in_micron);

			//pick tile
			if (micron_key_downflank(in_micron, MICRON_KEY_RMB))
			{
				for (
					int32_t y = 0;
					y < out_dev._edit_tiles_brush_aspect;
					++y
					)
				{
					for (
						int32_t x = 0;
						x < out_dev._edit_tiles_brush_aspect;
						++x
						)
					{
						const uint32_t PICKED = MP.x + x + (MP.y + y) * TILES_ON_TILEPAGE_X + PAGE_Y_OFFSET;
						assert(PICKED < MLM_M_MAX_TILE);
						out_dev._edit_tiles_brush[x + y * MLM_D_BRUSH_MAX_ASPECT] = PICKED;
					}
				}

				//revert
				out_dev._edit_tiles_tile_page_active = false;
			}
		}
		//on world
		else
		{
			//must not be able to paint on next screen
			//possible now with bigger dev resolution
			const c_vec2i_t SP{ in_micron.canvas_cursor_x, in_micron.canvas_cursor_y };
			if (
				SP.x < in_micron.canvas_width &&
				SP.y < in_micron.canvas_height
				)
			{
				//to tile space
				const c_vec2i_t MP = __edit_cursor_tile_space_screen(in_micron, out_dev);

				//if inside world
				if (m_in_world(MP.x, MP.y))
				{
					//dragging
					if (micron_key_is_down(in_micron, MICRON_KEY_SHIFT))
					{
						//start drag paint
						if (micron_key_is_down(in_micron, MICRON_KEY_LMB))
						{
							if (!out_dev._edit_tiles_drag)
							{
								out_dev._edit_tiles_tile_drag_start = MP;
								out_dev._edit_tiles_drag = true;
							}
						}
						//end drag paint
						else
						{
							if (out_dev._edit_tiles_drag)
							{
								__set_tile_area(out_dev._edit_tiles_tile_drag_start.x, out_dev._edit_tiles_tile_drag_start.y, MP.x, MP.y, (uint16_t)out_dev._edit_tiles_brush[0], out_im);
								out_dev._edit_tiles_drag = false;
							}
						}
					}
					//not dragging
					else
					{
						//turn off dragging
						out_dev._edit_tiles_drag = false;

						//paint
						if (micron_key_is_down(in_micron, MICRON_KEY_LMB))
						{
							for (
								int32_t y = 0;
								y < out_dev._edit_tiles_brush_aspect;
								++y
								)
							{
								for (
									int32_t x = 0;
									x < out_dev._edit_tiles_brush_aspect;
									++x
									)
								{
									__set_tile({ MP.x + x, MP.y + y }, (uint16_t)out_dev._edit_tiles_brush[x + y * MLM_D_BRUSH_MAX_ASPECT], out_im);
								}
							}
						}

						//pick tile
						if (micron_key_downflank(in_micron, MICRON_KEY_RMB))
						{
							for (
								int32_t y = 0;
								y < out_dev._edit_tiles_brush_aspect;
								++y
								)
							{
								for (
									int32_t x = 0;
									x < out_dev._edit_tiles_brush_aspect;
									++x
									)
								{
									out_dev._edit_tiles_brush[x + y * MLM_D_BRUSH_MAX_ASPECT] = m_tile(MP.x + x, MP.y + y, out_im);
								}
							}
						}
					}
				}
			}
		}

		{
			//constexpr hg_text_style_t SHADOW{ HG_TEXT_SHADOW, UINT32_MAX };
			if (!out_dev._edit_tiles_tile_page_active)
				out_dev.d_gui_text("<LMB>Paint");
			out_dev.d_gui_text("<RMB>Pick");
		}

		//constexpr hg_widget_t WIDGET{ NINJA_DEV_BUTTON_WIDTH, NINJA_DEV_BUTTON_HEIGHT, HG_TEXT_CENTER };

		//step entire brush contents
		if (out_dev.d_gui_button(in_micron, 'Q', "Brush Contents Left"))
		{
			for (
				int32_t y = 0;
				y < out_dev._edit_tiles_brush_aspect;
				++y
				)
			{
				for (
					int32_t x = 0;
					x < out_dev._edit_tiles_brush_aspect;
					++x
					)
				{
					const int32_t BRUSH_OFFSET = x + y * MLM_D_BRUSH_MAX_ASPECT;
					out_dev._edit_tiles_brush[BRUSH_OFFSET] = __max(0, out_dev._edit_tiles_brush[BRUSH_OFFSET] - 1);
				}
			}
		}
		if (out_dev.d_gui_button(in_micron, 'E', "Brush Contents Right"))
		{
			for (
				int32_t y = 0;
				y < out_dev._edit_tiles_brush_aspect;
				++y
				)
			{
				for (
					int32_t x = 0;
					x < out_dev._edit_tiles_brush_aspect;
					++x
					)
				{
					const int32_t BRUSH_OFFSET = x + y * MLM_D_BRUSH_MAX_ASPECT;
					out_dev._edit_tiles_brush[BRUSH_OFFSET] = __min(MLM_M_MAX_TILE - 1, out_dev._edit_tiles_brush[BRUSH_OFFSET] + 1);
				}
			}
		}
		if (out_dev.d_gui_button(in_micron, 'X', "Brush Contents Up"))
		{
			const int32_t TILES_ON_SOURCE_X = vc_tiles_on_source_x(in_tiles);
			for (
				int32_t y = 0;
				y < out_dev._edit_tiles_brush_aspect;
				++y
				)
			{
				for (
					int32_t x = 0;
					x < out_dev._edit_tiles_brush_aspect;
					++x
					)
				{
					const int32_t BRUSH_OFFSET = x + y * MLM_D_BRUSH_MAX_ASPECT;
					out_dev._edit_tiles_brush[BRUSH_OFFSET] = __max(0, out_dev._edit_tiles_brush[BRUSH_OFFSET] - TILES_ON_SOURCE_X);
				}
			}
		}
		if (out_dev.d_gui_button(in_micron, 'C', "Brush Contents Down"))
		{
			const int32_t TILES_ON_SOURCE_X = vc_tiles_on_source_x(in_tiles);
			for (
				int32_t y = 0;
				y < out_dev._edit_tiles_brush_aspect;
				++y
				)
			{
				for (
					int32_t x = 0;
					x < out_dev._edit_tiles_brush_aspect;
					++x
					)
				{
					const int32_t BRUSH_OFFSET = x + y * MLM_D_BRUSH_MAX_ASPECT;
					out_dev._edit_tiles_brush[BRUSH_OFFSET] = __min(MLM_M_MAX_TILE - 1, out_dev._edit_tiles_brush[BRUSH_OFFSET] + TILES_ON_SOURCE_X);
				}
			}
		}

		//change brush aspect
		if (out_dev.d_gui_button(in_micron, '1', "Aspect 1"))
			out_dev._edit_tiles_brush_aspect = 1;

		if (out_dev.d_gui_button(in_micron, '2', "Aspect 2"))
			out_dev._edit_tiles_brush_aspect = 2;

		if (out_dev.d_gui_button(in_micron, '3', "Aspect 3"))
			out_dev._edit_tiles_brush_aspect = 3;

		if (out_dev.d_gui_button(in_micron, '4', "Aspect 4"))
			out_dev._edit_tiles_brush_aspect = 4;

		if (out_dev.d_gui_button(in_micron, '5', "Aspect 8"))
			out_dev._edit_tiles_brush_aspect = 8;
	}

	static void __tiles_input(
		const micron_t& in_micron,
		m_immutable_t& out_im, m_mutable_t& out_mu, vc_assets_t& out_assets, d_dev_t& out_dev)
	{
		__edit_common_input(in_micron, out_im, out_mu, out_assets, out_dev);

		//toggle tile page
		if (out_dev.d_gui_radio(in_micron, out_dev._edit_tiles_tile_page_active, 'T', out_dev._edit_tiles_tile_page_active ? "Back To World" : "To TilePages"))
			out_dev._edit_tiles_tile_page_active ^= 1;

		if (out_dev._edit_tiles_tile_page_active)
			__tile_page_navigation(in_micron, out_assets.tiles, out_dev);

		__tiles_do_screen_interaction(in_micron, out_assets.tiles, out_im, out_dev);
	}

	static void __wang_input(
		const micron_t& in_micron,
		vc_assets_t& out_assets, d_dev_t& out_dev)
	{
		//const c_vec2i_t CS = out_dev.render_size(in_backbuffer_width, in_backbuffer_height);

		{
			//constexpr hg_text_style_t SHADOW{ HG_TEXT_SHADOW, UINT32_MAX };
			out_dev.d_gui_text("<1>Wang Off");
			out_dev.d_gui_text("<2>Wang 2-Corner");
			out_dev.d_gui_text("<3>Wang Blob");
		}
		__tile_page_navigation(in_micron, out_assets.tiles, out_dev);

		//select tile for case
		{
			if (micron_key_downflank(in_micron, '1'))
				out_assets.tile_info[__edit_cursor_index(in_micron, out_assets.tiles, out_dev)].wang = WANG_OFF;

			if (micron_key_downflank(in_micron, '2'))
				out_assets.tile_info[__edit_cursor_index(in_micron, out_assets.tiles, out_dev)].wang = WANG_2_CORNER;

			if (micron_key_downflank(in_micron, '3'))
				out_assets.tile_info[__edit_cursor_index(in_micron, out_assets.tiles, out_dev)].wang = WANG_BLOB;
		}
	}

	static bool __tiles_world_is_tile_index_in_use(const uint16_t in_index, const m_immutable_t& in_im)
	{
		for (
			uint32_t o = 0;
			o < MLM_M_WORLD_SIZE;
			++o
			)
		{
			if (in_im.world_tiles[o] == in_index)
				return true;
		}

		return false;
	}

	static void __tiles_draw_brush8(
		const c_blob_t& in_tiles,
		d_dev_t& out_dev, micron_t& out_micron)
	{
		const c_vec2i_t CAMERA = __camera(out_dev);
		const int32_t TILES_ON_SOURCE_X = FS_TGA_HEADER(in_tiles)->image_spec_width / MLM_M_TILE_ASPECT;
		c_vec2i_t mp;

		//cursor tile
		mp.x = out_micron.canvas_cursor_x;
		mp.y = out_micron.canvas_cursor_y;
		mp.x -= MLM_M_TILE_ASPECT / 2;
		mp.y -= MLM_M_TILE_ASPECT / 2;
		for (
			int32_t y = 0;
			y < out_dev._edit_tiles_brush_aspect;
			++y
			)
		{
			for (
				int32_t x = 0;
				x < out_dev._edit_tiles_brush_aspect;
				++x
				)
			{
				vc_octamap_blit_clip(
					mp.x + x * MLM_M_TILE_ASPECT,
					mp.y + y * MLM_M_TILE_ASPECT,
					MLM_M_TILE_ASPECT,
					MLM_M_TILE_ASPECT,
					(out_dev._edit_tiles_brush[x + y * MLM_D_BRUSH_MAX_ASPECT] % TILES_ON_SOURCE_X) * MLM_M_TILE_ASPECT,
					(out_dev._edit_tiles_brush[x + y * MLM_D_BRUSH_MAX_ASPECT] / TILES_ON_SOURCE_X) * MLM_M_TILE_ASPECT,
					in_tiles,
					out_micron
				);
			}
		}
	}

	static void __tiles_draw_common8(
		const c_blob_t& in_tiles,
		d_dev_t& out_dev, micron_t& out_micron)
	{
		__tiles_draw_brush8(in_tiles, out_dev, out_micron);
	}

	static bool __world_cursor_in_bounds(const micron_t& in_micron)
	{
		return
			in_micron.canvas_cursor_x >= 0 &&
			in_micron.canvas_cursor_y >= 0 &&
			in_micron.canvas_cursor_x < MLM_M_WORLD_WIDTH &&
			in_micron.canvas_cursor_y < MLM_M_WORLD_HEIGHT;
	}

	static c_vec2i_t __grid_to_screen(const c_vec2i_t& in_grid)
	{
		c_vec2i_t result = in_grid;
		result.x /= NINJA_VC_TILES_ON_SCREEN_X;
		result.y /= NINJA_VC_TILES_ON_SCREEN_Y;
		return result;
	}

	static bool __world_jump_to_screen(
		const micron_t& in_micron,
		d_dev_t& out_dev)
	{
		if (__world_cursor_in_bounds(in_micron))
		{
			const c_vec2i_t s = __grid_to_screen({ in_micron.canvas_cursor_x, in_micron.canvas_cursor_y });
			out_dev._edit_origin_x = s.x * NINJA_VC_TILES_ON_SCREEN_X;
			out_dev._edit_origin_y = s.y * NINJA_VC_TILES_ON_SCREEN_Y;

			return true;
		}

		return false;
	}

	static void __world_screen_copy(
		const c_vec2i_t& in_destination,
		m_immutable_t& out_im, d_dev_t& out_dev)
	{
		//tiles
		for (
			int32_t y = 0;
			y < NINJA_VC_TILES_ON_SCREEN_Y;
			++y
			)
		{
			for (
				int32_t x = 0;
				x < NINJA_VC_TILES_ON_SCREEN_X;
				++x
				)
			{
				const c_vec2i_t SRC_POS{ out_dev._edit_world_source.x * NINJA_VC_TILES_ON_SCREEN_X + x, out_dev._edit_world_source.y * NINJA_VC_TILES_ON_SCREEN_Y + y };
				const c_vec2i_t DST_POS{ in_destination.x * NINJA_VC_TILES_ON_SCREEN_X + x, in_destination.y * NINJA_VC_TILES_ON_SCREEN_Y + y };
				const uint16_t SRC_INDEX = m_tile(SRC_POS.x, SRC_POS.y, out_im);

				__set_tile(DST_POS, SRC_INDEX, out_im);
			}
		}

		/*
		//screens
		//these are persistent, so doing a standard swap is nasty
		//FIXME: that is probably no longer true...
		vc_screen_t& src = vc_immutable_screen(mu.edit_world_source, im);
		vc_screen_t& dst = vc_immutable_screen(aDestination, im);
		{
			const c_path_t FP = src.farplane;
			const c_path_t A = src.ambience;
			const uint32_t R = src.restart;

			src.farplane = dst.farplane;
			src.ambience = dst.ambience;
			src.restart = dst.restart;

			dst.farplane = FP;
			dst.ambience = A;
			dst.restart = R;
		}
		*/
	}

	static void __world_screen_swap(
		const c_vec2i_t& in_destination,
		m_immutable_t& out_im, vc_assets_t& out_assets, d_dev_t& out_dev)
	{
		//tiles
		for (
			int32_t y = 0;
			y < NINJA_VC_TILES_ON_SCREEN_Y;
			++y
			)
		{
			for (
				int32_t x = 0;
				x < NINJA_VC_TILES_ON_SCREEN_X;
				++x
				)
			{
				const c_vec2i_t SRC_POS{ out_dev._edit_world_source.x * NINJA_VC_TILES_ON_SCREEN_X + x, out_dev._edit_world_source.y * NINJA_VC_TILES_ON_SCREEN_Y + y };
				const c_vec2i_t DST_POS{ in_destination.x * NINJA_VC_TILES_ON_SCREEN_X + x, in_destination.y * NINJA_VC_TILES_ON_SCREEN_Y + y };
				const uint16_t SRC_INDEX = m_tile(SRC_POS.x, SRC_POS.y, out_im);
				const uint16_t DST_INDEX = m_tile(DST_POS.x, DST_POS.y, out_im);

				__set_tile(SRC_POS, DST_INDEX, out_im);
				__set_tile(DST_POS, SRC_INDEX, out_im);
			}
		}

		//screens
		//these are persistent, so doing a standard swap is nasty
		//FIXME: that is probably no longer true...
		vc_screen_t& src = __screen(out_dev._edit_world_source, out_assets);
		vc_screen_t& dst = __screen(in_destination, out_assets);
		{
			const c_path_t A = src.ambience;
			const uint32_t R = src.restart;

			src.ambience = dst.ambience;
			src.restart = dst.restart;

			dst.ambience = A;
			dst.restart = R;
		}
	}

	static void __world_input(
		const micron_t& in_micron,
		m_immutable_t& out_im, vc_assets_t& out_assets, d_dev_t& out_dev)
	{
		/*
		//bkg toggle
		if (out_dev.d_gui_radio(in_micron, out_dev._edit_world_farplanes, 'F', "Farplanes"))
			out_dev._edit_world_farplanes ^= 1;
			*/

			//legend
		{
			//constexpr hg_text_style_t SHADOW{ HG_TEXT_SHADOW, UINT32_MAX };
			out_dev.d_gui_text("<RMB>Set Swap From");
			if (micron_key_is_down(in_micron, MICRON_KEY_SHIFT))
				out_dev.d_gui_text("<LMB>Do Copy To");
			else
				out_dev.d_gui_text("<LMB>Do Swap To");
			out_dev.d_gui_text("<SPACE>Edit Tiles On");
			out_dev.d_gui_text("<1>Set Room 1 (GRAY)");
			out_dev.d_gui_text("<2>Set Room 2 (ORANGE)");
			out_dev.d_gui_text("<3>Set Room 3 (BLUE)");
			out_dev.d_gui_text("<4>Set Room 4 (GREEN)");
		}

		//swapping
		if (__world_cursor_in_bounds(in_micron))
		{
			const c_vec2i_t S = __grid_to_screen({ in_micron.canvas_cursor_x, in_micron.canvas_cursor_y });

			if (micron_key_downflank(in_micron, MICRON_KEY_LMB))
			{
				if (micron_key_is_down(in_micron, MICRON_KEY_SHIFT))
					__world_screen_copy(S, out_im, out_dev);
				else
					__world_screen_swap(S, out_im, out_assets, out_dev);
			}

			if (micron_key_downflank(in_micron, MICRON_KEY_RMB))
				out_dev._edit_world_source = S;

			if (micron_key_downflank(in_micron, '1'))
				out_assets.screens[S.x + S.y * NINJA_VC_SCREENS_IN_WORLD_X].room = 0;

			if (micron_key_downflank(in_micron, '2'))
				out_assets.screens[S.x + S.y * NINJA_VC_SCREENS_IN_WORLD_X].room = 1;

			if (micron_key_downflank(in_micron, '3'))
				out_assets.screens[S.x + S.y * NINJA_VC_SCREENS_IN_WORLD_X].room = 2;

			if (micron_key_downflank(in_micron, '4'))
				out_assets.screens[S.x + S.y * NINJA_VC_SCREENS_IN_WORLD_X].room = 3;
		}
	}


	static void __world_draw_entity_port8(
		const c_vec2f_t& aWorld, const uint8_t aColor,
		micron_t& out_micron)
	{
		const c_vec2i_t S = m_world_to_grid(aWorld);
		vc_canvas_set_pixel(S.x, S.y, aColor, out_micron);
	}

	static void __world_output16_port8(
		const m_immutable_t& in_im, const m_mutable_t& in_mu, const vc_assets_t& in_assets,
		d_dev_t& out_dev, micron_t& out_micron)
	{
		{//tiles
			for (
				int32_t y = 0;
				y < MLM_M_WORLD_HEIGHT;
				++y
				)
			{
				for (
					int32_t x = 0;
					x < MLM_M_WORLD_WIDTH;
					++x
					)
				{
					uint8_t air = in_assets.gray;
					uint8_t solid = in_assets.dark_gray;
					{
						const uint32_t SCREEN_OFFSET = (x / NINJA_VC_TILES_ON_SCREEN_X) + (y / NINJA_VC_TILES_ON_SCREEN_Y) * NINJA_VC_SCREENS_IN_WORLD_X;
						if (SCREEN_OFFSET < (NINJA_VC_SCREENS_IN_WORLD_X * NINJA_VC_SCREENS_IN_WORLD_Y))
						{
							switch (in_assets.screens[SCREEN_OFFSET].room)
							{
							default:
								assert(0);
								break;

							case 0:
								break;

							case 1:
								air = in_assets.orange;
								solid = in_assets.dark_orange;
								break;

							case 2:
								air = in_assets.light_blue;
								solid = in_assets.blue;
								break;

							case 3:
								air = in_assets.green;
								solid = in_assets.dark_green;
								break;
							}
						}
					}

					const uint16_t TILE = m_tile(x, y, in_im);
					const uint8_t VISITED = m_visited(x, y, in_mu);
					const uint32_t TYPE = m_tile_type(TILE, false, in_im, in_mu);
					uint8_t color;

					switch (TYPE)
					{
					case M_TYPE_SPIKES:
					case M_TYPE_LIQUID:
					case M_TYPE_FIRE:
					case M_TYPE_ELECTRICITY:
						color = in_assets.dark_red;
						break;

					default:
						if (m_logic_is(M_LOGIC_FLOWER, TILE, in_im))
						{
							if (MLM_M_VISITED_NOT == VISITED)
								color = in_assets.orange;
							else
								color = air;
						}
						else if (
							m_logic_is(M_LOGIC_WALKER, TILE, in_im) ||
							m_logic_is(M_LOGIC_WALKER2, TILE, in_im)
							)
						{
							color = in_assets.blue;
						}
						else if (
							m_logic_is(M_LOGIC_JUMPER, TILE, in_im) ||
							m_logic_is(M_LOGIC_JUMPER2, TILE, in_im)
							)
						{
							color = in_assets.that_pink;
						}
						else
						{
							if (M_TYPE_AIR == TYPE)
								color = air;
							else
								color = solid;
						}
						break;
					}

					//if (!out_dev._edit_world_farplanes || color != air)
					vc_canvas_clear_set(color, x, y, 1, 1, out_micron);

					switch (VISITED)
					{
					case MLM_M_VISITED_CHECKPOINT:
						vc_canvas_set_pixel(x * 2, y * 2, in_assets.green, out_micron);
						break;

					case MLM_M_VISITED:
						vc_canvas_set_pixel(x * 2, y * 2, in_assets.yellow, out_micron);
						break;
					}
				}
			}
		}//tiles

		//hero
		__world_draw_entity_port8(in_mu.hero_position, in_assets.white, out_micron);

		{
			const uint8_t DANGER = c_frame(out_dev._tick * M_SECONDS_PER_TICK, 4.f, 2) ? in_assets.red : in_assets.dark_red;

			for (const m_mob_t& MOB : in_mu.mobs)
			{
				switch (MOB.type)
				{
				default:
					assert(0);
					break;

				case M_LOGIC_WALKER:
				case M_LOGIC_WALKER2:
					__world_draw_entity_port8(MOB.position, DANGER, out_micron);
					break;

				case M_LOGIC_JUMPER:
				case M_LOGIC_JUMPER2:
					__world_draw_entity_port8(MOB.position, DANGER, out_micron);
					break;

				case M_LOGIC_SWIMMER:
				case M_LOGIC_SWIMMER2:
					__world_draw_entity_port8(MOB.position, DANGER, out_micron);
					break;
				}
			}
		}

		//screen edges
		for (
			int32_t i = 0;
			i < NINJA_VC_SCREENS_IN_WORLD_Y;
			++i
			)
			vc_canvas_h_line(0, MLM_M_WORLD_WIDTH, i * NINJA_VC_TILES_ON_SCREEN_Y, in_assets.dark_gray, out_micron);

		for (
			int32_t i = 0;
			i < NINJA_VC_SCREENS_IN_WORLD_X;
			++i
			)
			vc_canvas_v_line(i * NINJA_VC_TILES_ON_SCREEN_X, 0, MLM_M_WORLD_HEIGHT, in_assets.dark_gray, out_micron);
	}

	static void __draw_type(
		const m_immutable_t& in_im, const m_mutable_t& in_m_mu, const vc_assets_t& in_assets, const d_dev_t& in_d_mu,
		micron_t& out_micron)
	{
		const c_vec2i_t camera = __camera(in_d_mu);

		for (
			int32_t y = 0;
			y < MLM_M_WORLD_HEIGHT;
			++y
			)
		{
			for (
				int32_t x = 0;
				x < MLM_M_WORLD_WIDTH;
				++x
				)
			{
				const uint32_t type = m_tile_type(m_tile(x, y, in_im), false, in_im, in_m_mu);
				if (M_TYPE_AIR != type)
				{
					vc_octamap_blit_key_clip(
						(x * MLM_M_TILE_ASPECT) - camera.x,
						(y * MLM_M_TILE_ASPECT) - camera.y,
						FS_TGA_HEADER(in_assets.special_icons)->image_spec_width,
						FS_TGA_HEADER(in_assets.special_icons)->image_spec_width,
						0,
						FS_TGA_HEADER(in_assets.special_icons)->image_spec_width * type,
						in_assets.special_icons,
						out_micron
					);
				}
			}
		}
	}

	static void __type_input(
		const micron_t& in_micron,
		m_immutable_t& out_im, m_mutable_t& out_mu, vc_assets_t& out_assets, d_dev_t& out_dev)
	{
		__edit_common_input(in_micron, out_im, out_mu, out_assets, out_dev);

		{
			//constexpr hg_text_style_t SHADOW{ HG_TEXT_SHADOW, UINT32_MAX };
			out_dev.d_gui_text("<1>Air");
			out_dev.d_gui_text("<2>Solid");
			out_dev.d_gui_text("<3>Platform");
			out_dev.d_gui_text("<4>Liquid");
			out_dev.d_gui_text("<5>Passage");
			out_dev.d_gui_text("<6>Abduct");
			out_dev.d_gui_text("<7>Spikes");
			out_dev.d_gui_text("<8>Fire");
			out_dev.d_gui_text("<9>Electricity");
		}

		//if inside world
		const c_vec2i_t MP = __edit_cursor_tile_space_screen(in_micron, out_dev);
		if (m_in_world(MP.x, MP.y))
		{
			if (micron_key_is_down(in_micron, '1'))
				__set_tile_type(MP, M_TYPE_AIR, out_im);

			if (micron_key_is_down(in_micron, '2'))
				__set_tile_type(MP, M_TYPE_SOLID, out_im);

			if (micron_key_is_down(in_micron, '3'))
				__set_tile_type(MP, M_TYPE_PLATFORM, out_im);

			if (micron_key_is_down(in_micron, '4'))
				__set_tile_type(MP, M_TYPE_LIQUID, out_im);

			if (micron_key_is_down(in_micron, '5'))
				__set_tile_type(MP, M_TYPE_PASSAGE, out_im);

			if (micron_key_is_down(in_micron, '6'))
				__set_tile_type(MP, M_TYPE_ABDUCT, out_im);

			if (micron_key_is_down(in_micron, '7'))
				__set_tile_type(MP, M_TYPE_SPIKES, out_im);

			if (micron_key_is_down(in_micron, '8'))
				__set_tile_type(MP, M_TYPE_FIRE, out_im);

			if (micron_key_is_down(in_micron, '9'))
				__set_tile_type(MP, M_TYPE_ELECTRICITY, out_im);

		}
	}

	static void __tiles_draw_tile_usage_port8(
		const m_immutable_t& in_im, const vc_assets_t& in_assets,
		micron_t& out_micron)
	{
		for (
			int32_t y = 0;
			y < out_micron.canvas_width / MLM_M_TILE_ASPECT;
			++y
			)
		{
			for (
				int32_t x = 0;
				x < FS_TGA_HEADER(in_assets.tiles)->image_spec_width / MLM_M_TILE_ASPECT;
				++x
				)
			{
				const int32_t TILE = x + y * (FS_TGA_HEADER(in_assets.tiles)->image_spec_width / MLM_M_TILE_ASPECT);
				assert(TILE >= 0 && TILE <= 0xffff);
				if (__tiles_world_is_tile_index_in_use((uint16_t)TILE, in_im))
				{
					vc_canvas_clear_stipple(
						in_assets.dark_gray,
						x * MLM_M_TILE_ASPECT,
						y * MLM_M_TILE_ASPECT,
						MLM_M_TILE_ASPECT,
						MLM_M_TILE_ASPECT,
						out_micron
					);
				}
			}
		}
	}

	static void __tiles_draw_brush16_port8(
		const vc_assets_t& in_assets,
		micron_t& out_micron, d_dev_t& out_dev)
	{
		const c_vec2i_t CAMERA = __camera(out_dev);
		c_vec2i_t start;
		c_vec2i_t end;
		c_vec2i_t mp;

		//cursor tile
		mp.x = out_micron.canvas_cursor_x;
		mp.y = out_micron.canvas_cursor_y;
		mp.x -= MLM_M_TILE_ASPECT / 2;
		mp.y -= MLM_M_TILE_ASPECT / 2;

		//drag
		if (out_dev._edit_tiles_drag)
		{
			start = out_dev._edit_tiles_tile_drag_start;
			start.x *= MLM_M_TILE_ASPECT;
			start.y *= MLM_M_TILE_ASPECT;
			start -= CAMERA;

			end = mp;
			end += CAMERA;
			end.x /= MLM_M_TILE_ASPECT;
			end.y /= MLM_M_TILE_ASPECT;
			end.x++;
			end.y++;
			end.x *= MLM_M_TILE_ASPECT;
			end.y *= MLM_M_TILE_ASPECT;
			end -= CAMERA;

			vc_canvas_rect(start.x, start.y, end.x, end.y, in_assets.white, out_micron);
		}
	}

	static void __draw_cursor_port8(
		const vc_assets_t& in_assets, const uint8_t in_size, const bool in_edge_flag,
		micron_t& out_micron)
	{
		c_vec2i_t mp{ out_micron.canvas_cursor_x, out_micron.canvas_cursor_y };

		mp /= MLM_M_TILE_ASPECT;
		mp *= MLM_M_TILE_ASPECT;

		if (in_edge_flag)
		{
			vc_canvas_h_line(0, MLM_M_TILE_ASPECT, mp.y, in_assets.dark_gray, out_micron);
			vc_canvas_h_line(0, MLM_M_TILE_ASPECT, mp.y + MLM_M_TILE_ASPECT * in_size - 1, in_assets.dark_gray, out_micron);
			vc_canvas_h_line(out_micron.canvas_width - MLM_M_TILE_ASPECT, out_micron.canvas_width, mp.y, in_assets.dark_gray, out_micron);
			vc_canvas_h_line(out_micron.canvas_width - MLM_M_TILE_ASPECT, out_micron.canvas_width, mp.y + MLM_M_TILE_ASPECT * in_size - 1, in_assets.dark_gray, out_micron);

			vc_canvas_v_line(mp.x, 0, MLM_M_TILE_ASPECT, in_assets.dark_gray, out_micron);
			vc_canvas_v_line(mp.x + MLM_M_TILE_ASPECT * in_size - 1, 0, MLM_M_TILE_ASPECT, in_assets.dark_gray, out_micron);
			vc_canvas_v_line(mp.x, out_micron.canvas_height - MLM_M_TILE_ASPECT, out_micron.canvas_height, in_assets.dark_gray, out_micron);
			vc_canvas_v_line(mp.x + MLM_M_TILE_ASPECT * in_size - 1, out_micron.canvas_height - MLM_M_TILE_ASPECT, out_micron.canvas_height, in_assets.dark_gray, out_micron);
		}

		vc_canvas_clear_stipple(in_assets.gray, mp.x, mp.y, MLM_M_TILE_ASPECT, MLM_M_TILE_ASPECT, out_micron);
	}

	static void __tiles_draw_common16_port8(
		const vc_assets_t& in_assets,
		micron_t& out_micron, d_dev_t& out_dev)
	{
		__tiles_draw_brush16_port8(in_assets, out_micron, out_dev);
		__draw_cursor_port8(in_assets, (uint8_t)out_dev._edit_tiles_brush_aspect, true, out_micron);
	}

	static void __draw_index_rect_port8(
		const bool in_shrink, const c_blob_t& in_tiles, const uint8_t& in_color, const uint32_t in_tile,
		micron_t& out_micron, d_dev_t& out_dev)
	{
		const int32_t TILES_ON_TILEPAGE_X = FS_TGA_HEADER(in_tiles)->image_spec_width / MLM_M_TILE_ASPECT;
		c_vec2i_t dst;

		dst.x = in_tile % TILES_ON_TILEPAGE_X;
		dst.y = (in_tile / TILES_ON_TILEPAGE_X) - NINJA_VC_TILES_ON_SCREEN_Y * out_dev._edit_tilepage;
		dst.x *= MLM_M_TILE_ASPECT;
		dst.y *= MLM_M_TILE_ASPECT;

		if (in_shrink)
			vc_canvas_rect(dst.x + 1, dst.y + 1, dst.x + MLM_M_TILE_ASPECT - 2, dst.y + MLM_M_TILE_ASPECT - 2, in_color, out_micron);
		else
			vc_canvas_rect(dst.x, dst.y, dst.x + MLM_M_TILE_ASPECT - 1, dst.y + MLM_M_TILE_ASPECT - 1, in_color, out_micron);
	}

	//public
	//public
	//public
	//public

	bool d_edit_input(
		const micron_t& in_micron,
		m_immutable_t& out_im, m_mutable_t& out_mu, vc_assets_t& out_assets, vc_fatpack_t& out_fatpack, d_dev_t& out_dev)
	{
		//jump to screen
		if (
			d_mode_t::WORLD == out_dev._mode &&
			micron_key_upflank(in_micron, MICRON_KEY_SPACE) &&
			__world_jump_to_screen(in_micron, out_dev)
			)
			out_dev._mode = d_mode_t::TILE;

		//tool-specific gui
		//c_vec2i_t p{ out_dev.render_size(in_backbuffer_width, in_backbuffer_height).x, 0 };
		switch (out_dev._mode)
		{
		default:
			assert(0);
			break;

		case d_mode_t::WORLD:
			__world_input(in_micron, out_im, out_assets, out_dev);
			break;

		case d_mode_t::TILE:
			__tiles_input(in_micron, out_im, out_mu, out_assets, out_dev);
			break;

		case d_mode_t::TYPE:
			__type_input(in_micron, out_im, out_mu, out_assets, out_dev);
			break;

		case d_mode_t::ANIM:
			__anim_input(in_micron, out_assets, out_fatpack, out_dev);
			break;

		case d_mode_t::LOGIC:
			__logic_input(in_micron, out_assets, out_im, out_dev);
			break;

		case d_mode_t::GFX:
			__gfx_input(in_micron, out_assets, out_dev);
			break;

		case d_mode_t::WANG:
			__wang_input(in_micron, out_assets, out_dev);
			break;
		}

		//hero movement always available
		vc_do_hero_movement(in_micron, out_im, out_mu);

		return true;
	}

	void d_edit_output8(
		const m_immutable_t& in_im, const m_mutable_t& in_m_mu, const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack, d_dev_t& out_dev, micron_t& out_micron)
	{
		const c_vec2i_t CAMERA = __camera(out_dev);

		{
			bool draw_world = false;
			switch (out_dev._mode)
			{
			default:
				assert(0);
				break;

			case d_mode_t::TILE:
				draw_world = !out_dev._edit_tiles_tile_page_active;
				break;

			case d_mode_t::TYPE:
				//case d_mode_t::TERMINAL:
				draw_world = true;
				break;

			case d_mode_t::WORLD:
			case d_mode_t::ANIM:
			case d_mode_t::LOGIC:
			case d_mode_t::GFX:
			case d_mode_t::WANG:
				draw_world = false;
				break;
			}
			if (draw_world)
			{
				vc_draw_plax(out_dev._tick, in_im, in_m_mu, in_assets, CAMERA, out_fatpack.prng, out_micron);
				vc_tiles_draw(in_im, in_m_mu, in_assets, CAMERA, NINJA_VC_TILEFLAGS_NON_PASSAGE | NINJA_VC_TILEFLAGS_PASSAGE, out_fatpack, out_micron);
				vc_draw_hero(out_dev._tick, in_im, in_m_mu, CAMERA, in_assets, out_fatpack, out_micron);
			}
		}

		switch (out_dev._mode)
		{
		default:
			assert(0);
			break;

		case d_mode_t::WORLD:
			break;

		case d_mode_t::TILE:
		{
			if (out_dev._edit_tiles_tile_page_active)
			{
				__edit_draw_tile_page(in_assets.tiles, out_dev, out_micron);
				__tiles_draw_common8(in_assets.tiles, out_dev, out_micron);
			}
			else
			{
				//if (!out_hard_gui._cursor_inside)
				__tiles_draw_common8(in_assets.tiles, out_dev, out_micron);
			}
		}
		break;

		case d_mode_t::TYPE:
			__draw_type(in_im, in_m_mu, in_assets, out_dev, out_micron);
			break;

		case d_mode_t::ANIM:
			__edit_draw_tile_page(in_assets.tiles, out_dev, out_micron);
			break;

		case d_mode_t::LOGIC:
			__edit_draw_tile_page(in_assets.tiles, out_dev, out_micron);
			break;

		case d_mode_t::GFX:
			__edit_draw_tile_page(in_assets.tiles, out_dev, out_micron);
			break;

			/*
		case d_mode_t::TERMINAL:
			if (!mu.edit_terminal_screen)
				mu.edit_terminal_screen = im.terminal_content;
			assert(mu.edit_terminal_screen);
			vc_draw_terminals(im, CAMERA, m_world_to_offset(m_mu.hero_position), canvas8);
		break;
		*/

		case d_mode_t::WANG:
		{
			__edit_draw_tile_page(in_assets.tiles, out_dev, out_micron);
			for (
				uint32_t tile = 0;
				tile < MLM_M_MAX_TILE;
				++tile
				)
			{
				if (in_assets.tile_info[tile].wang)
					__draw_index_bitmap(in_assets.tiles, in_assets.special_icons, tile, 8 + in_assets.tile_info[tile].wang, out_dev, out_micron);
			}
		}
		break;
		}
	}

	void d_edit_output16_port8(
		const m_immutable_t& in_im, const m_mutable_t& in_mu, const vc_assets_t& in_assets,
		d_dev_t& out_dev, micron_t& out_micron)
	{
		const c_vec2i_t CAMERA = __camera(out_dev);

		switch (out_dev._mode)
		{
		default:
			assert(0);
			break;

		case d_mode_t::WORLD:
			__world_output16_port8(in_im, in_mu, in_assets, out_dev, out_micron);
			break;

		case d_mode_t::TILE:
		{
			if (out_dev._edit_tiles_tile_page_active)
			{
				if (micron_key_is_down(out_micron, MICRON_KEY_SHIFT))
					__tiles_draw_tile_usage_port8(in_im, in_assets, out_micron);
				__tiles_draw_common16_port8(in_assets, out_micron, out_dev);
			}
			else
			{
				//if (!out_hard_gui._cursor_inside)
				__tiles_draw_common16_port8(in_assets, out_micron, out_dev);
			}
		}
		break;

		case d_mode_t::TYPE:
			__draw_cursor_port8(in_assets, 1, false, out_micron);
			break;

		case d_mode_t::ANIM:
		{
			__draw_index_rect_port8(false, in_assets.tiles, in_assets.green, out_dev._edit_anim_index, out_micron, out_dev);
			__draw_index_rect_port8(true, in_assets.tiles, in_assets.red, in_assets.tile_info[out_dev._edit_anim_index].anim_target, out_micron, out_dev);
			__draw_cursor_port8(in_assets, 1, true, out_micron);
		}
		break;

		case d_mode_t::LOGIC:
			__draw_index_rect_port8(false, in_assets.tiles, in_assets.green, m_logic_index_of(out_dev._edit_logic_index, in_im), out_micron, out_dev);
			__draw_cursor_port8(in_assets, 1, true, out_micron);
			break;

		case d_mode_t::GFX:
			__draw_index_rect_port8(false, in_assets.tiles, in_assets.green, vc_gfx_index_of(out_dev._edit_gfx_index, in_assets), out_micron, out_dev);
			__draw_cursor_port8(in_assets, 1, true, out_micron);
			break;

		case d_mode_t::WANG:
			__draw_cursor_port8(in_assets, 1, true, out_micron);
			break;
		}

		out_micron.music = ASSET_TEMP_AMBIENCE;
	}
}
