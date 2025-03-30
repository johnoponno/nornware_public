#include "stdafx.h"
#include "game.h"

#include "../microlib/fs.h"
#include "../microlib/microlib.h"
#include "../micron.h"
#include "m_work.h"
#include "vc_work.h"

namespace m25
{
	const vc_screen_t& vc_world_to_screen(const c_vec2i_t& aWorld, const vc_assets_t& in_assets);

	bool vc_idle_input(
		const micron_t& in_micron, const m_immutable_t& in_im, const uint32_t in_tick,
		m_mutable_t& out_m_mu, vc_fatpack_t& out_fatpack);
	void vc_idle_output(
		const m_immutable_t& in_im, const uint32_t in_tick, const m_mutable_t& in_m_mu, const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack, micron_t& out_micron);

	void vc_play_input(
		const micron_t& in_micron, const m_immutable_t& in_im, const uint32_t in_tick,
		m_mutable_t& out_mu, vc_fatpack_t& out_fatpack, uint32_t& out_paused);
	void vc_play_output(
		const m_immutable_t& in_im, const uint32_t in_paused, const uint32_t in_tick, const m_mutable_t& in_mu, const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack, micron_t& out_micron);

	void vc_draw_cursor(
		const uint32_t in_tick, const vc_assets_t& in_assets,
		micron_t& out_micron);

	bool d_edit_input(
		const micron_t& in_micron,
		m_immutable_t& out_im, m_mutable_t& out_mu, vc_assets_t& out_assets, vc_fatpack_t& out_fatpack, d_dev_t& out_dev);
	void d_edit_output8(
		const m_immutable_t& in_im, const m_mutable_t& in_m_mu, const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack, d_dev_t& out_dev, micron_t& out_micron);
	void d_edit_output16_port8(
		const m_immutable_t& in_im, const m_mutable_t& in_mu, const vc_assets_t& in_assets,
		d_dev_t& out_dev, micron_t& out_micron);

	static bool __input(
		const micron_t& in_micron,
		m_immutable_t& out_im, m_mutable_t& out_mu, vc_assets_t& out_assets, vc_fatpack_t& out_fatpack, d_dev_t& out_dev)
	{
		if (
			d_mode_t::PLAY == out_dev._mode &&
			micron_key_downflank(in_micron, 'G')
			)
			out_dev._bit_dev ^= 1;

		c_path_t slask;

		if (
			out_dev._bit_dev &&
			m_game_active(out_mu)
			)
		{
			//const c_vec2i_t RENDER_SIZE = out_dev.render_size(d3d9_state.backbuffer_surface_desc.Width, d3d9_state.backbuffer_surface_desc.Height);

			//mode tabs
			{
				//constexpr hg_widget_t WIDGET{ NINJA_DEV_BUTTON_WIDTH, NINJA_DEV_BUTTON_HEIGHT, HG_TEXT_CENTER };

				//int32_t y = (int32_t)d3d9_state.backbuffer_surface_desc.Height - NINJA_DEV_BUTTON_HEIGHT * 11;

				//if (out_hard_gui.radio(d_mode_t::WORLD == out_dev._mode, RENDER_SIZE.x, y, WIDGET, VK_F1, "World"))
				if (out_dev.d_gui_radio(in_micron, d_mode_t::WORLD == out_dev._mode, MICRON_KEY_F1, "World"))
					out_dev.set_mode(out_im, out_assets, d_mode_t::WORLD, out_mu, out_fatpack);

				//if (out_hard_gui.radio(d_mode_t::TILE == out_dev._mode, RENDER_SIZE.x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, VK_F2, "Tiles"))
				if (out_dev.d_gui_radio(in_micron, d_mode_t::TILE == out_dev._mode, MICRON_KEY_F2, "Tiles"))
					out_dev.set_mode(out_im, out_assets, d_mode_t::TILE, out_mu, out_fatpack);

				//if (out_hard_gui.radio(d_mode_t::TYPE == out_dev._mode, RENDER_SIZE.x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, VK_F3, "Type"))
				if (out_dev.d_gui_radio(in_micron, d_mode_t::TYPE == out_dev._mode, MICRON_KEY_F3, "Type"))
					out_dev.set_mode(out_im, out_assets, d_mode_t::TYPE, out_mu, out_fatpack);

				//if (out_hard_gui.radio(d_mode_t::WANG == out_dev._mode, RENDER_SIZE.x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, VK_F4, "Wang"))
				if (out_dev.d_gui_radio(in_micron, d_mode_t::WANG == out_dev._mode, MICRON_KEY_F4, "Wang"))
					out_dev.set_mode(out_im, out_assets, d_mode_t::WANG, out_mu, out_fatpack);

				//if (out_hard_gui.radio(d_mode_t::ANIM == out_dev._mode, RENDER_SIZE.x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, VK_F5, "Anim"))
				if (out_dev.d_gui_radio(in_micron, d_mode_t::ANIM == out_dev._mode, MICRON_KEY_F5, "Anim"))
					out_dev.set_mode(out_im, out_assets, d_mode_t::ANIM, out_mu, out_fatpack);

				//if (out_hard_gui.radio(d_mode_t::LOGIC == out_dev._mode, RENDER_SIZE.x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, VK_F6, "Logic"))
				if (out_dev.d_gui_radio(in_micron, d_mode_t::LOGIC == out_dev._mode, MICRON_KEY_F6, "Logic"))
					out_dev.set_mode(out_im, out_assets, d_mode_t::LOGIC, out_mu, out_fatpack);

				//if (out_hard_gui.radio(d_mode_t::GFX == out_dev._mode, RENDER_SIZE.x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, VK_F7, "Gfx"))
				if (out_dev.d_gui_radio(in_micron, d_mode_t::GFX == out_dev._mode, MICRON_KEY_F7, "Gfx"))
					out_dev.set_mode(out_im, out_assets, d_mode_t::GFX, out_mu, out_fatpack);

				/*
#if 0
				if (im.num_terminal_content)
				{
					if (hg_radio(mode_t::TERMINAL == mu.mode, RENDER_SIZE.x, y += NINJA_DEV_BUTTON_HEIGHT, VK_F8, "Terminal", mu._hard_gui))
						__set_mode(m_im, tick, im, mode_t::TERMINAL, m_mu, mu);
				}
				else
#endif
				{
					if (out_hard_gui.toggle(false, RENDER_SIZE.x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, VK_F8, "NO TERMINAL CONTENT"))
						0;
				}
				*/

				//if (out_hard_gui.radio(d_mode_t::PLAY == out_dev._mode, RENDER_SIZE.x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, VK_F9, d_mode_t::PLAY == out_dev._mode ? "Playing..." : "Save -> Play"))
				if (out_dev.d_gui_radio(in_micron, d_mode_t::PLAY == out_dev._mode, MICRON_KEY_F8, d_mode_t::PLAY == out_dev._mode ? "Playing..." : "Save -> Play"))
					out_dev.set_mode(out_im, out_assets, d_mode_t::PLAY, out_mu, out_fatpack);

				/*
				if (out_hard_gui.toggle(false, RENDER_SIZE.x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, VK_F8, "SPACE FOR RENT"))
					0;
					*/

					/*
					if (out_hard_gui.radio(d_mode_t::MAPS == out_dev._mode, RENDER_SIZE.x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, VK_F11, "Maps"))
						out_dev.set_mode(out_im, out_assets, d_mode_t::MAPS, out_mu, out_fatpack);
						*/
			}
		}

		//temp
#if 1
		if (micron_key_is_down(in_micron, MICRON_KEY_SHIFT))
		{
			//out_hard_gui.text({ HG_TEXT_SHADOW, UINT32_MAX }, 0, 0, slask.format("%u", d3d9_state.app->_d3d9_app_frame_drops));
			//out_dev.d_gui_text(slask.format("frame_drops = %u", d3d9_state.app->_d3d9_app_frame_drops));

			if (micron_key_upflank(in_micron, 'M'))
				out_dev._bit_maximize_size ^= 1;
		}
#endif

		//current controller
		bool controller_result = true;
		if (m_game_active(out_mu))
		{
			switch (out_dev._mode)
			{
			default:
				assert(0);
				return false;

			case d_mode_t::PLAY:
				if (out_dev._bit_dev)
				{
					//constexpr hg_widget_t WIDGET{ NINJA_DEV_BUTTON_WIDTH, NINJA_DEV_BUTTON_HEIGHT, HG_TEXT_CENTER };
					//int32_t x;
					//int32_t y;

					//if (out_hard_gui.toggle(out_fatpack.bit_display_visited, x = out_dev.render_size(d3d9_state.backbuffer_surface_desc.Width, d3d9_state.backbuffer_surface_desc.Height).x, y = 0, WIDGET, 'B', "Visited"))
					if (out_dev.d_gui_radio(in_micron, out_fatpack.bit_display_visited, 'B', "Visited"))
						out_fatpack.bit_display_visited ^= 1;

					//if (out_hard_gui.toggle(out_fatpack.bit_display_hero_tests, x = out_dev.render_size(d3d9_state.backbuffer_surface_desc.Width, d3d9_state.backbuffer_surface_desc.Height).x, y = 0, WIDGET, 'N', "Hero Tests"))
					if (out_dev.d_gui_radio(in_micron, out_fatpack.bit_display_hero_tests, 'N', "Hero Tests"))
						out_fatpack.bit_display_hero_tests ^= 1;

					//if (out_hard_gui.toggle(out_fatpack.bit_display_cambits, x = out_dev.render_size(d3d9_state.backbuffer_surface_desc.Width, d3d9_state.backbuffer_surface_desc.Height).x, y = 0, WIDGET, 'C', "CamBits"))
					if (out_dev.d_gui_radio(in_micron, out_fatpack.bit_display_cambits, 'C', "CamBits"))
						out_fatpack.bit_display_cambits ^= 1;

					//if (out_hard_gui.toggle(out_fatpack.bit_play_music, x = out_dev.render_size(d3d9_state.backbuffer_surface_desc.Width, d3d9_state.backbuffer_surface_desc.Height).x, y = 0, WIDGET, 'M', "Music"))
					if (out_dev.d_gui_radio(in_micron, out_fatpack.bit_play_music, 'M', "Music"))
						out_fatpack.bit_play_music ^= 1;

					//x = out_dev.render_size(d3d9_state.backbuffer_surface_desc.Width, d3d9_state.backbuffer_surface_desc.Height).x;
					//y = 0;

					{
						const c_vec2i_t HERO_SCREEN = vc_hero_screen(out_mu);
						//out_hard_gui.cell(x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, 1, C_V2I(slask, HERO_SCREEN));
						out_dev.d_gui_text(C_V2I(slask, HERO_SCREEN));
					}

					if (out_fatpack.bit_display_cambits)
					{
						const vc_screen_t& HERO_SCREEN = vc_world_to_screen(c_floor_to_int(out_mu.hero_position), out_assets);
						//out_hard_gui.cell(x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, 1, C_U32(slask, HERO_SCREEN.room));
						out_dev.d_gui_text(C_U32(slask, HERO_SCREEN.room));

						//if (out_hard_gui.toggle(0 == HERO_SCREEN.room, x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, '1', "Room 1"))
						if (out_dev.d_gui_radio(in_micron, 0 == HERO_SCREEN.room, '1', "Room 0"))
							((vc_screen_t&)HERO_SCREEN).room = 0;

						//if (out_hard_gui.toggle(1 == HERO_SCREEN.room, x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, '2', "Room 2"))
						if (out_dev.d_gui_radio(in_micron, 1 == HERO_SCREEN.room, '2', "Room 1"))
							((vc_screen_t&)HERO_SCREEN).room = 1;

						//if (out_hard_gui.toggle(2 == HERO_SCREEN.room, x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, '3', "Room 3"))
						if (out_dev.d_gui_radio(in_micron, 2 == HERO_SCREEN.room, '3', "Room 2"))
							((vc_screen_t&)HERO_SCREEN).room = 2;

						//if (out_hard_gui.toggle(3 == HERO_SCREEN.room, x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, '4', "Room 4"))
						if (out_dev.d_gui_radio(in_micron, 3 == HERO_SCREEN.room, '4', "Room 3"))
							((vc_screen_t&)HERO_SCREEN).room = 3;
					}

					{
						const c_vec2i_t HERO_GRID = m_world_to_grid(out_mu.hero_position);
						//out_hard_gui.cell(x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, 1, C_V2I(slask, HERO_GRID));
						out_dev.d_gui_text(C_V2I(slask, HERO_GRID));
					}

					//out_hard_gui.cell(x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, 1, C_V2F(slask, out_mu.hero_position));
					out_dev.d_gui_text(C_V2F(slask, out_mu.hero_position));

					{
						c_vec2i_t screen_origin = c_floor_to_int(out_mu.hero_position);
						screen_origin.x /= NINJA_VC_SCREEN_WIDTH;
						screen_origin.x *= NINJA_VC_SCREEN_WIDTH;
						screen_origin.y /= NINJA_VC_SCREEN_HEIGHT;
						screen_origin.y *= NINJA_VC_SCREEN_HEIGHT;
						const c_vec2f_t HERO_LOCAL{ out_mu.hero_position.x - screen_origin.x, out_mu.hero_position.y - screen_origin.y };
						//out_hard_gui.cell(x, y += NINJA_DEV_BUTTON_HEIGHT, WIDGET, 1, C_V2F(slask, HERO_LOCAL));
						out_dev.d_gui_text(C_V2F(slask, HERO_LOCAL));
					}

					out_dev.d_gui_text(C_V2F(slask, out_mu.hero_speed));
					out_dev.d_gui_text(C_F32(slask, out_mu.hero_spawn_time));
					out_dev.d_gui_text(C_U32(slask, out_mu.hero_input));
					out_dev.d_gui_text(C_U32(slask, out_mu.hero_last_input));
					out_dev.d_gui_text(C_U32(slask, out_mu.hero_air));
					out_dev.d_gui_text(C_U32(slask, out_mu.hero_may_wall_jump));
					/*
#ifdef _DEBUG
					out_dev.d_gui_text(C_U32(slask, w32_actual_disk_reads));
#endif
*/
					out_dev.d_gui_text(C_U32(slask, out_dev._set_play_mode_milliseconds));
					//out_dev.d_gui_text(C_U32(slask, out_fatpack.fx.used));
					{
						extern double vc_tiles_draw_time;
						extern uint32_t vc_tiles_draw_count;
						out_dev.d_gui_text(C_F32(slask, vc_tiles_draw_time / vc_tiles_draw_count));
					}
				}
				vc_play_input(in_micron, out_im, out_dev._tick, out_mu, out_fatpack, out_dev._paused);
				break;

			case d_mode_t::WORLD:
			case d_mode_t::TILE:
			case d_mode_t::TYPE:
			case d_mode_t::ANIM:
			case d_mode_t::LOGIC:
			case d_mode_t::GFX:
				//case d_mode_t::TERMINAL:
			case d_mode_t::WANG:
				controller_result = d_edit_input(
					in_micron,
					out_im,
					out_mu,
					out_assets,
					out_fatpack,
					out_dev
				);
				break;

				/*
			case d_mode_t::MAPS:
				out_hard_maps.gui_heirachical(
					{ (int32_t)d3d9_state.backbuffer_surface_desc.Width, (int32_t)d3d9_state.backbuffer_surface_desc.Height },
					NINJA_DEV_BUTTON_WIDTH, 0,
					NINJA_DEV_BUTTON_WIDTH, NINJA_DEV_BUTTON_WIDTH, NINJA_DEV_BUTTON_WIDTH, NINJA_DEV_BUTTON_HEIGHT,
					out_hard_edit
				);
				break;
				*/
			}
		}
		else
		{
			controller_result = vc_idle_input(in_micron, out_im, out_dev._tick, out_mu, out_fatpack);
		}
		if (!controller_result)
		{
			/*
			if (out_dev._bit_dev && w32_key_is_down(VK_SHIFT))
				__dump_overview(out_im, out_assets);
				*/
			return false;
		}

		return true;
	}

	static void __output(
		const m_immutable_t& in_im, const m_mutable_t& in_mu, const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack, d_dev_t& out_dev, micron_t& out_micron)
	{
		if (!out_dev._paused)
			vc_tiles_update_animations(in_assets, out_fatpack);

		//8 bit output
		if (m_game_active(in_mu))
		{
			switch (out_dev._mode)
			{
			default:
				assert(0);
				break;

			case d_mode_t::PLAY:
				vc_play_output(in_im, out_dev._paused, out_dev._tick, in_mu, in_assets, out_fatpack, out_micron);
				break;

			case d_mode_t::WORLD:
			case d_mode_t::TILE:
			case d_mode_t::TYPE:
			case d_mode_t::ANIM:
			case d_mode_t::LOGIC:
			case d_mode_t::GFX:
				//case d_mode_t::TERMINAL:
			case d_mode_t::WANG:
				d_edit_output8(in_im, in_mu, in_assets, out_fatpack, out_dev, out_micron);
				break;

				/*
			case d_mode_t::MAPS:
				out_fatpack.ambience.cleanup();
				break;
				*/
			}
		}
		else
		{
			vc_idle_output(in_im, out_dev._tick, in_mu, in_assets, out_fatpack, out_micron);
		}

		vc_gui_draw_and_clear(in_assets, out_fatpack, out_micron);
		if (out_dev.show_cursor(in_mu))
			vc_draw_cursor(out_dev._tick, in_assets, out_micron);

#if CANVAS16
		__canvas_to_bitmap(in_assets.sd_palette, out_fatpack.canvas, out_dev._canvas16);
		if (m_game_active(in_m_mu))//16 bit output on top of 8 bit canvas
		{
			switch (out_dev._mode)
			{
			default:
				assert(0);
				break;

			case d_mode_t::PLAY:
				break;

			case d_mode_t::MAPS:
				break;

			case d_mode_t::WORLD:
			case d_mode_t::TILE:
			case d_mode_t::TYPE:
			case d_mode_t::ANIM:
			case d_mode_t::LOGIC:
			case d_mode_t::GFX:
				//case d_mode_t::TERMINAL:
			case d_mode_t::WANG:
				d_edit_output16(in_m_im, in_cursor_position, in_m_mu, in_assets, out_hard_gui, out_fatpack, out_dev);
				break;
			}
		}
#else
		if (m_game_active(in_mu))//port of 16 bit output to 8 bit output
		{
			switch (out_dev._mode)
			{
			default:
				assert(0);
				break;

			case d_mode_t::PLAY:
				break;

				/*
			case d_mode_t::MAPS:
				break;
				*/

			case d_mode_t::WORLD:
			case d_mode_t::TILE:
			case d_mode_t::TYPE:
			case d_mode_t::ANIM:
			case d_mode_t::LOGIC:
			case d_mode_t::GFX:
				//case d_mode_t::TERMINAL:
			case d_mode_t::WANG:
				d_edit_output16_port8(in_im, in_mu, in_assets, out_dev, out_micron);
				break;
			}
		}
#endif

		//gui on top
		out_dev.d_gui_draw(out_micron);
	}

	//public
	//public
	//public
	//public

	bool game_t::init(micron_t& out_micron)
	{
		out_micron.canvas_width = NINJA_VC_SCREEN_WIDTH;
		out_micron.canvas_height = NINJA_VC_SCREEN_HEIGHT;

		//m_mu (do this here to support .nwf)
		m_load(_im);

		/*
		//dev stuff
		{
			assert(nullptr == _dev._dev_ambience);
			assert(0 == _dev._dev_ambience_count);

			fs_directory_tree_t tree;
			w32_directory_scan("ambience", "ogg", tree);
			if (tree.files.size())
			{
				_dev._dev_ambience = new c_path_t[tree.files.size()];
				assert(_dev._dev_ambience);
				_dev._dev_ambience_count = tree.files.size();
				for (
					uint32_t i = 0;
					i < tree.files.size();
					++i
					)
					_dev._dev_ambience[i] = tree.files[i];
			}

			C_LOG("_dev_ambience_count %u", _dev._dev_ambience_count);
		}
		*/

		{
			if (!vc_view_init(_assets, _fatpack, out_micron))
				return false;
			assert(out_micron.sound_loads.size());

			/*
			if (!_engine.init(d3d9_state_window_handle()))
				return false;

			if (!_sounds.init())
				return false;

			for (const vc_sound_request_t& R : sound_requests)
			{
				if (!_sounds.add_sound(FS_VIRTUAL, _engine, R.file, R.id, R.channels))
					return false;
			}
			*/
		}

		//extract the palette for global use (arbitrarily taken from the cursor
		//__make_sd_palette(_assets.cursor, _sd_palette);
		//static void __make_sd_palette(
		//	const c_blob_t & in_src,
		//	uint16_t * out_sd_palette)
		{
			for (
				uint32_t i = 0;
				i < 256;
				++i
				)
			{
				//tga is bgr...
				out_micron.palette[i].r = FS_TGA_PALETTE(_assets.cursor)[i * 3 + 0];
				out_micron.palette[i].g = FS_TGA_PALETTE(_assets.cursor)[i * 3 + 1];
				out_micron.palette[i].b = FS_TGA_PALETTE(_assets.cursor)[i * 3 + 2];
			}
		}

		//cache a bunch of colors
#if 0
		_assets.orange = _assets.palettize_color(SD_ORANGE);
		_assets.dark_orange = _assets.palettize_color(127, 63, 0);
		_assets.gray = _assets.palettize_color(SD_GRAY);
		_assets.dark_gray = _assets.palettize_color(SD_DARK_GRAY);
		_assets.light_blue = _assets.palettize_color(SD_LIGHT_BLUE);
		_assets.blue = _assets.palettize_color(SD_BLUE);
		_assets.green = _assets.palettize_color(SD_GREEN);
		_assets.dark_green = _assets.palettize_color(SD_DARK_GREEN);
		_assets.red = _assets.palettize_color(SD_RED);
		_assets.dark_red = _assets.palettize_color(SD_DARK_RED);
		_assets.that_pink = _assets.palettize_color(SD_THAT_PINK);
		_assets.yellow = _assets.palettize_color(SD_YELLOW);
		_assets.white = _assets.palettize_color(SD_WHITE);
#else
		//FIXME: these need to be adjusted for the incoming palette
		_assets.orange = 61;
		_assets.dark_orange = 175;
		_assets.gray = 7;
		_assets.dark_gray = 10;
		_assets.light_blue = 164;
		_assets.blue = 3;
		_assets.green = 5;
		_assets.dark_green = 37;
		_assets.red = 6;
		_assets.dark_red = 183;
		_assets.that_pink = 2;
		_assets.yellow = 4;
		_assets.white = 0;
#endif

#if CANVAS16
		if (!_dev._canvas16.init(NINJA_VC_SCREEN_WIDTH, NINJA_VC_SCREEN_HEIGHT, 0))
			return false;
#endif

		//w32_xinput_poll_all_pads();

#if CANVAS16
		assert(_fatpack.canvas.width == _dev._canvas16.width);
		assert(_fatpack.canvas.height == _dev._canvas16.height);
#endif
		assert(out_micron.canvas_width == NINJA_VC_SCREEN_WIDTH);
		assert(out_micron.canvas_height == NINJA_VC_SCREEN_HEIGHT);

		return true;
	}

	bool game_t::tick(micron_t& out_micron)
	{
		if (!_dev._paused)
			++_dev._tick;

		if (!__input(out_micron, _im, _mu, _assets, _fatpack, _dev))
			return false;

		if (!_dev._paused)
		{
			const m_events_t EVENTS = m_update(_im, _dev._tick, _mu);
			vc_visualize_events(_im, _dev._tick, EVENTS, _mu, _assets, _fatpack, out_micron);
		}

		__output(_im, _mu, _assets, _fatpack, _dev, out_micron);

#if 0
		//play sound requests
		{
			const ds_space2d_player_t PLAYER{ out_mu.hero_position, 1.f, 1024.f };
			for (const vc_sound_play_t& SP : out_fatpack.request_sound_plays)
				PLAYER.play(in_sounds, SP.id, SP.event_position);
			out_fatpack.request_sound_plays.clear();
		}

		//hero sound
		{
			in_sounds.play_looped(

				m_game_active(out_mu) &&
				m_character_alive(out_mu) &&
				out_mu.hero_air &&
				(m_character_is_solid_left_more(out_im, out_mu) || m_character_is_solid_right(out_im, out_mu)),

				VC_SND_SLIDE,
				::fabsf(out_mu.hero_speed.y) / M_CHAR_MAX_AIR_SPEED_Y,
				0.f,
				1.f,
				nullptr
			);

			if (out_dev._tick == out_mu.hero_melee_tick)
				//vc_play(out_mu.hero_position, VC_SND_MELEE, out_mu.hero_position, in_assets);
				ds_space2d_player_t{ out_mu.hero_position, 1.f, 1024.f }.play(in_sounds, VC_SND_MELEE, out_mu.hero_position);
		}

		//service ambience requests
		if (out_fatpack.bit_play_music)
		{
			out_ambience.set(FS_VIRTUAL, in_engine, out_fatpack.request_ambience_file, true, true, out_fatpack.request_ambience_volume);
			out_ambience.update(FS_VIRTUAL, in_engine, out_dev._tick * M_SECONDS_PER_TICK, out_fatpack.request_ambience_volume);
		}
		else
		{
			out_ambience.cleanup();
		}
#endif

		/*
		if (!_dev._paused)
			++_dev._tick;
			*/

		return true;
	}

	void game_t::shutdown()
	{
		m_persist(_im);
	}
}
