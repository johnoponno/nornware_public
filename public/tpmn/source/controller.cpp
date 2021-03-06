#include "stdafx.h"
#include "controller.h"

#include "../../dx9/app.h"
#include "../../dx9/State.h"
#include "../../dx9/sound_stream.h"
#include "../../dx9/input.h"
#include "assets.h"

namespace tpmn
{
#define ASSET_TRACK0 "johno_Jungle_2012.ogg"
#define ASSET_TRACK1 "johno_Minimal_2012.ogg"
#define ASSET_TRACK2 "johno_Outspaced_Remix_2012.ogg"
#define ASSET_TRACK3 "johno_Sunshower_2012.ogg"
#define ASSET_TRACK4 "johno_They_Took_Their_Planet_Back_2012.ogg"
#define ASSET_TRACK5 "johno_Sevens_2012.ogg"

#define SCROLL_ENABLE false

#define SLIDER_PREPARE_SPEED 50.f

#define PLANT_S_PASSIVE 0
#define PLANT_S_SUSPICIOUS 1
#define PLANT_S_POISED 2
#define PLANT_S_BITE 3
#define PLANT_POISE_DISTANCE 48.f
#define PLANT_SUSPECT_DISTANCE 72.f

#define FIREDUDE_FRAME_ASPECT 24

#define FLAKE_FRAME_ASPECT 67

#define HS_STAND 0
#define HS_RUN 1
#define HS_WHIP 2
#define HS_CHARM 3
#define HS_AIR 4

#define TILE_FPS 16.f

	static int32_t __hero_view_position_x(const model_t& model)
	{
		int32_t x = (int32_t)model.hero.x;

		//scrolling
		if (SCROLL_ENABLE)
		{
			x -= TILE_ASPECT * TILES_X / 2;
		}
		//fixed to screen boundaries
		else
		{
			x = x / (TILE_ASPECT * TILES_X);
			x *= TILE_ASPECT * TILES_X;
		}

		//bounds
		if (x < 0)
			x = 0;
		else if (x > (WORLD_WIDTH * TILE_ASPECT - CANVAS_WIDTH))
			x = WORLD_WIDTH * TILE_ASPECT - CANVAS_WIDTH;

		return x;
	}

	static int32_t __hero_view_position_y(const model_t& model)
	{
		int32_t y = (int32_t)model.hero.y;

		//scrolling
		if (SCROLL_ENABLE)
		{
			y -= TILE_ASPECT * TILES_Y / 2;
		}
		//fixed to screen boundaries
		else
		{
			y = y / (TILE_ASPECT * TILES_Y);
			y *= TILE_ASPECT * TILES_Y;
		}

		//bounds
		if (y < 0)
			y = 0;
		else if (y > (WORLD_HEIGHT * TILE_ASPECT - CANVAS_HEIGHT))
			y = WORLD_HEIGHT * TILE_ASPECT - CANVAS_HEIGHT;

		return y;
	}

	static void __draw_portals(const model_t& model, const assets_t& assets, const int32_t vx, const int32_t vy, softdraw::bitmap_t& canvas)
	{
		for (const portal_t* p = model.level.portals; p < model.level.portals + model.level.num_portals; ++p)
		{
			const int32_t x = (int32_t)offset_to_world_x(p->offset) - vx - assets.portal.width / 4;
			const int32_t y = (int32_t)offset_to_world_y(p->offset) - vy - assets.portal.height + TILE_ASPECT / 2;

			//frame based on accessability
			int32_t frame;
			if (model.hero.fixed_server.count < p->server_count)
				frame = 1;
			else
				frame = 0;

			bitmap_blit_key_clip(assets.portal, canvas, nullptr, x, y, TILE_ASPECT, assets.portal.height, frame * assets.portal.width / 2, 0);

			//if not accessible, draw number
			if (model.hero.fixed_server.count < p->server_count)
			{
				char str[4];
				::sprintf_s(str, "%u", p->server_count);
				if (p->server_count >= 10)
					fontv_print(assets.font, x + 1, y + 4, str, canvas);
				else
					fontv_print(assets.font, x + 6, y + 4, str, canvas);
			}
		}
	}

	static void __draw_servers(const model_t& model, const assets_t& assets, const int32_t vx, const int32_t vy, softdraw::bitmap_t& canvas)
	{
		//calc starting grid
		const int32_t gx = vx / TILE_ASPECT;
		const int32_t gy = vy / TILE_ASPECT;

		//calc subtile shift
		const int32_t sx = vx % TILE_ASPECT;
		const int32_t sy = vy % TILE_ASPECT;

		for (int32_t y = 0; y < TILES_Y; ++y)
		{
			for (int32_t x = 0; x < TILES_X; ++x)
			{
				if (model_get_tile(gx + x, gy + y, model).index == LOGIC_INDEX_SERVER)
				{
					//fixed state
					int32_t frame;
					int32_t ox;
					int32_t oy;
					const bool fixed = model_is_server_fixed(model.world, grid_to_offset(gx + x, gy + y), model.hero);
					if (fixed)
					{
						frame = 2;
						ox = oy = 0;
					}
					else
					{
						frame = int32_t(model_now(model) * 10) % 2;
						ox = int32_t(random_unit() * 2) - 1;
						oy = int32_t(random_unit() * 2) - 1;
					}

					bitmap_blit(
						assets.server,
						canvas,
						x * TILE_ASPECT - sx + ox,
						y * TILE_ASPECT - sy - TILE_ASPECT + oy,
						TILE_ASPECT,
						assets.server.height,
						frame * TILE_ASPECT,
						0);

					//arcs
					if (!fixed)
					{
						frame = int32_t(model_now(model) * 16) % 4;

						bitmap_blit_add(
							assets.arcs,
							canvas,
							x * TILE_ASPECT - sx - 4,
							y * TILE_ASPECT - sy - TILE_ASPECT - 4 + int32_t(random_unit() * TILE_ASPECT),
							32,
							32,
							0,
							32 * frame);
					}
				}
			}
		}
	}

	static void __draw_tile(const bool half, const assets_t& assets, const int32_t x, const int32_t y, const uint32_t tile, softdraw::bitmap_t& canvas)
	{
		const int32_t TILES_ON_SOURCE_X = assets.tiles.width / TILE_ASPECT;

		if (half)
		{
			bitmap_blit_half_key_clip(
				assets.tiles,
				canvas,
				nullptr,
				x, y,
				TILE_ASPECT, TILE_ASPECT,
				(tile % TILES_ON_SOURCE_X) * TILE_ASPECT,
				tile / TILES_ON_SOURCE_X * TILE_ASPECT);
		}
		else
		{
			bitmap_blit_key_clip(
				assets.tiles,
				canvas,
				nullptr,
				x, y,
				TILE_ASPECT, TILE_ASPECT,
				(tile % TILES_ON_SOURCE_X) * TILE_ASPECT,
				tile / TILES_ON_SOURCE_X * TILE_ASPECT);
		}
	}

	static uint32_t __plant_state(const hero_t& hero, const enemy_t& p)
	{
		const float DISTANCE = plant_hero_distance(p, hero);
		if (DISTANCE < PLANT_BITE_DISTANCE)
			return PLANT_S_BITE;

		if (DISTANCE < PLANT_POISE_DISTANCE)
			return PLANT_S_POISED;

		if (DISTANCE < PLANT_SUSPECT_DISTANCE)
			return PLANT_S_SUSPICIOUS;

		return PLANT_S_PASSIVE;
	}

	static void __draw_scorpion(const model_t& model, const enemy_t& plant, const assets_t& assets, const int32_t x, const int32_t y, softdraw::bitmap_t& canvas)
	{
		int32_t frame;
		switch (__plant_state(model.hero, plant))
		{
		default:
		case PLANT_S_PASSIVE:
			frame = 0;
			break;

		case PLANT_S_SUSPICIOUS:
			frame = 4;
			break;

		case PLANT_S_POISED:
			frame = 3;
			break;

		case PLANT_S_BITE:
			frame = 5;
			break;
		}

		//direction
		int32_t xOffs;
		int32_t srcX;
		if (model.hero.x > plant.x)
		{
			xOffs = -11;
			srcX = assets.scorpion.width / 2;
		}
		else
		{
			xOffs = -26;
			srcX = 0;
		}

		bitmap_blit_key_clip(
			assets.scorpion,
			canvas,
			nullptr,
			x + xOffs, y - 14,
			assets.scorpion.width / 2, assets.scorpion.height / 6,
			srcX,
			frame * assets.scorpion.height / 6);
	}

	static const char* __text(const uint32_t id, const uint32_t line)
	{
		switch (id)
		{
		case 0:
			switch (line)
			{
			case 0:		return "Welcome TP-Man,";
			case 1:		return "use ADK to get around!";
			}
			break;

		case 1:
			switch (line)
			{
			case 0:		return "Press S";
			case 1:		return "to pass through doors!";
			}
			break;

		case 2:
			switch (line)
			{
			case 0:		return "You need to JUMP(K) over these blocks!";
			}
			break;

		case 3:
			switch (line)
			{
			case 0:		return "Beware of enemies!";
			case 1:		return "You need to JUMP(K) on this kind to defeat it!";
			}
			break;

		case 4:
			switch (line)
			{
			case 0:		return "This is a broken server. You need to";
			case 1:		return "fix them to proceed through doors.";
			}
			break;

		case 5:
			switch (line)
			{
			case 0:		return "Some enemies must be whipped,";
			case 1:		return "use J!";
			}
			break;

		case 6:
			switch (line)
			{
			case 0:		return "Keys (of various colors)";
			case 1:		return "will open LOCKED BLOCKS!";
			}
			break;

		case 7:
			switch (line)
			{
			case 0:		return "Things will get more challenging";
			case 1:		return "the further you proceed!";
			}
			break;

		case 8:
			switch (line)
			{
			case 0:		return "This concludes the tutorial,";
			case 1:		return "good luck in your quest!";
			}
			break;
		}

		return nullptr;
	}

	static uint32_t __num_broken_servers(const model_t& model)
	{
		uint32_t count = 0;

		for (uint32_t o = 0; o < WORLD_SIZE; ++o)
		{
			if (model.level.tiles[o].index == LOGIC_INDEX_SERVER)
			{
				if (model_is_server_fixed(model.world, o, model.hero) == false)
					++count;
			}
		}

		return count;
	}

	static void __draw_dust_pass(const bool add, const float now, const float speed_x, const float speed_y, const softdraw::bitmap_t& bitmap, softdraw::bitmap_t& canvas)
	{
		if (dx9::key_is_down('P'))
			return;

		const int32_t X = int32_t(now * speed_x) % bitmap.width;
		const int32_t Y = int32_t(now * speed_y) % bitmap.height;
		if (add)
		{
			bitmap_blit_add_key_clip(bitmap, canvas, nullptr, -X, -Y);
			bitmap_blit_add_key_clip(bitmap, canvas, nullptr, bitmap.width - X, -Y);
			bitmap_blit_add_key_clip(bitmap, canvas, nullptr, -X, bitmap.height - Y);
			bitmap_blit_add_key_clip(bitmap, canvas, nullptr, bitmap.width - X, bitmap.height - Y);
		}
		else
		{
			bitmap_blit_half_key_clip(bitmap, canvas, nullptr, -X, -Y);
			bitmap_blit_half_key_clip(bitmap, canvas, nullptr, bitmap.width - X, -Y);
			bitmap_blit_half_key_clip(bitmap, canvas, nullptr, -X, bitmap.height - Y);
			bitmap_blit_half_key_clip(bitmap, canvas, nullptr, bitmap.width - X, bitmap.height - Y);
		}
	}

	static void __text(const assets_t& assets, const int dst_y, const char* string, const uint16_t color, softdraw::bitmap_t& canvas)
	{
		fontv_print_color_not_black(assets.font, color, (canvas.width - fontv_string_width(assets.font, string)) / 2, dst_y, string, canvas);
	}

	static void __draw_foreground(const model_t& model, const assets_t& assets, const int32_t vx, const int32_t vy, controller_t& controller, softdraw::bitmap_t& canvas)
	{
		const int32_t SX = screen_x((float)vx);
		const int32_t SY = screen_y((float)vy);

		const uint32_t index = model_screen(SX, SY, model);
		switch (index)
		{
		case 1:
			__draw_dust_pass(true, model_now(model), 400, 50, assets.dust_near, canvas);
			break;

		case 7:
		{
			if (controller.credits_start_time < 0.)
				controller.credits_start_time = model_now(model);

			int32_t y;

			__text(assets, y = CANVAS_HEIGHT + int32_t((model_now(model) - controller.credits_start_time) * -16.), "Congratulations!", TITLE_COLOR, canvas);
			__text(assets, y += TEXT_SPACING, "You made it to the end of the game!", TEXT_COLOR, canvas);
			__text(assets, y += TEXT_SPACING, "We wanted to put an epic boss fight", TEXT_COLOR, canvas);
			__text(assets, y += TEXT_SPACING, "here, but we couldn't quite", TEXT_COLOR, canvas);
			__text(assets, y += TEXT_SPACING, "find the time.", TEXT_COLOR, canvas);

			__text(assets, y += TEXT_SPACING * 2, "Feel free to explore the game", TEXT_COLOR, canvas);
			__text(assets, y += TEXT_SPACING, "world further, or play it again,", TEXT_COLOR, canvas);
			__text(assets, y += TEXT_SPACING, "or whatever you like!", TEXT_COLOR, canvas);

			__text(assets, y += TEXT_SPACING * 2, "If you're interested in playing", TEXT_COLOR, canvas);
			__text(assets, y += TEXT_SPACING, "with the level editor,", TEXT_COLOR, canvas);
			__text(assets, y += TEXT_SPACING, "or playing with the code base,", TEXT_COLOR, canvas);
			__text(assets, y += TEXT_SPACING, "or have any other questions", TEXT_COLOR, canvas);
			__text(assets, y += TEXT_SPACING, "or comments, please contact:", TEXT_COLOR, canvas);

			__text(assets, y += TEXT_SPACING, "johannes.norneby@gmail.com", TITLE_COLOR, canvas);

			__text(assets, y += TEXT_SPACING * 2, "Thanks for playing!", TEXT_COLOR, canvas);

			__text(assets, y += TEXT_SPACING * 4, "TP-Man Nightmare", TITLE_COLOR, canvas);
			__text(assets, y += TEXT_SPACING, "(c)2012 NTI-Gymnasiet Goteborg", TEXT_COLOR, canvas);

			__text(assets, y += TEXT_SPACING * 2, "Art / Design", TITLE_COLOR, canvas);
			__text(assets, y += TEXT_SPACING, "Saga Velander", TEXT_COLOR, canvas);
			__text(assets, y += TEXT_SPACING, "Michael Awakim Manaz", TEXT_COLOR, canvas);

			__text(assets, y += TEXT_SPACING * 2, "Code / Music / Sound", TITLE_COLOR, canvas);
			__text(assets, y += TEXT_SPACING, "Johannes 'johno' Norneby", TEXT_COLOR, canvas);

			if (y < -32)
				controller.credits_start_time = model_now(model);
		}
		break;
		}
	}

	static void __draw_farplane(const model_t& model, const assets_t& assets, const int32_t vx, const int32_t vy, controller_t& controller)
	{
		const float now = model_now(model);

		const int32_t SX = screen_x((float)vx);
		const int32_t SY = screen_y((float)vy);

		uint32_t index = model_screen(SX, SY, model);

		//this mirrors how the flash version worked; there was indirection going in in a single case
		if (7 == index)
			index = 2;

		assert(index < _countof(assets.myBackgrounds));

		switch (index)
		{
		default:
			bitmap_blit(assets.myBackgrounds[index], controller.canvas, 0, 0);
			break;

		case 1:
			bitmap_blit(assets.myBackgrounds[index], controller.canvas, 0, 0);
			__draw_dust_pass(false, now, 100, 20, assets.dust_far, controller.canvas);
			break;

		case 3:
			bitmap_blit(assets.myBackgrounds[index], controller.canvas, 0, 0);
			for (snowflake_t* f = controller.flakes; f < controller.flakes + _countof(controller.flakes); ++f)
			{
				f->x += f->sx * TIME_PER_TICK;
				f->x += ::sinf(f->t + now * 5) * 16 * TIME_PER_TICK;
				f->y += f->sy * TIME_PER_TICK;

				if (f->x < -FLAKE_FRAME_ASPECT)
				{
					f->x = CANVAS_WIDTH;
				}
				else if (f->x >= CANVAS_WIDTH)
				{
					f->x = -FLAKE_FRAME_ASPECT;
				}
				if (f->y >= CANVAS_HEIGHT)
				{
					f->y = -FLAKE_FRAME_ASPECT;
				}

				bitmap_blit_half_key_clip(assets.flake, controller.canvas, nullptr, (int32_t)f->x, (int32_t)f->y, FLAKE_FRAME_ASPECT, FLAKE_FRAME_ASPECT, (f->t % 3) * FLAKE_FRAME_ASPECT, f->t / 3 * FLAKE_FRAME_ASPECT);
			}
			break;

		case 5:
			bitmap_blit(assets.myBackgrounds[index], controller.canvas, 0, 0);
#if 0
			{
				const int32_t WIDTH = assets.myCloud.width / 3;
				const int32_t HEIGHT = assets.myCloud.height / 2;

				for (cloud_t* c = controller.clouds; c < controller.clouds + _countof(controller.clouds); ++c)
				{
					c->x += c->s * TIME_PER_TICK;
					if (c->x < -(assets.myCloud.width / 3))
						c->x = CANVAS_WIDTH;
					bitmap_blit_add_key_clip(assets.myCloud, controller.canvas, nullptr, (int32_t)c->x, (int32_t)c->y, WIDTH, HEIGHT, (c->t % 3) * WIDTH, c->t / 3 * HEIGHT);
				}
			}
#endif
			break;
		}
	}

	static void __draw_hero(const model_t& model, const assets_t& assets, const int32_t vx, const int32_t vy, softdraw::bitmap_t& canvas)
	{
		//figure out state / frame
		int32_t state;
		int32_t frame;
		if (hero_whipping(model_now(model), model.hero))
		{
			state = HS_WHIP;
			frame = 0;
		}
		else
		{
			if (model.hero.air_bit)
			{
				state = HS_AIR;
				frame = 0;
			}
			else if (hero_left_input(model.hero) || hero_right_input(model.hero))
			{
				state = HS_RUN;
				frame = int(model_now(model) * 15) % 2;
			}
			else
			{
				state = HS_STAND;
				frame = 0;
			}
		}

		//blit
		bitmap_blit_key_clip(
			assets.hero,
			canvas,
			nullptr,
			int32_t(model.hero.x - vx - HERO_WIDTH / 2), int32_t(model.hero.y - vy - HERO_HEIGHT / 2),
			HERO_WIDTH, HERO_HEIGHT,
			(frame + (model.hero.right_bit ? 1 : 0) * 2) * HERO_WIDTH, state * HERO_HEIGHT);

		//whip
		if (state == HS_WHIP)
		{
			bitmap_blit_key_clip(
				assets.whip,
				canvas,
				nullptr,
				int32_t(hero_whip_min_x(model.hero) - vx),
				int32_t(hero_whip_y(model.hero) - vy - 22),
				assets.whip.width / 2,
				assets.whip.height / 2,
				model.hero.right_bit ? assets.whip.width / 2 : 0,
				hero_whip_extended(model_now(model), model.hero) ? assets.whip.height / 2 : 0);
		}
	}

	static void __draw_tiles(const model_t& model, const assets_t& assets, const int32_t vx, const int32_t vy, const bool replace, controller_t& controller)
	{
		//calc starting grid
		const int32_t gx = vx / TILE_ASPECT;
		const int32_t gy = vy / TILE_ASPECT;

		//calc subtile shift
		const int32_t sx = vx % TILE_ASPECT;
		const int32_t sy = vy % TILE_ASPECT;

		for (int32_t y = 0; y < TILES_Y; ++y)
		{
			for (int32_t x = 0; x < TILES_X; ++x)
			{
				const tile_t& tile = model_get_tile(gx + x, gy + y, model);
				uint32_t index = tile.index;

				if (replace)
				{
					switch (index)
					{
					case LOGIC_INDEX_CHECKPOINT:
						if (hero_is_checkpoint(gx + x, gy + y, model.hero))
							index = LOGIC_INDEX_CHECKPOINT_CURRENT;
						break;

					case LOGIC_INDEX_SPIKYGREEN:
					case LOGIC_INDEX_ICICLE:
					case LOGIC_INDEX_SANDBLOCK:
					case LOGIC_INDEX_BLUEBLOB:
					case LOGIC_INDEX_BROWNBLOB:
					case LOGIC_INDEX_PLANT:
					case LOGIC_INDEX_FIREDUDE:
					case LOGIC_INDEX_SERVER:
					case LOGIC_INDEX_BAT:
					case LOGIC_INDEX_PENGUIN:
					case LOGIC_INDEX_SCORPION:
						index = LOGIC_INDEX_AIR;
						break;

					case LOGIC_INDEX_KEY0:
					case LOGIC_INDEX_KEY0BLOCK:
						if (model.hero.keys & HERO_KEY0)
							index = LOGIC_INDEX_AIR;
						break;

					case LOGIC_INDEX_KEY1:
					case LOGIC_INDEX_KEY1BLOCK:
						if (model.hero.keys & HERO_KEY1)
							index = LOGIC_INDEX_AIR;
						break;

					case LOGIC_INDEX_KEY2:
					case LOGIC_INDEX_KEY2BLOCK:
						if (model.hero.keys & HERO_KEY2)
							index = LOGIC_INDEX_AIR;
						break;
					}
				}

				//animated
				assert(index < MAX_TILE);
				__draw_tile(false, assets, x * TILE_ASPECT - sx, y * TILE_ASPECT - sy, controller.current_tiles[index], controller.canvas);
			}
		}
	}

	static void __draw_deaths(const int32_t vx, const int32_t vy, controller_t& controller, softdraw::bitmap_t& canvas)
	{
		death_t* d = controller.deaths;
		while (d < controller.deaths + controller.num_deaths)
		{
			d->s += GRAVITY * TIME_PER_TICK;

			d->y += d->s * TIME_PER_TICK;

			const int32_t SX = int32_t(d->x - vx);
			const int32_t SY = int32_t(d->y - vy);

			if (SX >= (-d->w / 2) && SY >= (-d->h / 2) && SX < (CANVAS_WIDTH + d->w / 2) && SY < (CANVAS_HEIGHT + d->h / 2))
			{
				bitmap_blit_key_clip(
					*d->bm,
					canvas,
					nullptr,
					int32_t(d->x - vx - d->w / 2),
					int32_t(d->y - vy - d->h / 2),
					d->w,
					d->h,
					d->src_x,
					d->src_y);

				//advance iterator
				++d;
			}
			else
			{
				//copy back last, don't advance iterator
				*d = controller.deaths[controller.num_deaths - 1];
				--controller.num_deaths;
			}
		}
	}

	static void __draw_enemies(const model_t& model, const assets_t& assets, const int32_t vx, const int32_t vy, softdraw::bitmap_t& canvas)
	{
		const float now = model_now(model);
		const int32_t spikygreen_num_frames = assets.spikygreen.height / assets.spikygreen.width;
		const int32_t spikygreen_frame = int32_t(now * 12) % spikygreen_num_frames;
		const int32_t firedude_frame = int32_t(now * 10) % (assets.firedude.height / FIREDUDE_FRAME_ASPECT);
		const int32_t bat_frame_height = assets.bat.height / 6;
		const int32_t blueblob_frame = int32_t(now * 10) % (assets.blueblob.height / BLOB_FRAME_ASPECT);
		const int32_t brownblob_frame = int32_t(now * 10) % (assets.brownblob.height / BLOB_FRAME_ASPECT);
		int32_t x_offset;
		int32_t src_x;

		for (const enemy_t* e = model.enemy; e < model.enemy + model.num_enemy; ++e)
		{
			switch (e->type)
			{
			default:
				assert(0);
				break;

			case LOGIC_INDEX_SPIKYGREEN:
				if (now > e->spawn_time)
				{
					bitmap_blit_key_clip(assets.spikygreen, canvas, nullptr, int32_t(e->x - vx - assets.spikygreen.width / 2), int32_t(e->y - vy - assets.spikygreen.width / 2), assets.spikygreen.width, assets.spikygreen.width, 0,
						(e->speed > 0 ? spikygreen_num_frames - 1 - spikygreen_frame : spikygreen_frame) * assets.spikygreen.width);
				}
				break;

			case LOGIC_INDEX_BLUEBLOB:
				if (now > e->spawn_time)
					bitmap_blit_key_clip(assets.blueblob, canvas, nullptr, int32_t(e->x - vx - BLOB_FRAME_ASPECT / 2), int32_t(e->y - vy - BLOB_FRAME_ASPECT / 2), BLOB_FRAME_ASPECT, BLOB_FRAME_ASPECT, (e->speed > 0 ? 1 : 0) * BLOB_FRAME_ASPECT, blueblob_frame * BLOB_FRAME_ASPECT);
				break;

			case LOGIC_INDEX_BROWNBLOB:
				if (now > e->spawn_time)
					bitmap_blit_key_clip(assets.brownblob, canvas, nullptr, int32_t(e->x - vx - BLOB_FRAME_ASPECT / 2), int32_t(e->y - vy - BLOB_FRAME_ASPECT / 2), BLOB_FRAME_ASPECT, BLOB_FRAME_ASPECT, (e->speed > 0 ? 1 : 0) * BLOB_FRAME_ASPECT, brownblob_frame * BLOB_FRAME_ASPECT);
				break;

			case LOGIC_INDEX_PLANT:
				if (now > e->spawn_time)
				{
					int32_t frame;
					switch (__plant_state(model.hero, *e))
					{
					default:
					case PLANT_S_PASSIVE:
						frame = 0;
						break;

					case PLANT_S_SUSPICIOUS:
						frame = 2;
						break;

					case PLANT_S_POISED:
						frame = 1;
						break;

					case PLANT_S_BITE:
						frame = 3;
						break;
					}

					if (model.hero.x > e->x)
					{
						x_offset = -9;
						src_x = PLANT_FRAME_ASPECT;
					}
					else
					{
						x_offset = -22;
						src_x = 0;
					}
					bitmap_blit_key_clip(assets.plant, canvas, nullptr, int32_t(e->x - vx) + x_offset, int32_t(e->y - vy) - 20, PLANT_FRAME_ASPECT, PLANT_FRAME_ASPECT, src_x, frame * PLANT_FRAME_ASPECT);
				}
				break;

			case LOGIC_INDEX_SCORPION:
				if (now > e->spawn_time)
					__draw_scorpion(model, *e, assets, int32_t(e->x - vx), int32_t(e->y - vy), canvas);
				break;

			case LOGIC_INDEX_PENGUIN:
				if (now > e->spawn_time)
				{
					bitmap_blit_key_clip(assets.penguin, canvas, nullptr,
						int32_t(e->x - vx - assets.penguin.width / 4), int32_t(e->y - vy + 2),
						assets.penguin.width / 2, assets.penguin.height / 2,
						(e->speed > 0 ? 1 : 0) * assets.penguin.width / 2, (::fabsf(e->speed) < SLIDER_PREPARE_SPEED ? 1 : 0) * assets.penguin.height / 2);
				}
				break;

			case LOGIC_INDEX_FIREDUDE:
				bitmap_blit_key_clip(assets.firedude, canvas, nullptr, int32_t(e->x - vx - FIREDUDE_FRAME_ASPECT / 2), int32_t(e->y - vy - FIREDUDE_FRAME_ASPECT / 2), FIREDUDE_FRAME_ASPECT, FIREDUDE_FRAME_ASPECT, (e->vector_x > 0 ? 1 : 0) * FIREDUDE_FRAME_ASPECT, firedude_frame * FIREDUDE_FRAME_ASPECT);
				break;

			case LOGIC_INDEX_BAT:
			{
				int32_t frame;
				if (e->state == BAT_S_PASSIVE)
					frame = 0;
				else
					frame = 1 + int32_t(now * 16) % 2;

				if (e->scared)
					frame += 3;

				bitmap_blit_key_clip(assets.bat, canvas, nullptr, int32_t(e->x - vx - assets.bat.width / 2), int32_t(e->y - vy - bat_frame_height / 2), assets.bat.width, bat_frame_height, 0, frame * bat_frame_height);
			}
			break;

			case LOGIC_INDEX_ICICLE:
			case LOGIC_INDEX_SANDBLOCK:
				if (now > e->spawn_time)
				{
					switch (e->state)
					{
					default:
						__draw_tile(false, assets, int32_t(e->x - vx - TILE_ASPECT / 2), int32_t(e->y - vy - TILE_ASPECT / 2), e->type, canvas);
						break;

					case ICICLE_S_PENDING:
						__draw_tile(false, assets, int32_t(e->x - vx - TILE_ASPECT / 2 + random_unit() * 2 - 1), int32_t(e->y - vy - TILE_ASPECT / 2 + random_unit() * 2 - 1), e->type, canvas);
						break;
					}
				}
				break;
			}
		}
	}

	static void __play_update(const assets_t& assets, model_t& model, controller_t& c)
	{
		//play menu up?
		if (c.play_menu)
		{
			__text(assets, CANVAS_HEIGHT / 3, "ESC = Quit", softdraw::white, c.canvas);
			if (dx9::key_up_flank(VK_ESCAPE))
			{
				model.play_bit = 0;

				c.play_menu = false;
			}

			__text(assets, CANVAS_HEIGHT / 3 * 2, "R = Resume", softdraw::white, c.canvas);
			if (dx9::key_up_flank('R'))
			{
				c.play_menu = false;
			}
		}
		//play menu not up
		else
		{
			if (dx9::key_up_flank(VK_ESCAPE))
			{
				c.play_menu = true;
			}
			else
			{
				uint32_t input = 0;

				//hero movement
				if (model.hero.spawn_time < 0.f)
				{
					if (dx9::key_down_flank('S'))
						input |= HERO_FLAGS_DOWN;

					if (dx9::key_is_down('A'))
						input |= HERO_FLAGS_LEFT;

					if (dx9::key_is_down('D'))
						input |= HERO_FLAGS_RIGHT;

					if (dx9::key_is_down('K'))
						input |= HERO_FLAGS_JUMP;

					if (dx9::key_down_flank('J'))
						input |= HERO_FLAGS_WHIP;
				}

				//always set
				model.hero.input = input;
			}
		}

		//doOutput();
		{
			const int32_t VPX = __hero_view_position_x(model);
			const int32_t VPY = __hero_view_position_y(model);
			const float now = model_now(model);

			__draw_farplane(model, assets, VPX, VPY, c);
			__draw_tiles(model, assets, VPX, VPY, true, c);
			__draw_portals(model, assets, VPX, VPY, c.canvas);
			__draw_servers(model, assets, VPX, VPY, c.canvas);
			__draw_enemies(model, assets, VPX, VPY, c.canvas);
			__draw_deaths(VPX, VPY, c, c.canvas);

			if (model.hero.spawn_time < 0.f)
				__draw_hero(model, assets, VPX, VPY, c.canvas);

			__draw_foreground(model, assets, VPX, VPY, c, c.canvas);

			//drawInfos();
			{
				const info_t* info = model_info_for_offset(world_to_offset(model.hero.x, model.hero.y), model);
				const char* s;

				if (info)
				{
					s = __text(info->id, 0);
					if (s)
						__text(assets, 0, s, 0xffff, c.canvas);

					s = __text(info->id, 1);
					if (s)
						__text(assets, 24, s, 0xffff, c.canvas);
				}
			}

			//drawGui();
			{
				//total servers fixed
				bitmap_blit_key(assets.gui_server_fixed, c.canvas, 0, 0);
				{
					char str[4];
					::sprintf_s(str, "%u", model.hero.fixed_server.count);
					if (model.hero.fixed_server.count < 10)
						fontv_print(assets.font, 6, 6, str, c.canvas);
					else
						fontv_print(assets.font, 1, 6, str, c.canvas);
				}

				//keys gui
				int32_t x = TILE_ASPECT;
				if (model.hero.keys & HERO_KEY0)
					__draw_tile(false, assets, x, 0, LOGIC_INDEX_KEY0, c.canvas);
				if (model.hero.keys & HERO_KEY1)
					__draw_tile(false, assets, x += TILE_ASPECT, 0, LOGIC_INDEX_KEY1, c.canvas);
				if (model.hero.keys & HERO_KEY2)
					__draw_tile(false, assets, x += TILE_ASPECT, 0, LOGIC_INDEX_KEY2, c.canvas);

				//servers to fix (in current level)
				const uint32_t ns = __num_broken_servers(model);
				for (uint32_t i = 0; i < ns; ++i)
					bitmap_blit_key(assets.gui_server_broken, c.canvas, CANVAS_WIDTH - i * 14 - 20, 0);
			}

			if (c.play_menu)
			{
				__text(assets, CANVAS_HEIGHT / 3, "ESC = Quit", softdraw::white, c.canvas);
				__text(assets, CANVAS_HEIGHT / 3 * 2, "R = Resume", softdraw::white, c.canvas);
			}
		}
	}

	static app_event_t __idle_update(const assets_t& assets, controller_t& controller)
	{
#if 0
		bitmap_blit(assets.myBackgrounds[1], controller.canvas, 0, 0);
		__draw_dust_pass(false, model_now(controller._remove_model), 100, 20, assets.dust_far, controller.canvas);
		__draw_dust_pass(true, model_now(controller._remove_model), 400, 50, assets.dust_near, controller.canvas);
		return app_event_t::nothing;
#else
		bitmap_blit(assets.idle, controller.canvas, 0, 0);

		{
			int32_t y;

			__text(assets, y = 8, "TP-Man Nightmare", TITLE_COLOR, controller.canvas);
			__text(assets, y += TEXT_SPACING, "(c)2012-2019 nornware AB", TEXT_COLOR, controller.canvas);
			__text(assets, y += TEXT_SPACING * 2, "Talent", TITLE_COLOR, controller.canvas);
			__text(assets, y += TEXT_SPACING, "Saga Velander", TEXT_COLOR, controller.canvas);
			__text(assets, y += TEXT_SPACING, "Michael Awakim Manaz", TEXT_COLOR, controller.canvas);
			__text(assets, y += TEXT_SPACING, "Johannes 'johno' Norneby", TEXT_COLOR, controller.canvas);

			__text(assets, y += TEXT_SPACING * 4, "P = Play", softdraw::green, controller.canvas);
			if (dx9::key_up_flank('P'))
				return app_event_t::start_new_game;

			__text(assets, y += TEXT_SPACING, "ESC = Quit", softdraw::green, controller.canvas);
			if (dx9::key_up_flank(VK_ESCAPE))
				return app_event_t::exit_application;
		}

		{
			const dx9::cursor_position_t cp = dx9::mouse_cursor_position();
			bitmap_cross(controller.canvas, cp.x, cp.y, 3, 0xffff);
		}

		return app_event_t::nothing;
#endif
	}

	//public
	//public
	//public
	//public
	app_event_t controller_input_output(const assets_t& assets, model_t& model, controller_t& controller)
	{
		//music
		if (controller.track != model.level.music_track || !controller.music)
		{
			if (controller.music)
			{
				delete controller.music;
				controller.music = nullptr;
			}

			switch (model.level.music_track)
			{
			default:	controller.music = stream_create(assets.engine, ASSET_TRACK0);	break;
			case 1:		controller.music = stream_create(assets.engine, ASSET_TRACK1);	break;
			case 2:		controller.music = stream_create(assets.engine, ASSET_TRACK2);	break;
			case 3:		controller.music = stream_create(assets.engine, ASSET_TRACK3);	break;
			case 4:		controller.music = stream_create(assets.engine, ASSET_TRACK4);	break;
			case 5:		controller.music = stream_create(assets.engine, ASSET_TRACK5);	break;
			}
			if (controller.music)
				stream_play(true, 0.f, 1.f, *controller.music);

			controller.track = model.level.music_track;
		}
		if (controller.music)
			stream_update(model_now(model), 1.f, *controller.music);

		//tile animations
		{
			controller.tile_anim_tick += TILE_FPS * TIME_PER_TICK;
			while (controller.tile_anim_tick > 1)
			{
				for (uint32_t t = 0; t < MAX_TILE; ++t)
				{
					const uint32_t tile = controller.current_tiles[t];
					controller.current_tiles[t] = assets.anim_target[tile];
				}

				controller.tile_anim_tick -= 1;
			}
		}

		//this is single pass IMGUI, so input and output are intertwined
		app_event_t result = app_event_t::nothing;
		if (model.play_bit)
			__play_update(assets, model, controller);
		else
			result = __idle_update(assets, controller);

#if 0
		//display the number of dropped frames (60hz)
		{
			char str[16];
			::sprintf_s(str, "%u", dx9::state.app->_frame_drops);
			__text(assets, CANVAS_HEIGHT - assets.font.height, str, softdraw::red, controller.canvas);
		}
#endif

		return result;
	}

	void controller_on_load_new_world(const assets_t& assets, controller_t& controller)
	{
		sound_play(assets, snd_spawn);

		controller.play_menu = false;

		for (cloud_t* c = controller.clouds; c < controller.clouds + _countof(controller.clouds); ++c)
		{
			c->x = random_unit() * CANVAS_WIDTH;
			c->y = random_unit() * CANVAS_HEIGHT / 2;
			c->s = random_unit() * -20 - 10;
			c->t = uint32_t(random_unit() * 6);
		}

		for (snowflake_t* f = controller.flakes; f < controller.flakes + _countof(controller.flakes); ++f)
		{
			f->x = random_unit() * CANVAS_WIDTH;
			f->y = random_unit() * CANVAS_HEIGHT;
			f->sx = random_unit() * 50 - 25;
			f->sy = random_unit() * 75 + 25;
			f->t = uint32_t(random_unit() * 6);
		}
	}

	void controller_death_create(const softdraw::bitmap_t* aBitmap, const float aX, const float aY, const int32_t aWidth, const int32_t aHeight, const int32_t aSrcX, const int32_t aSrcY, controller_t& controller)
	{
		assert(controller.num_deaths < _countof(controller.deaths));
		death_t* d = controller.deaths + controller.num_deaths++;
		d->bm = aBitmap;
		d->x = aX;
		d->y = aY;
		d->s = -350;
		d->w = aWidth;
		d->h = aHeight;
		d->src_x = aSrcX;
		d->src_y = aSrcY;
	}
}
