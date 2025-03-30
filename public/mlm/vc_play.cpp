#include "stdafx.h"

#include "../microlib/c_math.h"
#include "../microlib/microlib.h"
#include "../micron.h"
#include "m_work.h"
#include "m_mutable.h"
#include "vc_assets.h"
#include "vc_fatpack.h"
#include "vc_work.h"
#include "vc_octamap.h"

#define CAM_LERP_SPEED 8.f
//#define ASSET_TEMP_AMBIENCE "hero_immortal.ogg"

#define ANOTHER_WORLD_RAIN_INDEX 7
#define HEAVY_FPS 8.f

namespace mlm
{
	void vc_sprite_draw(
		const c_vec2i_t& in_position, const uint8_t in_frame, const bool in_mirror, const vc_sprite_t& in_sprite,
		micron_t& out_micron);
	uint32_t vc_world_cambits(const c_vec2i_t& aWorld, const vc_assets_t& in_assets);
	uint32_t vc_screen_cambits(const c_vec2i_t& screen, const vc_assets_t& in_assets);

#if 0
	static void __draw_plax_foreground(
		const vc_assets_t& in_assets, const c_vec2i_t& in_camera,
		vc_fatpack_t& out_fatpack, micron_t& out_micron)
	{
		const int32_t SCREEN_X = in_camera.x / (MLM_M_TILE_ASPECT * NINJA_VC_TILES_ON_SCREEN_X);
		const int32_t SCREEN_Y = in_camera.y / (MLM_M_TILE_ASPECT * NINJA_VC_TILES_ON_SCREEN_Y);
		const uint32_t SCREEN_OFFSET = SCREEN_X + SCREEN_Y * NINJA_VC_SCREENS_IN_WORLD_X;
		assert(SCREEN_OFFSET < (NINJA_VC_SCREENS_IN_WORLD_X * NINJA_VC_SCREENS_IN_WORLD_Y));
		const uint32_t PLAX = in_assets.screens[SCREEN_OFFSET].plax;

		switch (PLAX % VC_PLAX_NUM_STYLES)
		{
		case VC_PLAX_ANOTHER_WORLD:
			for (
				uint32_t r = 0;
				r < 128;
				++r
				)
			{
				uint8_t* p = out_micron.canvas + out_fatpack.prng.int32(out_micron.canvas_width * (out_micron.canvas_height - 6));
				for (
					uint32_t d = 0;
					d < 6;
					++d
					)
				{
					*p = ANOTHER_WORLD_RAIN_INDEX % 256;
					p += out_micron.canvas_width;
				}
			}
			break;
		}
	}
#endif

	static void __safe_fat_pixel(
		const c_vec2f_t& in_world, const c_vec2i_t& in_camera, const uint8_t in_index,
		micron_t& out_micron)
	{
		const c_vec2i_t SP = vc_world_to_screen_int(in_world, in_camera);

		if (SP.x < 1)
			return;

		if (SP.y < 1)
			return;

		if (SP.x > out_micron.canvas_width - 2)
			return;

		if (SP.y > out_micron.canvas_height - 2)
			return;

		out_micron.canvas[SP.x + SP.y * out_micron.canvas_width] = in_index;
		out_micron.canvas[SP.x - 1 + SP.y * out_micron.canvas_width] = in_index;
		out_micron.canvas[SP.x + 1 + SP.y * out_micron.canvas_width] = in_index;
		out_micron.canvas[SP.x + (SP.y - 1) * out_micron.canvas_width] = in_index;
		out_micron.canvas[SP.x + (SP.y + 1) * out_micron.canvas_width] = in_index;
	}

	static void __safe_pixel(
		const c_vec2f_t& in_world, const c_vec2i_t& in_camera, const uint8_t in_index,
		micron_t& out_micron)
	{
		const c_vec2i_t SP = vc_world_to_screen_int(in_world, in_camera);

		if (SP.x < 0)
			return;

		if (SP.y < 0)
			return;

		if (SP.x > out_micron.canvas_width - 1)
			return;

		if (SP.y > out_micron.canvas_height - 1)
			return;

		out_micron.canvas[SP.x + SP.y * out_micron.canvas_width] = in_index;
	}

	void vc_play_input(
		const micron_t& in_micron, const m_immutable_t& in_im, const uint32_t in_tick,
		m_mutable_t& out_mu, vc_fatpack_t& out_fatpack, uint32_t& out_paused)
	{
#if 0
		if (ANOTHER_WORLD_RAIN_INDEX && w32_key_downflank(VK_UP))
			--ANOTHER_WORLD_RAIN_INDEX;

		if (ANOTHER_WORLD_RAIN_INDEX < 255 && w32_key_downflank(VK_DOWN))
			++ANOTHER_WORLD_RAIN_INDEX;
#endif

		if (out_paused)
		{
			//pause prompts
			{
				int32_t x;
				int32_t y;

				if (MLM_VC_GUI_LEFT & vc_gui_big_text(in_micron, x = in_micron.canvas_width / 8, y = 16, MICRON_KEY_ESCAPE, "Resume", out_fatpack))
				{
					out_paused ^= 1;
				}

				if (MLM_VC_GUI_LEFT & vc_gui_big_text(in_micron, x, y += 16, MICRON_KEY_RETURN, "Quit", out_fatpack))
				{
					out_paused ^= 1;
					m_restart(in_tick, m_mode_t::IDLE, in_im, out_mu);
				}
			}
		}
		else
		{
			//esc prompt
			{
				//if (sg_interaction_flags_left(vc_gui_big_text(cursor_position, 56, canvas8.height - VC_SOFT_Y_SPACE, VK_ESCAPE, "ESC Menu", mu)))
				//if (w32_key_upflank(VK_ESCAPE))
				if (micron_key_upflank(in_micron, MICRON_KEY_ESCAPE))
					out_paused ^= 1;
			}

			/*
			//stats
			{
				//flowers
				{
#if 0
					//legacy code was calculating this every tick (in 2 loops)
					//enable this to make sure that all the counting / caching is correct
					{
						uint32_t world_total_flowers = 0;
						uint32_t world_picked_flowers = 0;
						for (uint32_t i = 0; i < M_WORLD_SIZE; ++i)
						{
							if (logic_is(M_LOGIC_INDEX_FLOWER, m_mu.world_tiles[i].index, m_mu.logic))
								++world_total_flowers;

							if (logic_is(M_LOGIC_INDEX_FLOWER, m_mu.world_tiles[i].index, m_mu.logic) && m_mu.world_tiles[i].visited)
								++world_picked_flowers;
						}
						assert(mu.cache_total_flowers == world_total_flowers);
						assert(mu.cache_picked_flowers == world_picked_flowers);
					}
#endif

					::sprintf_s(mu.flower_count_string, "%u/%u", mu.cache_picked_flowers, mu.cache_total_flowers);
					vc_gui_big_text(cursor_position, 48, 16, -1, mu.flower_count_string, mu);
				}

				//elapsed
				mu.elapsed_string = c_string_milliseconds(m_model_elapsed_milliseconds(tick, m_mu));
				vc_gui_big_text(cursor_position, 40, 16 + VC_SOFT_BUTTON_HEIGHT, -1, mu.elapsed_string.buffer, mu);
			}
			*/

			vc_do_hero_movement(in_micron, in_im, out_mu);
		}
	}

	void vc_play_output(
		const m_immutable_t& in_im, const uint32_t in_paused, const uint32_t in_tick, const m_mutable_t& in_mu, const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack, micron_t& out_micron)
	{
		//make camera
		c_vec2i_t camera;
		{
			const c_vec2i_t HERO_WORLD = c_floor_to_int(in_mu.hero_position);

			//free scrolling all axes, hero centered on screen
			camera.x = HERO_WORLD.x - MLM_VC_SCREEN_WIDTH / 2;
			camera.y = HERO_WORLD.y - MLM_VC_SCREEN_HEIGHT / 2;

			const uint32_t cambits = vc_world_cambits(HERO_WORLD, in_assets);
			const c_vec2f_t hero_local = vc_screen_local(in_mu.hero_position);

			//constrain according to camera bits
			if (
				(1 & cambits) &&
				hero_local.y < MLM_VC_SCREEN_HEIGHT / 2
				)
			{
				camera.y = HERO_WORLD.y;
				camera.y /= MLM_VC_SCREEN_HEIGHT;
				camera.y *= MLM_VC_SCREEN_HEIGHT;
			}

			if (
				(2 & cambits) &&
				hero_local.y > MLM_VC_SCREEN_HEIGHT / 2
				)
			{
				camera.y = HERO_WORLD.y;
				camera.y /= MLM_VC_SCREEN_HEIGHT;
				camera.y *= MLM_VC_SCREEN_HEIGHT;
			}

			if (
				(4 & cambits) &&
				hero_local.x < MLM_VC_SCREEN_WIDTH / 2
				)
			{
				camera.x = HERO_WORLD.x;
				camera.x /= MLM_VC_SCREEN_WIDTH;
				camera.x *= MLM_VC_SCREEN_WIDTH;
			}

			if (
				(8 & cambits) &&
				hero_local.x > MLM_VC_SCREEN_WIDTH / 2
				)
			{
				camera.x = HERO_WORLD.x;
				camera.x /= MLM_VC_SCREEN_WIDTH;
				camera.x *= MLM_VC_SCREEN_WIDTH;
			}

			//extreme world bounds (both cases)
			if (camera.x < 0)
				camera.x = 0;
			else if (camera.x > (MLM_M_WORLD_WIDTH * MLM_M_TILE_ASPECT - MLM_VC_SCREEN_WIDTH))
				camera.x = MLM_M_WORLD_WIDTH * MLM_M_TILE_ASPECT - MLM_VC_SCREEN_WIDTH;

			if (camera.y < 0)
				camera.y = 0;
			else if (camera.y > (MLM_M_WORLD_HEIGHT * MLM_M_TILE_ASPECT - MLM_VC_SCREEN_HEIGHT))
				camera.y = MLM_M_WORLD_HEIGHT * MLM_M_TILE_ASPECT - MLM_VC_SCREEN_HEIGHT;
		}

		if (out_fatpack.bit_camera_lerp)
		{
			static c_vec2f_t LERP_CAMERA{ (float)camera.x, (float)camera.y };
			LERP_CAMERA.x += (camera.x - LERP_CAMERA.x) * M_SECONDS_PER_TICK * CAM_LERP_SPEED;
			LERP_CAMERA.y += (camera.y - LERP_CAMERA.y) * M_SECONDS_PER_TICK * CAM_LERP_SPEED;
			camera = c_floor_to_int(LERP_CAMERA);
		}

		vc_draw_plax(in_tick, in_im, in_mu, in_assets, camera, out_fatpack.prng, out_micron);

		//jumpers
		for (const m_mob_t& MOB : in_mu.mobs)
		{
			switch (MOB.type)
			{
			case M_LOGIC_JUMPER:
				vc_sprite_draw(vc_world_to_screen_int(MOB.position, camera), MOB.jumper.speed > 0.f, false, in_assets.jumper, out_micron);
				break;

			case M_LOGIC_JUMPER2:
				vc_sprite_draw(vc_world_to_screen_int(MOB.position, camera), MOB.jumper.speed > 0.f, false, in_assets.jumper2, out_micron);
				break;
			}
		}

		vc_tiles_draw(in_im, in_mu, in_assets, camera, NINJA_VC_TILEFLAGS_REPLACE | NINJA_VC_TILEFLAGS_NON_PASSAGE, out_fatpack, out_micron);

		//mobs after tiles
		for (const m_mob_t& MOB : in_mu.mobs)
		{
			switch (MOB.type)
			{
			case M_LOGIC_WALKER:
			{
				const c_vec2i_t SCREEN = vc_world_to_screen_int(MOB.position, camera);
				const int32_t FRAME_HEIGHT = 40;
				const int32_t FRAME = m_mob_sees_hero(in_im, in_mu, MOB) ? 0 : c_frame(in_tick * M_SECONDS_PER_TICK, HEAVY_FPS, 6);
				if (MOB.walker.speed > 0.f)
				{
					vc_octamap_blit_key_clip_flip_x(
						SCREEN.x - 16, SCREEN.y - 20,
						0, FRAME_HEIGHT,
						0, FRAME_HEIGHT * FRAME,
						in_assets.heavy, out_micron
					);
				}
				else
				{
					vc_octamap_blit_key_clip(
						SCREEN.x - 16, SCREEN.y - 20,
						0, FRAME_HEIGHT,
						0, FRAME_HEIGHT * FRAME,
						in_assets.heavy, out_micron
					);
				}
			}
			break;

			case M_LOGIC_WALKER2:
				vc_sprite_draw(vc_world_to_screen_int(MOB.position, camera), (uint8_t)c_frame(in_tick * M_SECONDS_PER_TICK, 12.f, 2), MOB.walker.speed > 0.f, in_assets.walker2, out_micron);
				break;

			case M_LOGIC_SWIMMER:
				vc_sprite_draw(vc_world_to_screen_int(MOB.position, camera), (uint8_t)c_frame(in_tick * M_SECONDS_PER_TICK, 8.f, 2), MOB.swimmer.speed.x > 0.f, in_assets.swimmer, out_micron);
				break;

			case M_LOGIC_SWIMMER2:
				vc_sprite_draw(vc_world_to_screen_int(MOB.position, camera), (uint8_t)c_frame(in_tick * M_SECONDS_PER_TICK, 8.f, 2), MOB.swimmer.speed.x > 0.f, in_assets.swimmer2, out_micron);
				break;

			case M_LOGIC_BULLET:
			{
				const c_vec2i_t SCREEN = vc_world_to_screen_int(MOB.position, camera);
				if (out_fatpack.prng.boolean())
				{
					vc_canvas_fill_circle(SCREEN.x, SCREEN.y, 3, 1, out_micron);
					vc_canvas_fill_circle(SCREEN.x, SCREEN.y, 1, 0, out_micron);
				}
				else
				{
					vc_canvas_fill_circle(SCREEN.x, SCREEN.y, 5, 1, out_micron);
					vc_canvas_fill_circle(SCREEN.x, SCREEN.y, 3, 0, out_micron);
				}
			}
			break;
			}
		}

		vc_draw_hero(in_tick, in_im, in_mu, camera, in_assets, out_fatpack, out_micron);

#if 0
		vc_draw_terminals(im, camera, m_world_to_offset(m_mu.hero_position), canvas8);
#endif

		vc_fx_draw_all(in_paused, in_tick, in_im, camera, in_assets, out_fatpack, out_micron);

#if 0
		__draw_plax_foreground(in_assets, camera, out_fatpack, out_micron);
#endif

		//cambits
		if (out_fatpack.bit_display_cambits)
		{
			constexpr uint8_t ON = 1;
			constexpr uint8_t OFF = 2;

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
					const uint32_t SCREEN_OFFSET = (x / NINJA_VC_TILES_ON_SCREEN_X) + (y / NINJA_VC_TILES_ON_SCREEN_Y) * NINJA_VC_SCREENS_IN_WORLD_X;
					if (SCREEN_OFFSET < (NINJA_VC_SCREENS_IN_WORLD_X * NINJA_VC_SCREENS_IN_WORLD_Y))
					{
						const uint32_t CAMBITS = vc_screen_cambits({ x / NINJA_VC_TILES_ON_SCREEN_X, y / NINJA_VC_TILES_ON_SCREEN_Y }, in_assets);

						const bool LEFT = 0 == x % NINJA_VC_TILES_ON_SCREEN_X;
						const bool RIGHT = (NINJA_VC_TILES_ON_SCREEN_X - 1) == x % NINJA_VC_TILES_ON_SCREEN_X;
						const bool HORIZONTAL = LEFT || RIGHT;
						const bool TOP = 0 == y % NINJA_VC_TILES_ON_SCREEN_Y;
						const bool BOTTOM = (NINJA_VC_TILES_ON_SCREEN_Y - 1) == y % NINJA_VC_TILES_ON_SCREEN_Y;
						const bool VERTICAL = TOP || BOTTOM;

						if (
							TOP &&
							!HORIZONTAL
							)
						{
							vc_canvas_clear(
								(1 & CAMBITS) ? ON : OFF,
								(x * MLM_M_TILE_ASPECT) - camera.x + 4,
								(y * MLM_M_TILE_ASPECT) - camera.y + 4,
								MLM_M_TILE_ASPECT - 8, MLM_M_TILE_ASPECT - 8,
								out_micron
							);
						}

						if (
							BOTTOM &&
							!HORIZONTAL
							)
						{
							vc_canvas_clear(
								(2 & CAMBITS) ? ON : OFF,
								(x * MLM_M_TILE_ASPECT) - camera.x + 4,
								(y * MLM_M_TILE_ASPECT) - camera.y + 4,
								MLM_M_TILE_ASPECT - 8, MLM_M_TILE_ASPECT - 8,
								out_micron
							);
						}

						if (
							LEFT &&
							!VERTICAL
							)
						{
							vc_canvas_clear(
								(4 & CAMBITS) ? ON : OFF,
								(x * MLM_M_TILE_ASPECT) - camera.x + 4,
								(y * MLM_M_TILE_ASPECT) - camera.y + 4,
								MLM_M_TILE_ASPECT - 8, MLM_M_TILE_ASPECT - 8,
								out_micron
							);
						}

						if (
							RIGHT &&
							!VERTICAL
							)
						{
							vc_canvas_clear(
								(8 & CAMBITS) ? ON : OFF,
								(x * MLM_M_TILE_ASPECT) - camera.x + 4,
								(y * MLM_M_TILE_ASPECT) - camera.y + 4,
								MLM_M_TILE_ASPECT - 8, MLM_M_TILE_ASPECT - 8,
								out_micron
							);
						}
					}
				}
			}
		}

		if (out_fatpack.bit_display_visited)
		{
			const uint8_t GREEN = 5;
			const uint8_t BLUE = 3;

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
					switch (m_visited(x, y, in_mu))
					{
					case MLM_M_VISITED_CHECKPOINT:
						vc_canvas_clear(
							GREEN,
							(x * MLM_M_TILE_ASPECT) - camera.x + 4,
							(y * MLM_M_TILE_ASPECT) - camera.y + 4,
							MLM_M_TILE_ASPECT - 8, MLM_M_TILE_ASPECT - 8,
							out_micron
						);
						break;

					case MLM_M_VISITED:
						vc_canvas_clear(
							BLUE,
							(x * MLM_M_TILE_ASPECT) - camera.x + 4,
							(y * MLM_M_TILE_ASPECT) - camera.y + 4,
							MLM_M_TILE_ASPECT - 8, MLM_M_TILE_ASPECT - 8,
							out_micron
						);
						break;
					}
				}
			}
		}

		if (out_fatpack.bit_display_hero_tests)
		{
			const uint8_t INDEX = (uint8_t)c_frame(in_tick * M_SECONDS_PER_TICK, 8.f, 8);

			{
				//c_vec2i_t sp;

				//pos
				__safe_fat_pixel(in_mu.hero_position, camera, INDEX, out_micron);

				//below
				__safe_pixel(m_character_test_below(false, in_mu), camera, INDEX, out_micron);
				__safe_pixel(m_character_test_below(true, in_mu), camera, INDEX, out_micron);

				//above
				__safe_pixel(m_character_test_above(in_mu), camera, INDEX, out_micron);

				//left
				__safe_pixel(m_character_get_test_left(false, in_mu), camera, INDEX, out_micron);
				__safe_pixel(m_character_get_test_left(true, in_mu), camera, INDEX, out_micron);

				//right
				__safe_pixel(m_character_get_test_right(false, in_mu), camera, INDEX, out_micron);
				__safe_pixel(m_character_get_test_right(true, in_mu), camera, INDEX, out_micron);
			}

			for (const m_mob_t& MOB : in_mu.mobs)
				__safe_fat_pixel(MOB.position, camera, INDEX, out_micron);
		}

		//play input key legend
		vc_canvas_atascii_print(0, out_micron.canvas_height - 8, 0, "ADK", out_micron);

		out_micron.music = ASSET_AMBIENCE;
	}
}
