#include "stdafx.h"
#include "vc_work.h"

#include "../microlib/c_math.h"
#include "../microlib/fs.h"
#include "../microlib/microlib.h"
#include "../microlib/p_context.h"
#include "../micron.h"
#include "m_mutable.h"
#include "m_work.h"
#include "vc_assets.h"
#include "vc_fatpack.h"
#include "vc_octamap.h"

#define C_ROUND(x) ((int32_t)(x + .5f))

#define WANG_2_CORNER_SYSTEMS 1

#define FX_GRAVITY 1300.f

#define BLADE_LOCAL_GRAVITY 100.f
#define BLADE_FALL_SPEED 20.f

#define ANIMATED_FRAME_WIDTH 16
#define ANIMATED_FPS 16.f

#define WANG_BLOB_NORTH 1
#define WANG_BLOB_NORTHEAST 2
#define WANG_BLOB_EAST 4
#define WANG_BLOB_SOUTHEAST 8
#define WANG_BLOB_SOUTH 16
#define WANG_BLOB_SOUTHWEST 32
#define WANG_BLOB_WEST 64
#define WANG_BLOB_NORTHWEST 128

#define DRAW_PASSAGE(flags) ((flags & NINJA_VC_TILEFLAGS_PASSAGE) && M_TYPE_PASSAGE == TYPE)
#define DRAW_NON_PASSAGE(flags) ((flags & NINJA_VC_TILEFLAGS_NON_PASSAGE) && M_TYPE_PASSAGE != TYPE)

#define ASSET_TILES "tiles.tga"
//#define ASSET_BIGFONT "bigfont8.tga"
//#define ASSET_CURSOR "cursor.tga"
#define ASSET_BLADES "blades.tga"
#define ASSET_GIBS "gibs.tga"
#define ASSET_IMPALEMENT "die_impaled.tga"
#define ASSET_DROWNING "die_drowned.tga"
#define ASSET_BURNING "die_burned.tga"
#define ASSET_ELECTROCUTION "die_electrocuted.tga"
#define ASSET_SPECIALICONS "specialicons.tga"
//#define ASSET_FISH "fish.tga"

//#define ASSET_HERO "PlaceholderCharacterTest.tga"
#define ASSET_HERO "chad.tga"

#define ASSET_HEAVY "heavy.tga"
#define ASSET_WALKER "walker.tga"
#define ASSET_WALKER2 "walker2.tga"
#define ASSET_JUMPER "jumper.tga"
#define ASSET_JUMPER2 "jumper2.tga"
#define ASSET_SWIMMER "swimmer.tga"
#define ASSET_SWIMMER2 "swimmer2.tga"
#define ASSET_CONTEXT_VIEW "view.txt"
#define ASSET_CONTEXT_FARPLANES "farplanes.txt"

#define GFX_KIND "sabk::vc::Gfx"
#define GFX_KEY 1

#define TILEINFO_KIND "sabk::vc::TileInfo"

#define SCREEN_KIND "sabk::vc::Screen"

#define TERMINAL_KIND "sabk::vc::Terminal"

#define SNAP_IMPALEMENT true
#define DROWNED_FX_OFFSET_Y -16.f
#define NP_COLOR 0
#define NP_GRAVITY 48.f

namespace mlm
{
	bool vc_gfx_is(const uint32_t aValue, const uint16_t tile, const vc_assets_t& in_assets);

#if 0
	static constexpr char* ASSET_TEMP_FARPLANE[] =
	{
		"plax/snowymountains00.tga",
		"plax/snowymountains01.tga",
		"plax/snowymountains02.tga",
		"plax/snowymountains03.tga",
		"plax/snowymountains04.tga",
		"plax/snowymountains05.tga",

		"plax/purplecaves00.tga",
		"plax/purplecaves01.tga",
		"plax/purplecaves02.tga",
		"plax/purplecaves03.tga",
		"plax/purplecaves04.tga",

		"plax/undersea00.tga",
		"plax/undersea01.tga",
		"plax/undersea02.tga",

		"plax/woody00.tga",
		"plax/woody01.tga",
		"plax/woody02.tga",
		"plax/woody03.tga",
		"plax/woody04.tga",
		"plax/woody05.tga",
		"plax/woody06.tga",

		"plax/monolith00.tga",
		"plax/monolith01.tga",
		"plax/monolith02.tga",
		"plax/monolith03.tga",

		"plax/vampire00.tga",
		"plax/vampire01.tga",

		"plax/city00.tga",
		"plax/city01.tga",
		"plax/city02.tga",
		"plax/city03.tga",
		"plax/city04.tga",

		"plax/mumin00.tga",
		"plax/mumin01.tga",
		"plax/mumin02.tga",
		"plax/mumin03.tga",
		"plax/mumin04.tga",
		"plax/mumin05.tga",

		"plax/mirkwood00.tga",
		"plax/mirkwood01.tga",
		"plax/mirkwood02.tga",
		"plax/mirkwood03.tga",
		"plax/mirkwood04.tga",
		"plax/mirkwood05.tga",
		"plax/mirkwood06.tga",

		"plax/industrial00.tga",
		"plax/industrial01.tga",
		"plax/industrial02.tga",
		"plax/industrial03.tga",

		"plax/anotherworld00.tga",
		"plax/anotherworld01.tga",

		"plax/alien00.tga",
		"plax/alien01.tga",
		"plax/alien02.tga",
	};
#endif

	static constexpr char* ASSET_SOUND[VC_NUM_SOUNDS] =
	{
		"snd_step.wav",
		"snd_jump.wav",
		"snd_land.wav",
		"snd_slide.wav",
		"snd_die_impaled.wav",
	};

	double vc_tiles_draw_time = 0.;
	uint32_t vc_tiles_draw_count = 0;

#if WANG_2_CORNER_SYSTEMS
	static constexpr int32_t WANG_2_CORNER_OFFSETS[16] =
	{
		0,
		3,
		-24,
		-23,
		-72,
		-21,
		2,
		-45,
		1,
		-48,
		-71,
		-22,
		-69,
		-70,
		-47,
		-46,
	};

	struct wang_2_corner_system_t
	{
		struct
		{
			int32_t index;	//tile index that is actually in the m_mu data, identifies the "system"
			uint32_t flags;	//regular corner flags for each system (set if that corner is the same as index)
		} systems[4];
		uint32_t bits;	//appropriate bit (1 - 4) set if we should draw the system in that corner
	};

	static wang_2_corner_system_t __wang_2_corner_system(const int32_t in_x, const int32_t in_y, const m_immutable_t& in_im, const vc_assets_t& in_assets)
	{
		wang_2_corner_system_t result{};

		//NOTE: only need one bit (corner) for each unique system in 4 neighbouring tiles
		//otherwise we WILL do it 4x if all the corners in 4 neighbouring tiles are the same

		result.systems[0].index = m_tile(in_x, in_y, in_im);
		if (WANG_2_CORNER == in_assets.tile_info[result.systems[0].index].wang)
			result.bits |= 1;

		result.systems[1].index = m_tile(in_x + 1, in_y, in_im);
		if (
			WANG_2_CORNER == in_assets.tile_info[result.systems[1].index].wang &&
			result.systems[1].index != result.systems[0].index
			)
			result.bits |= 2;

		result.systems[2].index = m_tile(in_x, in_y + 1, in_im);
		if (
			WANG_2_CORNER == in_assets.tile_info[result.systems[2].index].wang &&
			result.systems[2].index != result.systems[0].index &&
			result.systems[2].index != result.systems[1].index
			)
			result.bits |= 4;

		result.systems[3].index = m_tile(in_x + 1, in_y + 1, in_im);
		if (
			WANG_2_CORNER == in_assets.tile_info[result.systems[3].index].wang &&
			result.systems[3].index != result.systems[0].index &&
			result.systems[3].index != result.systems[1].index &&
			result.systems[3].index != result.systems[2].index
			)
			result.bits |= 8;

		for (
			uint32_t i = 0;
			i < 4;
			++i
			)
		{
			for (
				uint32_t j = 0;
				j < 4;
				++j
				)
			{
				if (result.systems[i].index == result.systems[j].index)
					result.systems[i].flags |= (1 << j);
			}
		}

		return result;
	}
#endif

	static bool __collide(const m_immutable_t& in_im, const m_mutable_t& in_mu, const c_vec2f_t& in_world)
	{
		switch (m_tile_type_at_position(in_world, true, in_im, in_mu))
		{
		default:
			return false;

		case M_TYPE_SOLID:
		case M_TYPE_PLATFORM:
			return true;
		}
	}

	static void __noise_particles(
		const uint32_t in_tick, const bool in_collide, const m_immutable_t& in_im, const m_mutable_t& in_mu, const c_vec2f_t& in_camera,
		c_xorshift128_t& out_prng, micron_t& out_micron)
	{
		struct speck_t
		{
			c_vec2f_t position;
			c_vec2f_t speed;
		};
		enum
		{
			NUM_POINTS = 1 << 10,
		};
		static speck_t specks[NUM_POINTS];
		const speck_t* END = specks + NUM_POINTS;
		c_vec2f_t a;
		float n;

		//init
		{
			static bool f = true;
			if (f)
			{
				for (
					speck_t* speck = specks;
					speck < END;
					++speck
					)
				{
					speck->position = { out_prng.float32(0.f, (float)out_micron.canvas_width), out_prng.float32(0.f, (float)out_micron.canvas_height) };
					speck->speed = {};
				}
				f = false;
			}
		}

		c_vec2f_t factor{};
		factor.x = .001f * factor.x + .01f * (1.f - factor.x);
		factor.y = .001f * factor.y + .01f * (1.f - factor.y);

		const float Z_FACTOR = in_tick * M_SECONDS_PER_TICK * .1f;
		const float ACC = 64.f;
		const float FRICTION = .75f;
		const float MIN_REFLECT = -.8f;
		const float MAX_REFLECT = -1.2f;

		for (
			speck_t* speck = specks;
			speck < END;
			++speck
			)
		{
			a.x = ACC;
			a.y = 0.f;
			n = (float)c_perlin_simplex_noise3(speck->position.x * factor.x, speck->position.y * factor.y, Z_FACTOR);
			n *= .5f;
			n += .5f;
			n *= C_PI2 * 2.f;
			a = c_rotate(n, a);

			speck->speed.y += NP_GRAVITY * M_SECONDS_PER_TICK;
			speck->speed += a * M_SECONDS_PER_TICK;
			speck->speed -= speck->speed * (FRICTION * M_SECONDS_PER_TICK);
			const c_vec2f_t pre_move = speck->position;
			speck->position += speck->speed * M_SECONDS_PER_TICK;

			if (in_collide)
			{
				//solids
				if (__collide(in_im, in_mu, in_camera + speck->position))
				{
					speck->position = pre_move;

					if (::fabsf(speck->speed.x) > ::fabsf(speck->speed.y))
						speck->speed.x *= out_prng.float32(MIN_REFLECT, MAX_REFLECT);
					else
						speck->speed.y *= out_prng.float32(MIN_REFLECT, MAX_REFLECT);

					if (__collide(in_im, in_mu, in_camera + speck->position))
					{
						speck->position = { out_prng.float32(0.f, (float)out_micron.canvas_width), out_prng.float32(0.f, (float)out_micron.canvas_height) };
						speck->speed = {};
					}
				}

				//screen bounds
				if (speck->position.x < 0.f)
				{
					speck->position.x = 0.f;
					speck->speed.x *= out_prng.float32(MIN_REFLECT, MAX_REFLECT);
				}
				else if (speck->position.x >= out_micron.canvas_width)
				{
					speck->position.x = out_micron.canvas_width - 1.f;
					speck->speed.x *= out_prng.float32(MIN_REFLECT, MAX_REFLECT);
				}

				if (speck->position.y < 0.f)
				{
					speck->position.y = 0.f;
					speck->speed.y *= out_prng.float32(MIN_REFLECT, MAX_REFLECT);
				}
				else if (speck->position.y >= out_micron.canvas_height)
				{
					speck->position.y = out_micron.canvas_height - 1.f;
					speck->speed.y *= out_prng.float32(MIN_REFLECT, MAX_REFLECT);
				}
			}
			else
			{
				if (
					speck->position.x < 0.f ||
					speck->position.x >= out_micron.canvas_width ||
					speck->position.y < 0.f ||
					speck->position.y >= out_micron.canvas_height
					)
				{
					speck->position = { out_prng.float32(0.f, (float)out_micron.canvas_width), 0.f };
					//speck->speed = {};
				}
			}
		}

		for (
			const speck_t* speck = specks;
			speck < END;
			++speck
			)
		{
			const int32_t PIXEL = (int32_t)speck->position.x + (int32_t)speck->position.y * out_micron.canvas_width;
			assert(PIXEL >= 0 && PIXEL < out_micron.canvas_width * out_micron.canvas_height);
#if OCTAMAP_DEPTH_COMPLEXITY
			if (out_micron.canvas[pixel] < 255)
				++out_micron.canvas[pixel];
#else
			out_micron.canvas[PIXEL] = NP_COLOR % 256;
#endif
		}
	}

#if 0
	static void __blades(
		const uint32_t in_tick, const c_vec2f_t& in_position, const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack)
	{
		const int32_t COUNT = out_fatpack.prng.int32(3) + 2;
		c_vec2f_t vec;
		vc_fx_t* fx;

		for (
			int32_t i = 0;
			i < COUNT;
			++i
			)
		{
			vec = c_rotate((.5f * C_PI / COUNT) + i * C_PI / COUNT, { 64, 0.f });

			fx = out_fatpack.fx_acquire();
			fx->type = VC_FX_BLADE;
			fx->position = in_position;

			fx->blade.time_to_die = in_tick * M_SECONDS_PER_TICK + out_fatpack.prng.float32(1.f, 2.f);
			fx->blade.speed = vec;
			fx->blade.frame = (uint8_t)out_fatpack.prng.int32(FS_TGA_HEADER(in_assets.blades)->image_spec_height / FS_TGA_HEADER(in_assets.blades)->image_spec_width);
			fx->blade.slowing = true;
		}
	}
#endif

	static void __puff(
		const uint32_t in_count, const float in_speed, const m_immutable_t& in_im, const uint32_t in_tick, const bool in_flowers, const c_vec2f_t& in_position,
		vc_fatpack_t& out_fatpack)
	{
		c_vec2f_t vec;
		vc_fx_t* fx;
		uint8_t shade;

		for (
			uint32_t i = 0;
			i < in_count;
			++i
			)
		{
			if (in_flowers)
			{
				vec = c_rotate(i * C_PI2 / in_count, { 0.f, 128.f });
				vec.y -= 256.f;

				fx = out_fatpack.fx_acquire();
				fx->type = VC_FX_TILE;

				fx->tile.time_to_die = in_tick * M_SECONDS_PER_TICK + out_fatpack.prng.float32(.3f, 1.f);
				fx->position = in_position;
				fx->tile.speed = vec;
				fx->tile.tile = m_logic_index_of(M_LOGIC_FLOWER, in_im);
			}

			vec = c_rotate(i * C_PI2 / in_count, { 0.f, in_speed });
			shade = (uint8_t)(out_fatpack.prng.int32(32) + 16);

			fx = out_fatpack.fx_acquire();
			fx->type = VC_FX_SMOKE;
			fx->position = in_position;

			fx->smoke.time_to_live = out_fatpack.prng.float32(1.f, 3.f);
			fx->smoke.time_to_die = in_tick * M_SECONDS_PER_TICK + fx->smoke.time_to_live;
			fx->smoke.size = 10.f;
#if 0
			fx->smoke.color1 = in_assets.palettize_color((uint8_t)(shade / 4 * 3), (uint8_t)(shade / 4 * 3), (uint8_t)(shade / 4 * 3));
			fx->smoke.color2 = in_assets.palettize_color(shade, shade, shade);
#else
			fx->smoke.color1 = 10;
			fx->smoke.color2 = 9;
#endif
			fx->smoke.speed = vec;
		}
	}

#if 0
	static void __gibs(
		const uint32_t in_tick, const c_vec2f_t& in_position, const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack)
	{
		const uint32_t COUNT = FS_TGA_HEADER(in_assets.gibs)->image_spec_height / FS_TGA_HEADER(in_assets.gibs)->image_spec_width;
		c_vec2f_t vec;
		vc_fx_t* fx;

		for (
			uint32_t i = 0;
			i < COUNT;
			++i
			)
		{
			vec = c_rotate(i * C_PI2 / COUNT, { 0.f, 64.f });
			vec.y -= 384.f;

			fx = out_fatpack.fx_acquire();
			fx->type = VC_FX_GIB;
			fx->position = in_position;

			fx->gib.frame = (uint8_t)i;
			fx->gib.time_to_die = in_tick * M_SECONDS_PER_TICK + 1.f;
			fx->gib.speed = vec;
		}
	}
#endif

#if 0
	static void __octafont_print(
		const c_blob_t& in_font, const int32_t in_dst_x, const int32_t in_dst_y, const char* in_message,
		micron_t& out_micron)
	{
		if (!in_message)
			return;

		const int32_t LENGTH = (int32_t)::strlen(in_message);
		if (!LENGTH)
			return;

		int32_t x = in_dst_x;
		int32_t y = in_dst_y;

		//draw characters
		for (
			int32_t i = 0;
			i < LENGTH;
			++i
			)
		{
			char ch = in_message[i];

			//space
			if (' ' == ch)
			{
				x += NINJA_VC_OCTAFONT_SPACE_WIDTH + NINJA_VC_OCTAFONT_CHAR_SPACING;
			}
			//end line
			else if ('\n' == ch)
			{
				x = in_dst_x;
				y += NINJA_VC_OCTAFONT_HEIGHT;
			}
			//printable chars
			else if (
				ch >= NINJA_VC_OCTAFONT_CHARBEGIN &&
				ch <= NINJA_VC_OCTAFONT_CHAREND
				)
			{
				ch -= NINJA_VC_OCTAFONT_CHARBEGIN;

				vc_octamap_blit_key_clip(
					x, y,
					VC_FONT_TABLES[ch].w, NINJA_VC_OCTAFONT_HEIGHT,
					VC_FONT_TABLES[ch].s, VC_FONT_TABLES[ch].t,
					in_font, out_micron
				);

				x += VC_FONT_TABLES[ch].w + NINJA_VC_OCTAFONT_CHAR_SPACING;
			}
		}
	}
#endif

	static bool __fx_draw(
		const uint32_t in_paused, const uint32_t in_tick, const c_vec2i_t& in_camera, const vc_assets_t& in_assets, const int32_t in_tiles_on_source_x,
		vc_fx_t& out_instance, vc_fatpack_t& out_fatpack, micron_t& out_micron)
	{
		switch (out_instance.type)
		{
		default:
			assert(0);
			return false;

		case VC_FX_TILE:
			if (!in_paused)
			{
				if (out_instance.tile.time_to_die < in_tick * M_SECONDS_PER_TICK)
					return false;

				//move
				out_instance.tile.speed.y += FX_GRAVITY * M_SECONDS_PER_TICK;
				out_instance.position += out_instance.tile.speed * M_SECONDS_PER_TICK;
			}

			vc_octamap_blit_key_clip(
				C_ROUND(out_instance.position.x - in_camera.x - MLM_M_TILE_ASPECT / 2), C_ROUND(out_instance.position.y - in_camera.y - MLM_M_TILE_ASPECT / 2),
				MLM_M_TILE_ASPECT, MLM_M_TILE_ASPECT,
				(out_instance.tile.tile % in_tiles_on_source_x) * MLM_M_TILE_ASPECT,
				(out_instance.tile.tile / in_tiles_on_source_x) * MLM_M_TILE_ASPECT,
				in_assets.tiles, out_micron);

			return true;

		case VC_FX_SMOKE:
			if (!in_paused)
			{
				if (out_instance.smoke.time_to_die < in_tick * M_SECONDS_PER_TICK)
					return false;

				//brake
				out_instance.smoke.speed -= out_instance.smoke.speed * (M_SECONDS_PER_TICK * 2.f);

				//move
				out_instance.position += out_instance.smoke.speed * M_SECONDS_PER_TICK;
			}

			{
				const int32_t SIZE = (int32_t)(out_instance.smoke.size * (out_instance.smoke.time_to_die - in_tick * M_SECONDS_PER_TICK) / out_instance.smoke.time_to_live);
				const c_vec2i_t SP = vc_world_to_screen_int(out_instance.position, in_camera);
				vc_canvas_fill_circle(SP.x, SP.y, SIZE, out_instance.smoke.color1, out_micron);
				vc_canvas_fill_circle(SP.x - SIZE / 4, SP.y - SIZE / 4, SIZE / 2, out_instance.smoke.color2, out_micron);
			}

			return true;

#if 0
		case VC_FX_BLADE:
			if (!in_paused)
			{
				if (out_instance.blade.time_to_die < in_tick * M_SECONDS_PER_TICK)
					return false;

				if (out_instance.blade.slowing)
				{
					out_instance.blade.speed -= out_instance.blade.speed * (M_SECONDS_PER_TICK * 2.f);
					if (c_length(out_instance.blade.speed) < BLADE_FALL_SPEED)
						out_instance.blade.slowing = false;
				}
				else
				{
					out_instance.blade.speed.y += BLADE_LOCAL_GRAVITY * M_SECONDS_PER_TICK;
				}

				out_instance.position += out_instance.blade.speed * M_SECONDS_PER_TICK;
			}

			{
				const c_vec2i_t DST = vc_world_to_screen_int(out_instance.position, in_camera);
				vc_octamap_blit_key_clip(
					DST.x, DST.y,
					FS_TGA_HEADER(in_assets.blades)->image_spec_width, FS_TGA_HEADER(in_assets.blades)->image_spec_width,
					0, (int32_t)out_instance.blade.frame * FS_TGA_HEADER(in_assets.blades)->image_spec_width,
					in_assets.blades, out_micron);
			}

			return true;
#endif

#if 0
		case VC_FX_GIB:
			if (!in_paused)
			{
				if (out_instance.gib.time_to_die < in_tick * M_SECONDS_PER_TICK)
					return false;

				//move
				out_instance.gib.speed.y += FX_GRAVITY * M_SECONDS_PER_TICK;
				out_instance.position += out_instance.gib.speed * M_SECONDS_PER_TICK;
			}

			{
				const c_vec2i_t DST = vc_world_to_screen_int(out_instance.position, in_camera);
				vc_octamap_blit_key_clip(
					DST.x, DST.y,
					FS_TGA_HEADER(in_assets.gibs)->image_spec_width, FS_TGA_HEADER(in_assets.gibs)->image_spec_width,
					0, out_instance.gib.frame * FS_TGA_HEADER(in_assets.gibs)->image_spec_width,
					in_assets.gibs, out_micron);
			}

			return true;
#endif

		case VC_FX_ANIMATED:
		{
			const uint32_t NUM_FRAMES = FS_TGA_HEADER(*out_instance.animated.bitmap)->image_spec_width / ANIMATED_FRAME_WIDTH;
			const uint32_t FRAME = c_frame(in_tick * M_SECONDS_PER_TICK - out_instance.animated.start_time, ANIMATED_FPS, UINT32_MAX);

			if (!in_paused)
			{
				if (FRAME >= NUM_FRAMES)
				{
					if (out_instance.animated.ends_with_gibs)
					{
#if 0
						__puff(16, 64.f, in_im, in_tick, in_assets, false, out_instance.position, out_fatpack);
						__gibs(in_tick, out_instance.position, in_assets, out_fatpack);
#else
						out_fatpack.deferred_fx.push_back(out_instance);
#endif
					}

					return false;
				}
			}

			assert(FRAME < NUM_FRAMES);

			const c_vec2i_t SP
			{
				(int32_t)(out_instance.position.x - in_camera.x - ANIMATED_FRAME_WIDTH * .5f),
				(int32_t)(out_instance.position.y - in_camera.y - FS_TGA_HEADER(*out_instance.animated.bitmap)->image_spec_height * .5f)
			};
			if (out_instance.animated.mirror)
			{
				vc_octamap_blit_key_clip_flip_x(
					SP.x, SP.y,
					ANIMATED_FRAME_WIDTH, FS_TGA_HEADER(*out_instance.animated.bitmap)->image_spec_height,
					FRAME * ANIMATED_FRAME_WIDTH, 0,
					*out_instance.animated.bitmap, out_micron);
			}
			else
			{
				vc_octamap_blit_key_clip(
					SP.x, SP.y,
					(int32_t)ANIMATED_FRAME_WIDTH, FS_TGA_HEADER(*out_instance.animated.bitmap)->image_spec_height,
					(int32_t)(FRAME * ANIMATED_FRAME_WIDTH), 0,
					*out_instance.animated.bitmap, out_micron);
			}

			return true;
		}
		}//switch (out_instance.type)
	}

#if 0
	//FIXME: probably wrong when scrolling...
	static bool __on_hero_screen(const c_vec2f_t& in_world, const m_mutable_t& in_mu)
	{
		const c_vec2i_t SCREEN
		{
			(int32_t)in_world.x / (MLM_M_TILE_ASPECT * NINJA_VC_TILES_ON_SCREEN_X),
			(int32_t)in_world.y / (MLM_M_TILE_ASPECT * NINJA_VC_TILES_ON_SCREEN_Y)
		};
		return SCREEN == vc_hero_screen(in_mu);
	}
#endif

#if 0
	static bool __hero_near_ground(const m_immutable_t& in_im, const m_mutable_t& in_mu)
	{
		switch (m_tile_type_at_position({ in_mu.hero_position.x, in_mu.hero_position.y + MLM_M_CHAR_HALF_HEIGHT + MLM_M_TILE_ASPECT * 2.f }, true, in_im, in_mu))
		{
		case M_TYPE_SOLID:
		case M_TYPE_PLATFORM:
			return true;

		default:
			return false;
		}
	}
#endif

#if 0
	static void __new_animated_fx_t(
		const uint32_t in_tick, const bool in_end_width_gibs, const c_vec2f_t& in_position, const c_blob_t& in_bitmap, const m_mutable_t& in_mu,
		vc_fatpack_t& out_fatpack)
	{
		vc_fx_t* i = out_fatpack.fx_acquire();
		i->type = VC_FX_ANIMATED;
		i->position = in_position;

		i->animated.start_time = in_tick * M_SECONDS_PER_TICK;
		i->animated.mirror = in_mu.hero_mirror;
		i->animated.ends_with_gibs = in_end_width_gibs;
		i->animated.bitmap = &in_bitmap;
	}
#endif

#if 0
	static int32_t __string_width(const char* in_string)
	{
		const int32_t LENGTH = (int32_t)::strlen(in_string);
		int32_t result = 0;
		int32_t index = 0;

		while (index < LENGTH)
		{
			const char CHAR = in_string[index];
			if (CHAR == '\t')
			{
				result += NINJA_VC_OCTAFONT_SPACE_WIDTH;
			}
			else if (CHAR == '\n')
			{
				return result;
			}
			else
			{
				assert(CHAR >= 32 && CHAR <= 128);
				if (CHAR == ' ')
					result += NINJA_VC_OCTAFONT_SPACE_WIDTH;
				else
					result += VC_FONT_TABLES[CHAR - NINJA_VC_OCTAFONT_CHARBEGIN].w + NINJA_VC_OCTAFONT_CHAR_SPACING;
			}
			++index;
		}

		return result;
	}
#endif

	static const vc_text_t* __add_text(
		const int32_t in_x, const int32_t in_y, const int32_t in_key, const char* in_text,
		vc_fatpack_t& out_fatpack)
	{
		assert(out_fatpack.gui_num_text < _countof(out_fatpack.gui_text));
		if (out_fatpack.gui_num_text < _countof(out_fatpack.gui_text))
		{
			vc_text_t* text = &out_fatpack.gui_text[out_fatpack.gui_num_text++];
			assert(in_text && *in_text);
			if (in_key > -1)
				text->text.format("<%s>%s", w32_vk_name(in_key), in_text);
			else
				text->text = in_text;
			text->x = in_x;
			text->y = in_y;
			return text;
		}

		return nullptr;
	}

	/*
	static bool __cursor_inside(const micron_t& in_micron, const c_vec2i_t& in_position, const c_vec2i_t& in_size)
	{
		return
			in_micron.canvas_cursor_x >= in_position.x &&
			in_micron.canvas_cursor_x < in_position.x + in_size.x &&
			in_micron.canvas_cursor_y >= in_position.y &&
			in_micron.canvas_cursor_y < in_position.y + in_size.y;
	}

	uint32_t __gui_invisible_button(const micron_t& in_micron, const int32_t in_x, const int32_t in_y, const int32_t in_width, const int32_t in_height, const int32_t in_key)
	{
		if (micron_key_upflank(in_micron, in_key))
			return MLM_VC_GUI_LEFT;

		if (__cursor_inside(in_micron, { in_x, in_y }, { in_width, in_height }))
		{
			uint32_t f = 0;

			f |= MLM_VC_GUI_INSIDE;

			if (micron_key_upflank(in_micron, MICRON_KEY_LMB))
			{
				f |= MLM_VC_GUI_LEFT;
			}
			else if (micron_key_upflank(in_micron, MICRON_KEY_RMB))
			{
				f |= MLM_VC_GUI_RIGHT;
			}

			return f;
		}

		return 0;
	}
	*/

	static bool __all_bits(const uint32_t in_bits, const uint32_t in_to_test)
	{
		return in_bits == (in_bits & in_to_test);
	}

	static void __draw_tile(
		const vc_assets_t& in_assets, const int32_t in_tiles_on_source_x, const int32_t in_dst_x, const int32_t in_dst_y, const int32_t in_tile_index, const c_blob_t& in_src,
		micron_t& out_micron)
	{
		const c_vec2i_t TILE_SRC = vc_tile_src(in_tiles_on_source_x, in_tile_index);

		//FIXME: re-order the tiles in memory post-load so that each tile is cohesive memory (MLM_M_TILE_ASPECT x lots bitmap...) -> can write a more efficient blit

		//this separation of clipping vs non-clipping doesn't seem to make a difference (timed) because clipping is not in the inner loop...
#if 1
		if (
			in_dst_x >= 0 &&
			in_dst_y >= 0 &&
			in_dst_x + MLM_M_TILE_ASPECT <= out_micron.canvas_width &&
			in_dst_y + MLM_M_TILE_ASPECT <= out_micron.canvas_height
			)
		{
			if (NINJA_VC_TILE_BITS_KEY & in_assets.tile_bits[in_tile_index])
				vc_octamap_blit_key(in_dst_x, in_dst_y, MLM_M_TILE_ASPECT, MLM_M_TILE_ASPECT, TILE_SRC.x, TILE_SRC.y, in_src, out_micron);
			else
				vc_octamap_blit(in_dst_x, in_dst_y, MLM_M_TILE_ASPECT, MLM_M_TILE_ASPECT, TILE_SRC.x, TILE_SRC.y, in_src, out_micron);
		}
		else
		{
			if (NINJA_VC_TILE_BITS_KEY & in_assets.tile_bits[in_tile_index])
				vc_octamap_blit_key_clip(in_dst_x, in_dst_y, MLM_M_TILE_ASPECT, MLM_M_TILE_ASPECT, TILE_SRC.x, TILE_SRC.y, in_src, out_micron);
			else
				vc_octamap_blit_clip(in_dst_x, in_dst_y, MLM_M_TILE_ASPECT, MLM_M_TILE_ASPECT, TILE_SRC.x, TILE_SRC.y, in_src, out_micron);
		}
#else
		vc_octamap_blit_key_clip(
			dst_x, dst_y,
			MLM_M_TILE_ASPECT, MLM_M_TILE_ASPECT,
			src_x, src_y,
			src, out_canvas
		);
#endif
	}

	static bool __load_blob(
		const char* in_asset,
		c_blob_t& out_blob)
	{
		assert(nullptr == out_blob.data);
		assert(0 == out_blob.size);
		out_blob = fs_file_contents(in_asset);
		return nullptr != out_blob.data;
	}

	bool __tile_needs_key(const c_blob_t& in_src, const int32_t in_tiles_on_source_x, const int32_t in_tile_index)
	{
		const c_vec2i_t TILE_SRC = vc_tile_src(in_tiles_on_source_x, in_tile_index);

		/*
		//we are keying and flipping in y...
		const uint8_t* BYTE_SRC = fs_tga_pixels_palettized(in_src) + TILE_SRC.x + (FS_TGA_HEADER(in_src)->image_spec_height - 1 - TILE_SRC.y) * FS_TGA_HEADER(in_src)->image_spec_width;
		*/
		const uint8_t* PIXELS = fs_tga_pixels_palettized(in_src);
		const uint8_t* BYTE_SRC = fs_tga_src(FS_TGA_HEADER(in_src), PIXELS, TILE_SRC.x, TILE_SRC.y);
		for (
			int32_t y = 0;
			y < MLM_M_TILE_ASPECT;
			++y
			)
		{
			const uint8_t* BYTE_SCAN_SRC = BYTE_SRC;
			for (
				int32_t x = 0;
				x < MLM_M_TILE_ASPECT;
				++x
				)
			{
				if (MLM_VC_OCTAMAP_COLOR_KEY_INDEX == *BYTE_SRC)
					return true;
				++BYTE_SRC;
			}
			/*
			BYTE_SRC = BYTE_SCAN_SRC - FS_TGA_HEADER(in_src)->image_spec_width;
			*/
			BYTE_SRC = fs_tga_row_advance(FS_TGA_HEADER(in_src), BYTE_SCAN_SRC);
		}

		return false;
	}

	static bool __sprite_init(
		const bool in_hero, const char* in_file,
		vc_sprite_t& out_sprite)
	{
		if (!__load_blob(in_file, out_sprite.bitmap))
			return false;

		out_sprite.width = FS_TGA_HEADER(out_sprite.bitmap)->image_spec_width;
		out_sprite.half_width = out_sprite.width / 2;
		if (in_hero)
		{
			out_sprite.height = FS_TGA_HEADER(out_sprite.bitmap)->image_spec_height;
			out_sprite.half_height = out_sprite.half_width;
		}
		else
		{
			out_sprite.height = FS_TGA_HEADER(out_sprite.bitmap)->image_spec_height / 2;
			out_sprite.half_height = out_sprite.height / 2;
		}

		return true;
	}

#if 0
	static const vc_named_bitmap_t* __find_terminal_content(const vc_immutable_t& im, const char* aFile)
	{
		for (const vc_named_bitmap_t* CONTENT = im.terminal_content; CONTENT < im.terminal_content + im.num_terminal_content; ++CONTENT)
		{
			if (CONTENT->name == aFile)
				return CONTENT;
		}
		return nullptr;
	}
#endif

	static const vc_screen_t& __screen(const c_vec2i_t& in_screen, const vc_assets_t& in_assets)
	{
		const uint32_t OFFSET = in_screen.x + in_screen.y * NINJA_VC_SCREENS_IN_WORLD_X;
		assert(OFFSET < (NINJA_VC_SCREENS_IN_WORLD_X * NINJA_VC_SCREENS_IN_WORLD_Y));
		return in_assets.screens[OFFSET];
	}

#if 0
	static void __draw_terminal(const c_vec2i_t& aCamera, const uint32_t anOffset, const vc_octamap_t& bitmap, vc_canvas_t& canvas)
	{
		c_vec2i_t w = vc_world_to_screen_int(m_offset_to_world(anOffset), aCamera);

		w.x -= bitmap.header->image_spec_width / 2;
		w.y -= bitmap.header->image_spec_height + MLM_M_TILE_ASPECT;
		if (w.x < 0)
			w.x = 0;
		else if (w.x > (canvas.width - bitmap.header->image_spec_width))
			w.x = canvas.width - bitmap.header->image_spec_width;
		if (w.y < 0)
			w.y = 0;
		else if (w.y > (canvas.height - bitmap.header->image_spec_height))
			w.y = canvas.height - bitmap.header->image_spec_height;

		vc_octamap_blit(w.x, w.y, 0, 0, 0, 0, bitmap, canvas);
	}
#endif

	bool vc_reload_graphics(vc_assets_t& out_assets)
	{
		//tiles
		delete[] out_assets.tiles.data;
		out_assets.tiles = {};
		if (!__load_blob(ASSET_TILES, out_assets.tiles))
			return false;

		//calculate tile bits (key needs, etc)
		{
			const uint16_t TILES_ON_SOURCE_X = (uint16_t)vc_tiles_on_source_x(out_assets.tiles);

			for (
				int32_t tile = 0;
				tile < MLM_M_MAX_TILE;
				++tile
				)
			{
				if (__tile_needs_key(out_assets.tiles, TILES_ON_SOURCE_X, tile))
					out_assets.tile_bits[tile] |= NINJA_VC_TILE_BITS_KEY;
			}
		}

#if 0
		//farplanes
		static_assert(_countof(ASSET_TEMP_FARPLANE) == _countof(out_assets.farplane_assets), "wtf?");
		for (
			uint32_t i = 0;
			i < _countof(out_assets.farplane_assets);
			++i
			)
		{
			delete[] out_assets.farplane_assets[i].data;
			out_assets.farplane_assets[i] = {};
			if (!__load_blob(ASSET_TEMP_FARPLANE[i], out_assets.farplane_assets[i]))
				return false;
		}
#else
		delete[] out_assets.tempcave.data;
		out_assets.tempcave = {};
		//if (!__load_blob("tempcave.tga", out_assets.tempcave))
		if (!__load_blob("watercave.tga", out_assets.tempcave))
			return false;
		/*
		const fs_tga_header_t* HEADER = FS_TGA_HEADER(out_assets.tempcave);
		const uint8_t SCREEN_ORIGIN_BIT = HEADER->image_spec_descriptor & (1 << 5);
		SCREEN_ORIGIN_BIT;
		*/
#endif

		return true;
	}

	static bool __assets_init(vc_assets_t& out_assets, micron_t& out_micron)
	{
		//gfx
		{
			/*
			if (!__load_blob(ASSET_BIGFONT, out_assets.gui_big_font))
				return false;

			if (!__load_blob(ASSET_CURSOR, out_assets.cursor))
				return false;

			if (!__load_blob(ASSET_BLADES, out_assets.blades))
				return false;
				*/

			/*
			if (!__load_blob(ASSET_GIBS, out_assets.gibs))
				return false;
				*/
			if (!__load_blob(ASSET_IMPALEMENT, out_assets.impalement))
				return false;
			/*
			if (!__load_blob(ASSET_DROWNING, out_assets.drowning))
				return false;
			if (!__load_blob(ASSET_BURNING, out_assets.burning))
				return false;
			if (!__load_blob(ASSET_ELECTROCUTION, out_assets.electrination))
				return false;
				*/
			if (!__load_blob(ASSET_SPECIALICONS, out_assets.special_icons))
				return false;
			/*
			if (!__load_blob(ASSET_FISH, out_assets.fish))
				return false;
				*/

			if (!__sprite_init(true, ASSET_HERO, out_assets.hero))
				return false;

			/*
			if (!__load_blob(ASSET_HEAVY, out_assets.heavy))
				return false;

			if (!__sprite_init(false, ASSET_WALKER, out_assets.walker))
				return false;
			if (!__sprite_init(false, ASSET_WALKER2, out_assets.walker2))
				return false;

			if (!__sprite_init(false, ASSET_JUMPER, out_assets.jumper))
				return false;
			if (!__sprite_init(false, ASSET_JUMPER2, out_assets.jumper2))
				return false;

			if (!__sprite_init(false, ASSET_SWIMMER, out_assets.swimmer))
				return false;
			if (!__sprite_init(false, ASSET_SWIMMER2, out_assets.swimmer2))
				return false;
				*/
		}

		//sounds
		for (
			uint8_t i = 0;
			i < VC_NUM_SOUNDS;
			++i
			)
		{
			out_micron.sound_loads.push_back({ ASSET_SOUND[i], i });
		}

#if 0
		//terminal content
		{
			delete[] im.terminal_content;
			im.num_terminal_content = 0;

			w32_directory_tree_t tree;
			assert(0 && "this should be a dev only feature, in pub we can pool the referenced assets (see bloom farplanes)");
			w32_directory_tree("terminals", "tga", tree);
			if (tree.files.size())
			{
				im.terminal_content = new vc_named_bitmap_t[tree.files.size()];
				assert(im.terminal_content);
				im.num_terminal_content = tree.files.size();
				for (uint32_t i = 0; i < tree.files.size(); ++i)
				{
					im.terminal_content[i].name = tree.files[i];
					im.terminal_content[i].bitmap = {};
					if (!__load_blob(im.terminal_content[i].name.buffer, im.terminal_content[i].bitmap))
						return false;
				}
			}

			C_LOG("%u terminal_contents total", im.num_terminal_content);
		}
#endif

		{
			p_context_t view_context;
			view_context.read(ASSET_CONTEXT_VIEW);

			p_context_t farplanes_context;
			farplanes_context.read(ASSET_CONTEXT_FARPLANES);

			//tiles
			{
				for (
					uint32_t i = 0;
					i < VC_NUM_GFX;
					++i
					)
					out_assets.gfx[i] = view_context.pull_int32(GFX_KIND, GFX_KEY, VC_GFX_NAME_DEFAULT[i].name, VC_GFX_NAME_DEFAULT[i].default);

				for (
					uint32_t i = 0;
					i < MLM_M_MAX_TILE;
					++i
					)
				{
					out_assets.tile_info[i].anim_target = view_context.pull_uint32(TILEINFO_KIND, 1 + i, "myAnimTarget", i);	//default to point to self
					out_assets.tile_info[i].wang = view_context.pull_uint32(TILEINFO_KIND, 1 + i, "wang", 0);
				}

				if (!vc_reload_graphics(out_assets))
					return false;
			}

			//screens
			for (
				int32_t i = 0;
				i < NINJA_VC_SCREENS_IN_WORLD_X * NINJA_VC_SCREENS_IN_WORLD_Y;
				++i
				)
			{
				const uint32_t KEY = 1 + i;
				vc_screen_t& s = out_assets.screens[i];

				farplanes_context.pull_string(SCREEN_KIND, KEY, "ASSET_myAmbience", "none", s.ambience);
				s.restart = 0 != farplanes_context.pull_uint32(SCREEN_KIND, KEY, "myRestart", 0);
				s.room = farplanes_context.pull_uint32(SCREEN_KIND, KEY, "room", 0);
				s.plax = farplanes_context.pull_uint32(SCREEN_KIND, KEY, "plax", 0);
			}

#if 0
			//terminals
			{
				assert(nullptr == im.terminal);
				assert(0 == im.num_terminal);

				std::set<uint32_t>keys;
				p_context_get_all_keys(TERMINAL_KIND, keys, viewContext);
				if (keys.size())
				{
					im.terminal = new vc_terminal_t[keys.size()];
					if (im.terminal)
					{
						auto i = keys.cbegin();
						while (keys.cend() != i)
						{
							im.terminal[im.num_terminal].key = *i;
							im.terminal[im.num_terminal].contents = __find_terminal_content(im, p_string_puller_t(TERMINAL_KIND, *i, "ASSET_myScreen", "none", viewContext).result.buffer);
							im.terminal[im.num_terminal].offset = p_context_pull_int32(TERMINAL_KIND, *i, "myOffset", 0, viewContext);
							++im.num_terminal;

							++i;
						}
					}
				}
			}
#endif
		}

		return true;
	}

	//public
	//public
	//public
	//public

	c_vec2i_t vc_tile_src(const int32_t in_tiles_on_source_x, const int32_t in_tile_index)
	{
		return
		{
			(in_tile_index % in_tiles_on_source_x) * MLM_M_TILE_ASPECT,
				(in_tile_index / in_tiles_on_source_x) * MLM_M_TILE_ASPECT,
		};
	}

	bool vc_view_init(vc_assets_t& out_assets, vc_fatpack_t& out_fatpack, micron_t& out_micron)
	{
		if (!__assets_init(out_assets, out_micron))
			return false;
		vc_reset_animations(out_fatpack);

		/*
		out_fatpack.canvas = vc_canvas_make(MLM_VC_SCREEN_WIDTH, MLM_VC_SCREEN_HEIGHT);
		if (!out_fatpack.canvas.pixels)
			return false;
			*/

		return true;
	}

	c_vec2i_t vc_hero_screen(const m_mutable_t& in_mu)
	{
		c_vec2i_t p = c_floor_to_int(in_mu.hero_position);

		p.x /= MLM_VC_SCREEN_WIDTH;
		p.y /= MLM_VC_SCREEN_HEIGHT;

		return p;
	}

	const c_vec2i_t vc_world_to_screen_int(const c_vec2f_t& in_world, const c_vec2i_t& in_camera)
	{
		return c_floor_to_int(in_world) - in_camera;
	}

	const c_vec2f_t vc_world_to_screen_float(const c_vec2f_t& in_world, const c_vec2i_t& in_camera)
	{
		return in_world - c_vec2f(in_camera);
	}

	c_vec2f_t vc_screen_local(const c_vec2f_t& in_world)
	{
		c_vec2i_t screen_origin = c_floor_to_int(in_world);
		screen_origin.x /= MLM_VC_SCREEN_WIDTH;
		screen_origin.x *= MLM_VC_SCREEN_WIDTH;
		screen_origin.y /= MLM_VC_SCREEN_HEIGHT;
		screen_origin.y *= MLM_VC_SCREEN_HEIGHT;

		return { in_world.x - screen_origin.x, in_world.y - screen_origin.y };
	}

	void vc_tiles_clear_animations(vc_assets_t& out_assets, vc_fatpack_t& out_fatpack)
	{
		for (
			uint32_t i = 0;
			i < MLM_M_MAX_TILE;
			++i
			)
		{
			out_fatpack.current_tiles[i] = i;	//first frame
			out_assets.tile_info[i].anim_target = i;		//not pointing to anything else
		}
	}

	void vc_tiles_update_animations(
		const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack)
	{
		out_fatpack.tile_anim_tick += 16 * M_SECONDS_PER_TICK;
		while (out_fatpack.tile_anim_tick > 1.f)
		{
			for (
				uint32_t t = 0;
				t < MLM_M_MAX_TILE;
				++t
				)
			{
				out_fatpack.current_tiles[t] = in_assets.tile_info[out_fatpack.current_tiles[t]].anim_target;
			}
			out_fatpack.tile_anim_tick -= 1.f;
		}
	}

	void vc_fx_draw_all(
		const uint32_t in_paused, const uint32_t in_tick, const m_immutable_t& in_im, const c_vec2i_t& in_camera, const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack, micron_t& out_micron)
	{
		const int32_t TILES_ON_SOURCE_X = vc_tiles_on_source_x(in_assets.tiles);

#if 0
		vc_fx_t* i = out_fatpack.fx.pool;
		while (i < C_POOL_END(out_fatpack.fx))
		{
			if (__fx_draw(in_paused, in_tick, in_im, in_camera, in_assets, TILES_ON_SOURCE_X, i, out_fatpack))
				++i;
			else
				out_fatpack.fx.release(i);
		}
#else
		assert(0 == out_fatpack.deferred_fx.size());
		auto itr = out_fatpack.fx.begin();
		while (out_fatpack.fx.cend() != itr)
		{
			if (__fx_draw(in_paused, in_tick, in_camera, in_assets, TILES_ON_SOURCE_X, *itr, out_fatpack, out_micron))
			{
				++itr;
			}
			else
			{
				itr = out_fatpack.fx.erase(itr);
			}
		}
		for (const vc_fx_t& DFX : out_fatpack.deferred_fx)
		{
			assert(DFX.type == VC_FX_ANIMATED);
			__puff(16, 64.f, in_im, in_tick, false, DFX.position, out_fatpack);
			//__gibs(in_tick, DFX.position, in_assets, out_fatpack);
		}
		out_fatpack.deferred_fx.clear();
#endif
	}

	void vc_visualize_events(
		const m_immutable_t& in_im, const uint32_t in_tick, const m_events_t& in_events, const m_mutable_t& in_mu,
		vc_fatpack_t& out_fatpack, micron_t& out_micron)
	{
		//assert(0 == out_fatpack.request_sound_plays.size());
		assert(0 == out_micron.sound_plays.size());

		for (
			const m_event_t* EVENT = in_events.events;
			EVENT < in_events.events + in_events.count;
			++EVENT
			)
		{
			switch (EVENT->type)
			{
			default:
				assert(0);
				break;

			case M_EVT_DO_SWITCHES:
				//out_micron.sound_plays.push_back(VC_SND_SWITCH);
				break;

			case M_EVT_HERO_JUMP:
				//out_micron.sound_plays.push_back(VC_SND_JUMP1 + out_fatpack.prng.generate() % NINJA_VC_SOUNDS_NUM_JUMPS);
				out_micron.sound_plays.push_back(VC_SND_JUMP);
				break;

			case M_EVT_HERO_LANDED:
				out_micron.sound_plays.push_back(VC_SND_LAND);
				break;

			case M_EVT_HERO_DIED:
				switch (EVENT->argument)
				{
				case M_COD_IMPALED:
#if 0
					if (SNAP_IMPALEMENT)
					{
						const c_vec2i_t GRID = m_world_to_grid(EVENT->position);
						__new_animated_fx_t(in_tick, true, m_grid_to_world(GRID.x, GRID.y), in_assets.impalement, in_mu, out_fatpack);
					}
					else
					{
						__new_animated_fx_t(in_tick, true, EVENT->position, in_assets.impalement, in_mu, out_fatpack);
					}
#else
					__puff(16, 64.f, in_im, in_tick, false, EVENT->position, out_fatpack);
#endif
					break;

				case M_COD_DROWNED:
					//__new_animated_fx_t(in_tick, false, EVENT->position + c_vec2f_t{ 0.f, DROWNED_FX_OFFSET_Y }, in_assets.drowning, in_mu, out_fatpack);
					break;

				case M_COD_BURNED:
					//__new_animated_fx_t(in_tick, false, EVENT->position, in_assets.burning, in_mu, out_fatpack);
					break;

				case M_COD_ELECTRINATED:
					//__new_animated_fx_t(in_tick, false, EVENT->position, in_assets.electrination, in_mu, out_fatpack);
					break;

				default:
					__puff(16, 64.f, in_im, in_tick, false, EVENT->position, out_fatpack);
					//__gibs(in_tick, EVENT->position, in_assets, out_fatpack);
					break;
				}

				out_micron.sound_plays.push_back(VC_SND_DIE_IMPALED + EVENT->argument);
				break;

			case M_EVT_VISIT:
				if (out_fatpack.last_visit_offset != m_world_to_offset(EVENT->position))
				{
					/*
					{
						const uint16_t TILE = m_tile(EVENT->position, in_im);
						if (vc_gfx_is(VC_GFX_INFO, TILE, in_assets))
							out_micron.sound_plays.push_back(VC_SND_INFO);
						else if (vc_gfx_is(VC_GFX_INFO2, TILE, in_assets))
							out_micron.sound_plays.push_back(VC_SND_INFO2);
					}
					*/
					out_fatpack.last_visit_offset = m_world_to_offset(EVENT->position);
				}
				break;

			case M_EVT_CHECKPOINT:
				//out_micron.sound_plays.push_back(VC_SND_SAVE);
				break;

			case M_EVT_BACK_TO_CHECKPOINT:
				//out_micron.sound_plays.push_back(VC_SND_SAVE);
				__puff(16, 64.f, in_im, in_tick, false, in_mu.hero_position, out_fatpack);

				//recalc this explicitly to sync with checkpoint state
				vc_calculate_flowers(in_im, in_mu, out_fatpack);
				break;

			case M_EVT_PICKED_FLOWER:
			{
				const c_vec2i_t G = m_world_to_grid(EVENT->position);
				//__blades(in_tick, m_grid_to_world(G.x, G.y), in_assets, out_fatpack);
				//out_micron.sound_plays.push_back(VC_SND_PICKFLOWER);

				//do this instead of re-calcing from visitation data on the fly...
				++out_fatpack.cache_picked_flowers;
			}
			break;

			case M_EVT_JUMPER_IN:
				/*
				if (__on_hero_screen(EVENT->position, in_mu))
					out_micron.sound_plays.push_back(VC_SND_INWATER);
					*/
				break;

			case M_EVT_JUMPER_OUT:
				/*
				if (__on_hero_screen(EVENT->position, in_mu))
					out_micron.sound_plays.push_back(VC_SND_OUTWATER);
					*/
				break;

			case M_EVT_EXIT:
				//vc_idle_complete(in_mu, out_vc_mu);
				break;

			case M_EVT_BULLET_FIRE:
				//out_micron.sound_plays.push_back(VC_SND_BULLET_FIRE);
				break;

			case M_EVT_MOB_PRUNE:
				switch (EVENT->argument)
				{
				case M_LOGIC_BULLET:
					//out_micron.sound_plays.push_back(VC_SND_BULLET_HIT);
					__puff(8, 32.f, in_im, in_tick, false, EVENT->position, out_fatpack);
					break;

				case M_LOGIC_WALKER:
				case M_LOGIC_WALKER2:
					//out_micron.sound_plays.push_back(VC_SND_DIE_WALKER);
					__puff(16, 64.f, in_im, in_tick, false, EVENT->position, out_fatpack);
					break;
				}
				break;

			case M_EVT_MELEE_HIT:
				//out_micron.sound_plays.push_back(VC_SND_MELEE_HIT);
				__puff(4, 16.f, in_im, in_tick, false, EVENT->position, out_fatpack);
				break;
			}
		}
	}

	void vc_gui_draw_and_clear(vc_fatpack_t& out_fatpack, micron_t& out_micron)
	{
		for (
			const vc_text_t* T = out_fatpack.gui_text;
			T < out_fatpack.gui_text + out_fatpack.gui_num_text;
			++T
			)
		{
			assert(T->text.buffer && *T->text.buffer);

			//__octafont_print(in_assets.gui_big_font, T->position.x, T->position.y, T->text, out_micron);
			vc_canvas_atascii_print(T->x, T->y, 9, T->text.buffer, out_micron);
		}

		out_fatpack.gui_num_text = 0;
	}

	void vc_sprite_draw(
		const c_vec2i_t& in_position, const uint8_t in_frame, const bool in_mirror, const vc_sprite_t& in_sprite,
		micron_t& out_micron)
	{
		if (in_mirror)
		{
			vc_octamap_blit_key_clip_flip_x(
				in_position.x - in_sprite.half_width, in_position.y - in_sprite.half_height,
				FS_TGA_HEADER(in_sprite.bitmap)->image_spec_width, in_sprite.width,
				0, in_frame * in_sprite.width,
				in_sprite.bitmap, out_micron);
		}
		else
		{
			vc_octamap_blit_key_clip(
				in_position.x - in_sprite.half_width, in_position.y - in_sprite.half_height,
				FS_TGA_HEADER(in_sprite.bitmap)->image_spec_width, in_sprite.width,
				0, in_frame * in_sprite.width,
				in_sprite.bitmap, out_micron);
		}
	}

	void vc_draw_hero(
		const uint32_t in_tick, const m_immutable_t& in_im, const m_mutable_t& in_mu, const c_vec2i_t& in_camera, const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack, micron_t& out_micron)
	{
		if (!m_character_alive(in_mu))
			return;

#if 0

		uint8_t frame;

		if (in_mu.hero_air)
		{
			out_fatpack.hero_anim = 0.f;
			if (in_mu.hero_speed.y < 0.f)
			{
				if (__hero_near_ground(in_im, in_mu))
					frame = 9;//launch_frame
				else
					frame = 7;// air_up_frame;
			}
			else
			{
				if (__hero_near_ground(in_im, in_mu))
					frame = 10;//land_frame
				else
					frame = 8;// air_down_frame;
			}
		}
		else
		{
			if (
				(in_mu.hero_input & MLM_M_FLAGS_LEFT || in_mu.hero_input & MLM_M_FLAGS_RIGHT) &&
				in_mu.hero_speed.x
				)
			{
				const bool LAST_FRAME_STEP =
					3 == (uint8_t)out_fatpack.hero_anim ||
					6 == (uint8_t)out_fatpack.hero_anim;

				const bool IN_LIQUID = M_TYPE_LIQUID == m_tile_type_at_position(in_mu.hero_position, true, in_im, in_mu);
				if (IN_LIQUID)
					out_fatpack.hero_anim += 7.5f * M_SECONDS_PER_TICK;
				else if (MLM_M_FLAGS_SPRINT & in_mu.hero_input)
					out_fatpack.hero_anim += 30.f * M_SECONDS_PER_TICK;
				else
					out_fatpack.hero_anim += 15.f * M_SECONDS_PER_TICK;
				if (out_fatpack.hero_anim >= 7)
					out_fatpack.hero_anim = 1.f;

				const bool THIS_FRAME_STEP =
					3 == (uint8_t)out_fatpack.hero_anim ||
					6 == (uint8_t)out_fatpack.hero_anim;
				if (
					THIS_FRAME_STEP &&
					!LAST_FRAME_STEP
					)
					out_micron.sound_plays.push_back(uint32_t(3 == (uint8_t)out_fatpack.hero_anim ? VC_SND_STEP1 : VC_SND_STEP2));
			}
			else
			{
				out_fatpack.hero_anim = 0.f;
			}

			frame = (uint8_t)out_fatpack.hero_anim;
		}

		const c_vec2i_t SCREEN = vc_world_to_screen_int(in_mu.hero_position, in_camera);

		vc_sprite_draw(SCREEN, frame, in_mu.hero_mirror, in_assets.hero, out_micron);

		//melee test
		int32_t melee = in_tick - in_mu.hero_melee_tick;
		if (melee < 8)
		{
			const int32_t RANGE = MLM_M_MELEE_RANGE_X - 16;	//adjust this so it looks right
			const int32_t STEP = RANGE / 4;
			int32_t d;
			if (in_mu.hero_mirror)
				d = 1;
			else
				d = -1;
			if (melee < 4)
			{
				vc_canvas_line(SCREEN.x + (melee * STEP) * d, SCREEN.y - MLM_M_MELEE_RANGE_Y, SCREEN.x + RANGE * d, SCREEN.y - MLM_M_MELEE_RANGE_Y + melee * 8, 0, out_micron);
			}
			else
			{
				melee -= 4;
				vc_canvas_line(SCREEN.x + (RANGE - melee * STEP) * d, SCREEN.y + MLM_M_MELEE_RANGE_Y, SCREEN.x + RANGE * d, SCREEN.y - MLM_M_MELEE_RANGE_Y + melee * 8, 0, out_micron);
			}
		}

#else

		in_tick;
		in_im;
		out_fatpack;

		const c_vec2i_t SCREEN = vc_world_to_screen_int(in_mu.hero_position, in_camera);
		if (in_mu.hero_air)
		{
			out_fatpack.hero_anim = 0.f;
		}
		else
		{
			if (
				(in_mu.hero_input & MLM_M_FLAGS_LEFT || in_mu.hero_input & MLM_M_FLAGS_RIGHT) &&
				in_mu.hero_speed.x
				)
			{
				const bool LAST_FRAME_STEP = 2 == (int32_t)out_fatpack.hero_anim;

				out_fatpack.hero_anim += 12.f * M_SECONDS_PER_TICK;
				if (out_fatpack.hero_anim >= 6)
					out_fatpack.hero_anim = 1.f;

				const bool THIS_FRAME_STEP = 2 == (int32_t)out_fatpack.hero_anim;

				if (
					THIS_FRAME_STEP &&
					!LAST_FRAME_STEP
					)
					out_micron.sound_plays.push_back(VC_SND_STEP);
			}
			else
			{
				out_fatpack.hero_anim = 0.f;
			}
		}
		const int32_t FRAME = (int32_t)out_fatpack.hero_anim;
		if (in_mu.hero_mirror)
		{
			vc_octamap_blit_key_clip_flip_x(
				SCREEN.x - in_assets.hero.half_width, SCREEN.y - in_assets.hero.half_height * 2,
				FS_TGA_HEADER(in_assets.hero.bitmap)->image_spec_width, in_assets.hero.width * 2,
				0, FRAME * in_assets.hero.width * 2,
				in_assets.hero.bitmap, out_micron);
		}
		else
		{
			vc_octamap_blit_key_clip(
				SCREEN.x - in_assets.hero.half_width, SCREEN.y - in_assets.hero.half_height * 2,
				FS_TGA_HEADER(in_assets.hero.bitmap)->image_spec_width, in_assets.hero.width * 2,
				0, FRAME * in_assets.hero.width * 2,
				in_assets.hero.bitmap, out_micron);
		}

#endif

	}

	void vc_do_hero_movement(
		const micron_t& in_micron, const m_immutable_t& in_im,
		m_mutable_t& out_mu)
	{
		uint8_t input = 0;

		//hero movement when character is alive
		if (m_character_alive(out_mu))
		{
			/*
			if (micron_key_is_down(in_micron, 'S'))
				input |= MLM_M_FLAGS_DOWN;
				*/

			if (
				micron_key_is_down(in_micron, 'A') ||
				micron_key_is_down(in_micron, MICRON_KEY_LEFT)
				)
				input |= MLM_M_FLAGS_LEFT;

			if (
				micron_key_is_down(in_micron, 'D') ||
				micron_key_is_down(in_micron, MICRON_KEY_RIGHT)
				)
				input |= MLM_M_FLAGS_RIGHT;

			if (
				micron_key_is_down(in_micron, 'K') ||
				micron_key_is_down(in_micron, MICRON_KEY_UP)
				)
				input |= MLM_M_FLAGS_JUMP;

			/*
			if (micron_key_is_down(in_micron, 'J'))
				input |= MLM_M_FLAGS_SPRINT;

			if (micron_key_is_down(in_micron, 'L'))
				input |= MLM_M_FLAGS_MELEE;
				*/
		}

		//always set
		m_set_hero_input(in_im, input, out_mu);
	}

	void vc_draw_plax(
		const uint32_t in_tick, const m_immutable_t& in_im, const m_mutable_t& in_mu, const vc_assets_t& in_assets, const c_vec2i_t& in_camera,
		c_xorshift128_t& out_prng, micron_t& out_micron)
	{
#if 0
		const int32_t SCREEN_X = in_camera.x / (MLM_M_TILE_ASPECT * NINJA_VC_TILES_ON_SCREEN_X);
		const int32_t SCREEN_Y = in_camera.y / (MLM_M_TILE_ASPECT * NINJA_VC_TILES_ON_SCREEN_Y);
		const uint32_t SCREEN_OFFSET = SCREEN_X + SCREEN_Y * NINJA_VC_SCREENS_IN_WORLD_X;
		assert(SCREEN_OFFSET < (NINJA_VC_SCREENS_IN_WORLD_X * NINJA_VC_SCREENS_IN_WORLD_Y));
		const uint32_t PLAX = in_assets.screens[SCREEN_OFFSET].plax;

		uint32_t layer_start = 0;
		uint32_t layer_count = 0;

		switch (PLAX % VC_PLAX_NUM_STYLES)
		{
		default:
			assert(0);
			break;

		case VC_PLAX_SNOWY_MOUNTAINS:
			layer_start = 0;
			layer_count = 6;
			break;

		case VC_PLAX_PURPLE_CAVES:
			layer_start = 6;
			layer_count = 5;
			break;

		case VC_PLAX_UNDERSEA:
			layer_start = 11;
			layer_count = 3;
			break;

		case VC_PLAX_WOODY:
			layer_start = 14;
			layer_count = 7;
			break;

		case VC_PLAX_MONOLITH:
			layer_start = 21;
			layer_count = 4;
			break;

		case VC_PLAX_VAMPIRE:
			layer_start = 25;
			layer_count = 2;
			break;

		case VC_PLAX_CITY:
			layer_start = 27;
			layer_count = 5;
			break;

		case VC_PLAX_MUMIN:
			layer_start = 32;
			layer_count = 6;
			break;

		case VC_PLAX_MIRKWOOD:
			layer_start = 38;
			layer_count = 7;
			break;

		case VC_PLAX_INDUSTRIAL:
			layer_start = 45;
			layer_count = 4;
			break;

		case VC_PLAX_ANOTHER_WORLD:
			layer_start = 49;
			layer_count = 2;
			break;

		case VC_PLAX_ALIEN:
			layer_start = 51;
			layer_count = 3;
			break;
		}

		const int32_t FISH_FRAME = (in_tick / 12) % 4;
		int32_t fish_y = 0;

		//layers loop
		int32_t div = (1 << layer_count);
		for (
			uint32_t layer = 0;
			layer < layer_count;
			++layer
			)
		{
			const c_blob_t& BITMAP = in_assets.farplane_assets[layer_start + layer];
			assert(div > 1);

			const int32_t STATIC_X = (in_camera.x / div) % FS_TGA_HEADER(BITMAP)->image_spec_width;
			const int32_t MOVING_X = ((in_tick + in_camera.x) / div) % FS_TGA_HEADER(BITMAP)->image_spec_width;

			int32_t x;

			switch (PLAX % VC_PLAX_NUM_STYLES)
			{
			default:
				x = STATIC_X;
				break;

			case VC_PLAX_SNOWY_MOUNTAINS:
				if (2 == layer)
					x = STATIC_X;
				else
					x = MOVING_X;
				break;

			case VC_PLAX_MONOLITH:
				if (1 == layer || 2 == layer)
					x = MOVING_X;
				else
					x = STATIC_X;
				break;

			case VC_PLAX_ANOTHER_WORLD:
				if (0 == layer)
					x = MOVING_X;
				else
					x = STATIC_X;
				break;
			}

			//layer
			x = -x;
			while (x < out_micron.canvas_width)
			{
				vc_octamap_blit_key_clip(x, 0, 0, 0, 0, 0, BITMAP, out_micron);
				x += FS_TGA_HEADER(BITMAP)->image_spec_width;
			}

			//extras
			switch (PLAX % VC_PLAX_NUM_STYLES)
			{
				//FIXME: not seamless
			case VC_PLAX_UNDERSEA:
				x = ((2 * in_tick + in_camera.x) / div) % out_micron.canvas_width;
				x = -x;
				while (x < out_micron.canvas_width)
				{
					vc_octamap_blit_key_clip(x, fish_y, 0, FS_TGA_HEADER(in_assets.fish)->image_spec_width, 0, FISH_FRAME * FS_TGA_HEADER(in_assets.fish)->image_spec_width, in_assets.fish, out_micron);
					vc_octamap_blit_key_clip(FS_TGA_HEADER(in_assets.fish)->image_spec_width + x, FS_TGA_HEADER(in_assets.fish)->image_spec_width / 2 + fish_y, 0, FS_TGA_HEADER(in_assets.fish)->image_spec_width, 0, (4 + FISH_FRAME) * FS_TGA_HEADER(in_assets.fish)->image_spec_width, in_assets.fish, out_micron);
					vc_octamap_blit_key_clip(2 * FS_TGA_HEADER(in_assets.fish)->image_spec_width + x, 3 * FS_TGA_HEADER(in_assets.fish)->image_spec_width / 2 + fish_y, 0, FS_TGA_HEADER(in_assets.fish)->image_spec_width, 0, (8 + FISH_FRAME) * FS_TGA_HEADER(in_assets.fish)->image_spec_width, in_assets.fish, out_micron);

					fish_y += FS_TGA_HEADER(in_assets.fish)->image_spec_width;
					x += FS_TGA_HEADER(BITMAP)->image_spec_width;
				}
				break;
			}

			//next layer
			div /= 2;
		}

		//noise particles (snow)
		if (VC_PLAX_SNOWY_MOUNTAINS == PLAX % VC_PLAX_NUM_STYLES)
			__noise_particles(in_tick, false, in_im, in_mu, { (float)in_camera.x, (float)in_camera.y }, out_prng, out_micron);
#else
		in_tick;
		in_im;
		in_mu;
		in_assets;
		in_camera;
		out_prng;
		vc_octamap_blit_clip(0, 0, 0, 0, 0, 0, in_assets.tempcave, out_micron);
#endif
	}

	void vc_reset_animations(vc_fatpack_t& out_fatpack)
	{
		for (
			int32_t t = 0;
			t < MLM_M_MAX_TILE;
			++t
			)
			out_fatpack.current_tiles[t] = t;	//first frame
	}

	/*
	void vc_do_ambience(
		const uint32_t in_tick, const char* in_file, const float in_max_volume, const vc_assets_t& in_assets,
		vc_fatpack_t& out_fatpack)
	{
		if (out_fatpack.bit_play_music)
		{
			out_fatpack.ambience.set(FS_VIRTUAL, in_assets.engine, in_file, true, true, in_max_volume);
			out_fatpack.ambience.update(FS_VIRTUAL, in_assets.engine, in_tick * M_SECONDS_PER_TICK, in_max_volume);
		}
		else
		{
			out_fatpack.ambience.cleanup();
		}
	}
	*/

	void vc_calculate_flowers(
		const m_immutable_t& in_im, const m_mutable_t& in_mu,
		vc_fatpack_t& out_fatpack)
	{
		out_fatpack.cache_total_flowers = 0;
		out_fatpack.cache_picked_flowers = 0;
		for (
			uint32_t i = 0;
			i < MLM_M_WORLD_SIZE;
			++i
			)
		{
			if (m_logic_is(M_LOGIC_FLOWER, in_im.world_tiles[i], in_im))
				++out_fatpack.cache_total_flowers;

			if (
				m_logic_is(M_LOGIC_FLOWER, in_im.world_tiles[i], in_im) &&
				in_mu.world_visited[i]
				)
				++out_fatpack.cache_picked_flowers;
		}
	}

	uint32_t vc_gui_big_text(
		const micron_t& in_micron, const int32_t in_x, const int32_t in_y, const int32_t in_key, const char* in_text,
		vc_fatpack_t& out_fatpack)
	{
		const vc_text_t* T = nullptr;

		if (
			in_text &&
			*in_text
			)
			T = __add_text(in_x, in_y, in_key, in_text, out_fatpack);

		if (T)
			//return __gui_invisible_button(in_micron, T->x, T->y, ::strlen(T->text.buffer) * 8, 8, in_key);
			if (micron_key_upflank(in_micron, in_key))
				return MLM_VC_GUI_LEFT;

		return 0;
	}

	int32_t vc_tiles_on_source_x(const c_blob_t& in_tiles)
	{
		return FS_TGA_HEADER(in_tiles)->image_spec_width / MLM_M_TILE_ASPECT;
	}

	bool vc_gfx_is(const uint32_t in_value, const uint16_t in_tile, const vc_assets_t& in_assets)
	{
		assert(in_value < _countof(in_assets.gfx));
		return in_assets.gfx[in_value] == in_tile;
	}

	uint32_t vc_gfx_index_of(const uint32_t in_value, const vc_assets_t& in_assets)
	{
		assert(in_value < _countof(in_assets.gfx));
		return in_assets.gfx[in_value];
	}

	uint32_t vc_wang_blob_flags(const int32_t in_x, const int32_t in_y, const uint32_t in_index, const m_immutable_t& in_im)
	{
		uint32_t bits256 = 0;

		if (in_index == m_tile(in_x, in_y - 1, in_im))
			bits256 |= WANG_BLOB_NORTH;

		if (in_index == m_tile(in_x + 1, in_y - 1, in_im))
			bits256 |= WANG_BLOB_NORTHEAST;

		if (in_index == m_tile(in_x + 1, in_y, in_im))
			bits256 |= WANG_BLOB_EAST;

		if (in_index == m_tile(in_x + 1, in_y + 1, in_im))
			bits256 |= WANG_BLOB_SOUTHEAST;

		if (in_index == m_tile(in_x, in_y + 1, in_im))
			bits256 |= WANG_BLOB_SOUTH;

		if (in_index == m_tile(in_x - 1, in_y + 1, in_im))
			bits256 |= WANG_BLOB_SOUTHWEST;

		if (in_index == m_tile(in_x - 1, in_y, in_im))
			bits256 |= WANG_BLOB_WEST;

		if (in_index == m_tile(in_x - 1, in_y - 1, in_im))
			bits256 |= WANG_BLOB_NORTHWEST;

		uint32_t result = (WANG_BLOB_NORTH | WANG_BLOB_EAST | WANG_BLOB_SOUTH | WANG_BLOB_WEST) & bits256;

		if (__all_bits(WANG_BLOB_NORTH | WANG_BLOB_EAST | WANG_BLOB_NORTHEAST, bits256))
			result |= WANG_BLOB_NORTHEAST;

		if (__all_bits(WANG_BLOB_EAST | WANG_BLOB_SOUTH | WANG_BLOB_SOUTHEAST, bits256))
			result |= WANG_BLOB_SOUTHEAST;

		if (__all_bits(WANG_BLOB_SOUTH | WANG_BLOB_WEST | WANG_BLOB_SOUTHWEST, bits256))
			result |= WANG_BLOB_SOUTHWEST;

		if (__all_bits(WANG_BLOB_WEST | WANG_BLOB_NORTH | WANG_BLOB_NORTHWEST, bits256))
			result |= WANG_BLOB_NORTHWEST;

		return result;
	}

	void vc_tiles_draw(
		const m_immutable_t& in_im, const m_mutable_t& in_mu, const vc_assets_t& in_assets, const c_vec2i_t& in_camera, const uint8_t in_flags,
		vc_fatpack_t& out_fatpack, micron_t& out_micron)
	{
		//const double QP_START = w32_query_performance_timer_now();

		const uint16_t TILES_ON_SOURCE_X = (uint16_t)vc_tiles_on_source_x(in_assets.tiles);

		//calc starting grid
		const c_vec2i_t GRID{ in_camera.x / MLM_M_TILE_ASPECT, in_camera.y / MLM_M_TILE_ASPECT };

		//calc subtile shift
		const c_vec2i_t SHIFT{ in_camera.x % MLM_M_TILE_ASPECT, in_camera.y % MLM_M_TILE_ASPECT };

		//-1 in each axis to support our custom wang 2-corner systems
		//+1 in each axis to support scrolling (normal and wang)
		for (
			int32_t y = -1;
			y < NINJA_VC_TILES_ON_SCREEN_Y + 1;
			++y
			)
		{
			for (
				int32_t x = -1;
				x < NINJA_VC_TILES_ON_SCREEN_X + 1;
				++x
				)
			{
#if WANG_2_CORNER_SYSTEMS
				const wang_2_corner_system_t WANG_2_CORNER_SYSTEM = __wang_2_corner_system(GRID.x + x, GRID.y + y, in_im, in_assets);
				if (WANG_2_CORNER_SYSTEM.bits)
				{
					for (
						uint32_t i = 0;
						i < 4;
						++i
						)
					{
						if ((1 << i) & WANG_2_CORNER_SYSTEM.bits)
						{
							const int32_t DRAW_INDEX = WANG_2_CORNER_SYSTEM.systems[i].index + WANG_2_CORNER_OFFSETS[WANG_2_CORNER_SYSTEM.systems[i].flags];
							assert(DRAW_INDEX >= 0 && DRAW_INDEX < MLM_M_MAX_TILE);

							__draw_tile(
								in_assets,
								TILES_ON_SOURCE_X,
								MLM_M_TILE_ASPECT / 2 + x * MLM_M_TILE_ASPECT - SHIFT.x,
								MLM_M_TILE_ASPECT / 2 + y * MLM_M_TILE_ASPECT - SHIFT.y,
								out_fatpack.current_tiles[DRAW_INDEX],
								in_assets.tiles,
								out_micron
							);
						}
					}
				}
#endif

				//negactive only required for wang 2-corner systems (shifted +8 in each axis)
				if (
					-1 == y ||
					-1 == x
					)
					continue;

				const uint16_t TILE = m_tile(GRID.x + x, GRID.y + y, in_im);
				const uint8_t VISITED = m_visited(GRID.x + x, GRID.y + y, in_mu);
				const uint32_t TYPE = m_tile_type(TILE, false, in_im, in_mu);

				if (
					DRAW_PASSAGE(in_flags) ||
					DRAW_NON_PASSAGE(in_flags)
					)
				{
					uint16_t index = TILE;

					//replacements
					if (in_flags & NINJA_VC_TILEFLAGS_REPLACE)
					{
						//LOGIC
						//current biswitch
						if (m_logic_is(M_LOGIC_BISWITCH, TILE, in_im))
						{
							index += in_mu.bi;
						}
						//biswitch false
						else if (m_logic_is(M_LOGIC_FALSE, TILE, in_im))
						{
							if (in_mu.bi != 0)
								index = (uint16_t)vc_gfx_index_of(VC_GFX_CLEAR, in_assets);
						}
						//biswitch true
						else if (m_logic_is(M_LOGIC_TRUE, TILE, in_im))
						{
							if (in_mu.bi != 1)
								index = (uint16_t)vc_gfx_index_of(VC_GFX_CLEAR, in_assets);
						}
						//current triswitch
						if (m_logic_is(M_LOGIC_TRISWITCH, TILE, in_im))
						{
							index += in_mu.tri;
						}
						//rgb based on triswitch
						else if (m_logic_is(M_LOGIC_RED, TILE, in_im))
						{
							if (in_mu.tri != 0)
								index = (uint16_t)vc_gfx_index_of(VC_GFX_CLEAR, in_assets);
#if PUSH_RGB_GLOWS
							else
								mu.rgb_glows.push_back({ 0, x * MLM_M_TILE_ASPECT - shift.x, y * MLM_M_TILE_ASPECT - shift.y });
#endif
						}
						else if (m_logic_is(M_LOGIC_GREEN, TILE, in_im))
						{
							if (in_mu.tri != 1)
								index = (uint16_t)vc_gfx_index_of(VC_GFX_CLEAR, in_assets);
#if PUSH_RGB_GLOWS
							else
								mu.rgb_glows.push_back({ 1, x * MLM_M_TILE_ASPECT - shift.x, y * MLM_M_TILE_ASPECT - shift.y });
#endif
						}
						else if (m_logic_is(M_LOGIC_BLUE, TILE, in_im))
						{
							if (in_mu.tri != 2)
								index = (uint16_t)vc_gfx_index_of(VC_GFX_CLEAR, in_assets);
#if PUSH_RGB_GLOWS
							else
								mu.rgb_glows.push_back({ 2, x * MLM_M_TILE_ASPECT - shift.x, y * MLM_M_TILE_ASPECT - shift.y });
#endif
						}
						//taken flowers
						else if (m_logic_is(M_LOGIC_FLOWER, TILE, in_im))
						{
							if (VISITED != MLM_M_VISITED_NOT)
								index = (uint16_t)vc_gfx_index_of(VC_GFX_CLEAR, in_assets);
						}
						//diverse -> clear
						else if (
							m_logic_is(M_LOGIC_WALKER, TILE, in_im) ||
							m_logic_is(M_LOGIC_WALKER2, TILE, in_im) ||
							m_logic_is(M_LOGIC_EXIT_INVISIBLE, TILE, in_im)
							)
						{
							index = (uint16_t)vc_gfx_index_of(VC_GFX_CLEAR, in_assets);
						}
						//diverse -> water
						else if (
							m_logic_is(M_LOGIC_JUMPER, TILE, in_im) ||
							m_logic_is(M_LOGIC_JUMPER2, TILE, in_im) ||
							m_logic_is(M_LOGIC_SWIMMER, TILE, in_im) ||
							m_logic_is(M_LOGIC_SWIMMER2, TILE, in_im)
							)
						{
							index = (uint16_t)vc_gfx_index_of(VC_GFX_WATER, in_assets);
						}
						//current checkpoint replace
						else if (m_logic_is(M_LOGIC_CHECKPOINT, TILE, in_im))
						{
							if ((uint32_t)(GRID.x + x + (GRID.y + y) * MLM_M_WORLD_WIDTH) == in_mu.checkpoint_offset)
								index = (uint16_t)vc_gfx_index_of(VC_GFX_CHECKPOINT_CURRENT, in_assets);
						}

						//SPECIAL
						//abduct tiles -> clear
						else if (M_TYPE_ABDUCT == TYPE && !m_character_being_abducted(in_im, in_mu))
						{
							index = (uint16_t)vc_gfx_index_of(VC_GFX_CLEAR, in_assets);
						}

						//GFX
						//info tiles touch
						else if (vc_gfx_is(VC_GFX_INFO, TILE, in_assets))
						{
							if (VISITED != MLM_M_VISITED_NOT)
								index = (uint16_t)vc_gfx_index_of(VC_GFX_INFO_TOUCHED, in_assets);
						}
						else if (vc_gfx_is(VC_GFX_INFO2, TILE, in_assets))
						{
							if (VISITED != MLM_M_VISITED_NOT)
								index = (uint16_t)vc_gfx_index_of(VC_GFX_INFO2_TOUCHED, in_assets);
						}
					}//replacement

					if (WANG_2_CORNER == in_assets.tile_info[index].wang)
						index = (uint16_t)vc_gfx_index_of(VC_GFX_CLEAR, in_assets);

					//FIXME: should we really be doing this POST potential replacement?
					if (WANG_BLOB == in_assets.tile_info[index].wang)
					{
						const uint32_t WANG_BLOB_FLAGS = vc_wang_blob_flags(GRID.x + x, GRID.y + y, index, in_im);
						switch (WANG_BLOB_FLAGS)
						{
						default:	index += 6;	break;

						case 0:		break;

						case 4:		index += 1;	break;
						case 92:	index += 2;	break;
						case 124:	index += 3;	break;
						case 116:	index += 4;	break;
						case 80:	index += 5;	break;

						case 16:	index += 0 + TILES_ON_SOURCE_X;	break;
						case 20:	index += 1 + TILES_ON_SOURCE_X;	break;
						case 87:	index += 2 + TILES_ON_SOURCE_X;	break;
						case 223:	index += 3 + TILES_ON_SOURCE_X;	break;
						case 241:	index += 4 + TILES_ON_SOURCE_X;	break;
						case 21:	index += 5 + TILES_ON_SOURCE_X;	break;
						case 64:	index += 6 + TILES_ON_SOURCE_X;	break;

						case 29:	index += 0 + 2 * TILES_ON_SOURCE_X;	break;
						case 117:	index += 1 + 2 * TILES_ON_SOURCE_X;	break;
						case 85:	index += 2 + 2 * TILES_ON_SOURCE_X;	break;
						case 71:	index += 3 + 2 * TILES_ON_SOURCE_X;	break;
						case 221:	index += 4 + 2 * TILES_ON_SOURCE_X;	break;
						case 125:	index += 5 + 2 * TILES_ON_SOURCE_X;	break;
						case 112:	index += 6 + 2 * TILES_ON_SOURCE_X;	break;

						case 31:	index += 0 + 3 * TILES_ON_SOURCE_X;	break;
						case 253:	index += 1 + 3 * TILES_ON_SOURCE_X;	break;
						case 113:	index += 2 + 3 * TILES_ON_SOURCE_X;	break;
						case 28:	index += 3 + 3 * TILES_ON_SOURCE_X;	break;
						case 127:	index += 4 + 3 * TILES_ON_SOURCE_X;	break;
						case 247:	index += 5 + 3 * TILES_ON_SOURCE_X;	break;
						case 209:	index += 6 + 3 * TILES_ON_SOURCE_X;	break;

						case 23:	index += 0 + 4 * TILES_ON_SOURCE_X;	break;
						case 199:	index += 1 + 4 * TILES_ON_SOURCE_X;	break;
						case 213:	index += 2 + 4 * TILES_ON_SOURCE_X;	break;
						case 95:	index += 3 + 4 * TILES_ON_SOURCE_X;	break;
						case 255:	index += 4 + 4 * TILES_ON_SOURCE_X;	break;
						case 245:	index += 5 + 4 * TILES_ON_SOURCE_X;	break;
						case 81:	index += 6 + 4 * TILES_ON_SOURCE_X;	break;

						case 5:		index += 0 + 5 * TILES_ON_SOURCE_X;	break;
						case 84:	index += 1 + 5 * TILES_ON_SOURCE_X;	break;
						case 93:	index += 2 + 5 * TILES_ON_SOURCE_X;	break;
						case 119:	index += 3 + 5 * TILES_ON_SOURCE_X;	break;
						case 215:	index += 4 + 5 * TILES_ON_SOURCE_X;	break;
						case 193:	index += 5 + 5 * TILES_ON_SOURCE_X;	break;
						case 17:	index += 6 + 5 * TILES_ON_SOURCE_X;	break;

						case 1:		index += 1 + 6 * TILES_ON_SOURCE_X;	break;
						case 7:		index += 2 + 6 * TILES_ON_SOURCE_X;	break;
						case 197:	index += 3 + 6 * TILES_ON_SOURCE_X;	break;
						case 69:	index += 4 + 6 * TILES_ON_SOURCE_X;	break;
						case 68:	index += 5 + 6 * TILES_ON_SOURCE_X;	break;
						case 65:	index += 6 + 6 * TILES_ON_SOURCE_X;	break;
						}

						__draw_tile(
							in_assets,
							TILES_ON_SOURCE_X,
							x * MLM_M_TILE_ASPECT - SHIFT.x,
							y * MLM_M_TILE_ASPECT - SHIFT.y,
							out_fatpack.current_tiles[index],
							in_assets.tiles,
							out_micron
						);
					}
					else
					{
						__draw_tile(
							in_assets,
							TILES_ON_SOURCE_X,
							x * MLM_M_TILE_ASPECT - SHIFT.x,
							y * MLM_M_TILE_ASPECT - SHIFT.y,
							out_fatpack.current_tiles[index],
							in_assets.tiles,
							out_micron
						);
					}
				}//if (DRAW_PASSAGE || DRAW_NON_PASSAGE)
			}//for (int32_t x = 0; x < VC_TILES_ON_SCREEN_X + 1; ++x)
		}//for (int32_t y = 0; y < VC_TILES_ON_SCREEN_Y + 1; ++y)

		/*
		vc_tiles_draw_time += w32_query_performance_timer_now() - QP_START;
		++vc_tiles_draw_count;
		*/
	}//void vc_tiles_draw()

	/*
	void vc_play(const c_vec2f_t& in_hero_position, const uint32_t in_id, const c_vec2f_t& in_position, const vc_assets_t& in_assets)
	{
		ds_space2d_player_t{ in_hero_position, 1.f, 1024.f }.play(in_assets.sounds, in_id, in_position);
	}

	void vc_do_hero_sound(const uint32_t in_tick, const m_immutable_t& in_im, const m_mutable_t& in_mu, const vc_assets_t& in_assets)
	{
		in_assets.sounds.play_looped(

			m_game_active(in_mu) &&
			m_character_alive(in_mu) &&
			in_mu.hero_air &&
			(m_character_is_solid_left_more(in_im, in_mu) || m_character_is_solid_right(in_im, in_mu)),

			VC_SND_SLIDE,
			::fabsf(in_mu.hero_speed.y) / M_CHAR_MAX_AIR_SPEED_Y,
			0.f,
			1.f,
			nullptr
		);

		if (in_tick == in_mu.hero_melee_tick)
			vc_play(in_mu.hero_position, VC_SND_MELEE, in_mu.hero_position, in_assets);

	}
	*/

	bool vc_restart(const c_vec2i_t& in_screen, const vc_assets_t& in_assets)
	{
		return __screen(in_screen, in_assets).restart;
	}

	const char* vc_ambience(const c_vec2i_t& in_screen, const vc_assets_t& in_assets)
	{
		const vc_screen_t& S = __screen(in_screen, in_assets);
		if (
			S.ambience.buffer &&
			*S.ambience.buffer
			)
			return S.ambience.buffer;

		return "none";
	}

	void vc_persist(const vc_assets_t& in_assets)
	{
		{
			p_context_t view;

			for (
				uint32_t i = 0;
				i < VC_NUM_GFX;
				++i
				)
				view.push_int32(GFX_KIND, GFX_KEY, VC_GFX_NAME_DEFAULT[i].name, in_assets.gfx[i]);

			for (
				uint32_t i = 0;
				i < MLM_M_MAX_TILE;
				++i
				)
			{
				view.push_uint32(TILEINFO_KIND, 1 + i, "myAnimTarget", in_assets.tile_info[i].anim_target);
				view.push_uint32(TILEINFO_KIND, 1 + i, "wang", in_assets.tile_info[i].wang);
			}
#if 0
			for (uint32_t i = 0; i < im.num_terminal; ++i)
			{
				p_context_push_string(TERMINAL_KIND, im.terminal[i].key, "ASSET_myScreen", im.terminal[i].contents->name.buffer, view);
				p_context_push_int32(TERMINAL_KIND, im.terminal[i].key, "myOffset", im.terminal[i].offset, view);
			}
#endif

			view.write(ASSET_CONTEXT_VIEW);
		}

		{
			p_context_t farplanes;

			for (
				uint32_t i = 0;
				i < NINJA_VC_SCREENS_IN_WORLD_X * NINJA_VC_SCREENS_IN_WORLD_Y;
				++i
				)
			{
				const uint32_t KEY = 1 + i;
				const vc_screen_t& S = in_assets.screens[i];

				farplanes.push_string(SCREEN_KIND, KEY, "ASSET_myAmbience", S.ambience.buffer);
				farplanes.push_uint32(SCREEN_KIND, KEY, "myRestart", S.restart);
				farplanes.push_uint32(SCREEN_KIND, KEY, "room", S.room);
				farplanes.push_uint32(SCREEN_KIND, KEY, "plax", S.plax);
			}

			farplanes.write(ASSET_CONTEXT_FARPLANES);
		}
	}

	const vc_screen_t& vc_world_to_screen(const c_vec2i_t& in_world, const vc_assets_t& in_assets)
	{
		return __screen({ in_world.x / (MLM_M_TILE_ASPECT * NINJA_VC_TILES_ON_SCREEN_X), in_world.y / (MLM_M_TILE_ASPECT * NINJA_VC_TILES_ON_SCREEN_Y) }, in_assets);
	}

	uint32_t vc_screen_cambits(const c_vec2i_t& in_screen, const vc_assets_t& in_assets)
	{
		const uint32_t SCREEN_OFFSET = in_screen.x + in_screen.y * NINJA_VC_SCREENS_IN_WORLD_X;
		assert(SCREEN_OFFSET < (NINJA_VC_SCREENS_IN_WORLD_X * NINJA_VC_SCREENS_IN_WORLD_Y));
		const uint32_t THIS_ROOM = in_assets.screens[SCREEN_OFFSET].room;

		uint32_t result = 0;

		//top
		if (
			in_screen.y == 0 ||
			THIS_ROOM != in_assets.screens[in_screen.x + (in_screen.y - 1) * NINJA_VC_SCREENS_IN_WORLD_X].room
			)
			result |= 1;

		//bottom
		if (
			in_screen.y == (NINJA_VC_SCREENS_IN_WORLD_Y - 1) ||
			THIS_ROOM != in_assets.screens[in_screen.x + (in_screen.y + 1) * NINJA_VC_SCREENS_IN_WORLD_X].room
			)
			result |= 2;

		//left
		if (
			in_screen.x == 0 ||
			THIS_ROOM != in_assets.screens[(in_screen.x - 1) + in_screen.y * NINJA_VC_SCREENS_IN_WORLD_X].room
			)
			result |= 4;

		//right
		if (
			in_screen.x == (NINJA_VC_SCREENS_IN_WORLD_X - 1) ||
			THIS_ROOM != in_assets.screens[(in_screen.x + 1) + in_screen.y * NINJA_VC_SCREENS_IN_WORLD_X].room
			)
			result |= 8;

		return result;
	}

	uint32_t vc_world_cambits(const c_vec2i_t& in_world, const vc_assets_t& in_assets)
	{
		return vc_screen_cambits({ in_world.x / (MLM_M_TILE_ASPECT * NINJA_VC_TILES_ON_SCREEN_X), in_world.y / (MLM_M_TILE_ASPECT * NINJA_VC_TILES_ON_SCREEN_Y) }, in_assets);
	}

#if 0
	void vc_draw_terminals(const vc_immutable_t& im, const c_vec2i_t& aCamera, const uint32_t anOffset, vc_canvas_t& canvas)
	{
		for (uint32_t i = 0; i < im.num_terminal; ++i)
		{
			if (im.terminal[i].offset == anOffset)
				__draw_terminal(aCamera, anOffset, vc_octamap_make(im.terminal[i].contents->bitmap), canvas);
		}
	}
#endif

#if 0
	void vc_draw_cursor(
		const uint32_t in_tick, const vc_assets_t& in_assets,
		micron_t& out_micron)
	{
		vc_octamap_blit_key_clip(
			out_micron.canvas_cursor_x - FS_TGA_HEADER(in_assets.cursor)->image_spec_width / 2,
			out_micron.canvas_cursor_y - FS_TGA_HEADER(in_assets.cursor)->image_spec_width / 2,
			FS_TGA_HEADER(in_assets.cursor)->image_spec_width,
			FS_TGA_HEADER(in_assets.cursor)->image_spec_width,
			0,
			FS_TGA_HEADER(in_assets.cursor)->image_spec_width * c_frame(in_tick * M_SECONDS_PER_TICK, 16.f, 3),
			in_assets.cursor,
			out_micron
		);
	}
#endif
}
