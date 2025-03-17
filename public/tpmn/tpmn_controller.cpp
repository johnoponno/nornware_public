#include "stdafx.h"
#include "tpmn_controller.h"

#include "../micron.h"
#include "../microlib/microlib.h"
#include "tpmn_assets.h"

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

static int32_t __hero_view_position_x(const tpmn_model_t& model)
{
	int32_t x = (int32_t)model.hero.x;

	//scrolling
	if (SCROLL_ENABLE)
	{
		x -= TPMN_TILE_ASPECT * TPMN_TILES_X / 2;
	}
	//fixed to screen boundaries
	else
	{
		x = x / (TPMN_TILE_ASPECT * TPMN_TILES_X);
		x *= TPMN_TILE_ASPECT * TPMN_TILES_X;
	}

	//bounds
	if (x < 0)
		x = 0;
	else if (x > (TPMN_WORLD_WIDTH * TPMN_TILE_ASPECT - TPMN_CANVAS_WIDTH))
		x = TPMN_WORLD_WIDTH * TPMN_TILE_ASPECT - TPMN_CANVAS_WIDTH;

	return x;
}

static int32_t __hero_view_position_y(const tpmn_model_t& model)
{
	int32_t y = (int32_t)model.hero.y;

	//scrolling
	if (SCROLL_ENABLE)
	{
		y -= TPMN_TILE_ASPECT * TPMN_TILES_Y / 2;
	}
	//fixed to screen boundaries
	else
	{
		y = y / (TPMN_TILE_ASPECT * TPMN_TILES_Y);
		y *= TPMN_TILE_ASPECT * TPMN_TILES_Y;
	}

	//bounds
	if (y < 0)
		y = 0;
	else if (y > (TPMN_WORLD_HEIGHT * TPMN_TILE_ASPECT - TPMN_CANVAS_HEIGHT))
		y = TPMN_WORLD_HEIGHT * TPMN_TILE_ASPECT - TPMN_CANVAS_HEIGHT;

	return y;
}

static void __draw_portals(
	const tpmn_model_t& model, const tpmn_assets_t& assets, const int32_t vx, const int32_t vy,
	sd_bitmap_t& canvas)
{
	for (const tpmn_portal_t* p = model.level.portals; p < model.level.portals + model.level.num_portals; ++p)
	{
		const int32_t x = (int32_t)tpmn_offset_to_world_x(p->offset) - vx - assets.portal.width / 4;
		const int32_t y = (int32_t)tpmn_offset_to_world_y(p->offset) - vy - assets.portal.height + TPMN_TILE_ASPECT / 2;

		//frame based on accessability
		int32_t frame;
		if (model.hero.fixed_servers.count < p->server_count)
			frame = 1;
		else
			frame = 0;

		sd_bitmap_blit_key_clip(assets.portal, canvas, nullptr, x, y, TPMN_TILE_ASPECT, assets.portal.height, frame * assets.portal.width / 2, 0);

		//if not accessible, draw number
		if (model.hero.fixed_servers.count < p->server_count)
		{
			char str[4];
			::sprintf_s(str, "%u", p->server_count);
			if (p->server_count >= 10)
				sd_fontv_print(assets.font, x + 1, y + 4, str, canvas);
			else
				sd_fontv_print(assets.font, x + 6, y + 4, str, canvas);
		}
	}
}

static void __draw_servers(
	const tpmn_model_t& model, const tpmn_assets_t& assets, const int32_t vx, const int32_t vy,
	sd_bitmap_t& canvas)
{
	//calc starting grid
	const int32_t gx = vx / TPMN_TILE_ASPECT;
	const int32_t gy = vy / TPMN_TILE_ASPECT;

	//calc subtile shift
	const int32_t sx = vx % TPMN_TILE_ASPECT;
	const int32_t sy = vy % TPMN_TILE_ASPECT;

	for (int32_t y = 0; y < TPMN_TILES_Y; ++y)
	{
		for (int32_t x = 0; x < TPMN_TILES_X; ++x)
		{
			if (tpmn_model_get_tile(gx + x, gy + y, model).index == TPMN_LOGIC_INDEX_SERVER)
			{
				//fixed state
				int32_t frame;
				int32_t ox;
				int32_t oy;
				const bool fixed = model_is_server_fixed(model.world, tpmn_grid_to_offset(gx + x, gy + y), model.hero);
				if (fixed)
				{
					frame = 2;
					ox = oy = 0;
				}
				else
				{
					frame = int32_t(tpmn_model_now(model) * 10) % 2;
					ox = int32_t(tpmn_random_unit() * 2) - 1;
					oy = int32_t(tpmn_random_unit() * 2) - 1;
				}

				sd_bitmap_blit(
					assets.server,
					canvas,
					x * TPMN_TILE_ASPECT - sx + ox,
					y * TPMN_TILE_ASPECT - sy - TPMN_TILE_ASPECT + oy,
					TPMN_TILE_ASPECT,
					assets.server.height,
					frame * TPMN_TILE_ASPECT,
					0
				);

				//arcs
				if (!fixed)
				{
					frame = int32_t(tpmn_model_now(model) * 16) % 4;

					sd_bitmap_blit_add(
						assets.arcs,
						canvas,
						x * TPMN_TILE_ASPECT - sx - 4,
						y * TPMN_TILE_ASPECT - sy - TPMN_TILE_ASPECT - 4 + int32_t(tpmn_random_unit() * TPMN_TILE_ASPECT),
						32,
						32,
						0,
						32 * frame
					);
				}
			}
		}
	}
}

static void __draw_tile(
	const bool half, const tpmn_assets_t& assets, const int32_t x, const int32_t y, const uint32_t tile,
	sd_bitmap_t& canvas)
{
	const int32_t TILES_ON_SOURCE_X = assets.tiles.width / TPMN_TILE_ASPECT;

	if (half)
	{
		sd_bitmap_blit_half_key_clip(
			assets.tiles,
			canvas,
			nullptr,
			x, y,
			TPMN_TILE_ASPECT, TPMN_TILE_ASPECT,
			(tile % TILES_ON_SOURCE_X) * TPMN_TILE_ASPECT,
			tile / TILES_ON_SOURCE_X * TPMN_TILE_ASPECT
		);
	}
	else
	{
		sd_bitmap_blit_key_clip(
			assets.tiles,
			canvas,
			nullptr,
			x, y,
			TPMN_TILE_ASPECT, TPMN_TILE_ASPECT,
			(tile % TILES_ON_SOURCE_X) * TPMN_TILE_ASPECT,
			tile / TILES_ON_SOURCE_X * TPMN_TILE_ASPECT
		);
	}
}

static uint32_t __plant_state(const tpmn_hero_t& hero, const tpmn_enemy_t& p)
{
	const float DISTANCE = tpmn_plant_hero_distance(p, hero);
	if (DISTANCE < TPMN_PLANT_BITE_DISTANCE)
		return PLANT_S_BITE;

	if (DISTANCE < PLANT_POISE_DISTANCE)
		return PLANT_S_POISED;

	if (DISTANCE < PLANT_SUSPECT_DISTANCE)
		return PLANT_S_SUSPICIOUS;

	return PLANT_S_PASSIVE;
}

static void __draw_scorpion(
	const tpmn_model_t& model, const tpmn_enemy_t& plant, const tpmn_assets_t& assets, const int32_t x, const int32_t y,
	sd_bitmap_t& canvas)
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

	sd_bitmap_blit_key_clip(
		assets.scorpion,
		canvas,
		nullptr,
		x + xOffs, y - 14,
		assets.scorpion.width / 2, assets.scorpion.height / 6,
		srcX,
		frame * assets.scorpion.height / 6
	);
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

static uint32_t __num_broken_servers(const tpmn_model_t& model)
{
	uint32_t count = 0;

	for (uint32_t o = 0; o < TPMN_WORLD_SIZE; ++o)
	{
		if (model.level.tiles[o].index == TPMN_LOGIC_INDEX_SERVER)
		{
			if (model_is_server_fixed(model.world, o, model.hero) == false)
				++count;
		}
	}

	return count;
}

static void __draw_dust_pass(
	const micron_t& in_micron, const bool add, const float now, const float speed_x, const float speed_y, const sd_bitmap_t& bitmap,
	sd_bitmap_t& canvas)
{
	if (micron_key_is_down(in_micron, 'P'))
		return;

	const int32_t X = int32_t(now * speed_x) % bitmap.width;
	const int32_t Y = int32_t(now * speed_y) % bitmap.height;
	if (add)
	{
		sd_bitmap_blit_add_key_clip(bitmap, canvas, nullptr, -X, -Y);
		sd_bitmap_blit_add_key_clip(bitmap, canvas, nullptr, bitmap.width - X, -Y);
		sd_bitmap_blit_add_key_clip(bitmap, canvas, nullptr, -X, bitmap.height - Y);
		sd_bitmap_blit_add_key_clip(bitmap, canvas, nullptr, bitmap.width - X, bitmap.height - Y);
	}
	else
	{
		sd_bitmap_blit_half_key_clip(bitmap, canvas, nullptr, -X, -Y);
		sd_bitmap_blit_half_key_clip(bitmap, canvas, nullptr, bitmap.width - X, -Y);
		sd_bitmap_blit_half_key_clip(bitmap, canvas, nullptr, -X, bitmap.height - Y);
		sd_bitmap_blit_half_key_clip(bitmap, canvas, nullptr, bitmap.width - X, bitmap.height - Y);
	}
}

static void __text(
	const tpmn_assets_t& assets, const int dst_y, const char* string, const uint16_t color,
	sd_bitmap_t& canvas)
{
	sd_fontv_print_color_not_black(assets.font, color, (canvas.width - sd_fontv_string_width(assets.font, string)) / 2, dst_y, string, canvas);
}

static void __draw_foreground(
	const micron_t& in_micron, const tpmn_model_t& model, const tpmn_assets_t& assets, const int32_t vx, const int32_t vy,
	tpmn_controller_t& controller)
{
	const int32_t SX = tpmn_screen_x((float)vx);
	const int32_t SY = tpmn_screen_y((float)vy);

	const uint32_t index = model_screen(SX, SY, model);
	switch (index)
	{
	case 1:
		__draw_dust_pass(in_micron, true, tpmn_model_now(model), 400, 50, assets.dust_near, controller.canvas);
		break;

	case 7:
	{
		if (controller.credits_start_time < 0.)
			controller.credits_start_time = tpmn_model_now(model);

		int32_t y;

		__text(assets, y = TPMN_CANVAS_HEIGHT + int32_t((tpmn_model_now(model) - controller.credits_start_time) * -16.), "Congratulations!", TPMN_TITLE_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "You made it to the end of the game!", TPMN_TEXT_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "We wanted to put an epic boss fight", TPMN_TEXT_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "here, but we couldn't quite", TPMN_TEXT_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "find the time.", TPMN_TEXT_COLOR, controller.canvas);

		__text(assets, y += TPMN_TEXT_SPACING * 2, "Feel free to explore the game", TPMN_TEXT_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "world further, or play it again,", TPMN_TEXT_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "or whatever you like!", TPMN_TEXT_COLOR, controller.canvas);

		__text(assets, y += TPMN_TEXT_SPACING * 2, "If you're interested in playing", TPMN_TEXT_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "with the level editor,", TPMN_TEXT_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "or playing with the code base,", TPMN_TEXT_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "or have any other questions", TPMN_TEXT_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "or comments, please contact:", TPMN_TEXT_COLOR, controller.canvas);

		__text(assets, y += TPMN_TEXT_SPACING, "johannes.norneby@gmail.com", TPMN_TITLE_COLOR, controller.canvas);

		__text(assets, y += TPMN_TEXT_SPACING * 2, "Thanks for playing!", TPMN_TEXT_COLOR, controller.canvas);

		__text(assets, y += TPMN_TEXT_SPACING * 4, "TP-Man Nightmare", TPMN_TITLE_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "(c)2012 NTI-Gymnasiet Goteborg", TPMN_TEXT_COLOR, controller.canvas);

		__text(assets, y += TPMN_TEXT_SPACING * 2, "Art / Design", TPMN_TITLE_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "Saga Velander", TPMN_TEXT_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "Michael Awakim Manaz", TPMN_TEXT_COLOR, controller.canvas);

		__text(assets, y += TPMN_TEXT_SPACING * 2, "Code / Music / Sound", TPMN_TITLE_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "Johannes 'johno' Norneby", TPMN_TEXT_COLOR, controller.canvas);

		if (y < -32)
			controller.credits_start_time = tpmn_model_now(model);
	}
	break;
	}
}

static void __draw_farplane(
	const micron_t& in_micron, const tpmn_model_t& model, const tpmn_assets_t& assets, const int32_t vx, const int32_t vy,
	tpmn_controller_t& controller)
{
	const float now = tpmn_model_now(model);

	const int32_t SX = tpmn_screen_x((float)vx);
	const int32_t SY = tpmn_screen_y((float)vy);

	uint32_t index = model_screen(SX, SY, model);

	//this mirrors how the flash version worked; there was indirection going in in a single case
	if (7 == index)
		index = 2;

	assert(index < _countof(assets.backgrounds));

	switch (index)
	{
	default:
		sd_bitmap_blit(assets.backgrounds[index], controller.canvas, 0, 0);
		break;

	case 1:
		sd_bitmap_blit(assets.backgrounds[index], controller.canvas, 0, 0);
		__draw_dust_pass(in_micron, false, now, 100, 20, assets.dust_far, controller.canvas);
		break;

	case 3:
		sd_bitmap_blit(assets.backgrounds[index], controller.canvas, 0, 0);
		for (tpmn_snowflake_t* f = controller.flakes; f < controller.flakes + _countof(controller.flakes); ++f)
		{
			f->x += f->sx * TPMN_SECONDS_PER_TICK;
			f->x += ::sinf(f->t + now * 5) * 16 * TPMN_SECONDS_PER_TICK;
			f->y += f->sy * TPMN_SECONDS_PER_TICK;

			if (f->x < -FLAKE_FRAME_ASPECT)
			{
				f->x = TPMN_CANVAS_WIDTH;
			}
			else if (f->x >= TPMN_CANVAS_WIDTH)
			{
				f->x = -FLAKE_FRAME_ASPECT;
			}
			if (f->y >= TPMN_CANVAS_HEIGHT)
			{
				f->y = -FLAKE_FRAME_ASPECT;
			}

			sd_bitmap_blit_half_key_clip(assets.flake, controller.canvas, nullptr, (int32_t)f->x, (int32_t)f->y, FLAKE_FRAME_ASPECT, FLAKE_FRAME_ASPECT, (f->t % 3) * FLAKE_FRAME_ASPECT, f->t / 3 * FLAKE_FRAME_ASPECT);
		}
		break;

	case 5:
		sd_bitmap_blit(assets.backgrounds[index], controller.canvas, 0, 0);
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

static void __draw_hero(
	const tpmn_model_t& model, const tpmn_assets_t& assets, const int32_t vx, const int32_t vy, sd_bitmap_t& canvas)
{
	//figure out state / frame
	int32_t state;
	int32_t frame;
	if (tpmn_hero_whipping(tpmn_model_now(model), model.hero))
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
		else if (tpmn_hero_left_input(model.hero) || tpmn_hero_right_input(model.hero))
		{
			state = HS_RUN;
			frame = int(tpmn_model_now(model) * 15) % 2;
		}
		else
		{
			state = HS_STAND;
			frame = 0;
		}
	}

	//blit
	sd_bitmap_blit_key_clip(
		assets.hero,
		canvas,
		nullptr,
		int32_t(model.hero.x - vx - TPMN_HERO_WIDTH / 2), int32_t(model.hero.y - vy - TPMN_HERO_HEIGHT / 2),
		TPMN_HERO_WIDTH, TPMN_HERO_HEIGHT,
		(frame + (model.hero.right_bit ? 1 : 0) * 2) * TPMN_HERO_WIDTH, state * TPMN_HERO_HEIGHT
	);

	//whip
	if (state == HS_WHIP)
	{
		sd_bitmap_blit_key_clip(
			assets.whip,
			canvas,
			nullptr,
			int32_t(tpmn_hero_whip_min_x(model.hero) - vx),
			int32_t(tpmn_hero_whip_y(model.hero) - vy - 22),
			assets.whip.width / 2,
			assets.whip.height / 2,
			model.hero.right_bit ? assets.whip.width / 2 : 0,
			tpmn_hero_whip_extended(tpmn_model_now(model), model.hero) ? assets.whip.height / 2 : 0
		);
	}
}

static void __draw_tiles(const tpmn_model_t& model, const tpmn_assets_t& assets, const int32_t vx, const int32_t vy, const bool replace, tpmn_controller_t& controller)
{
	//calc starting grid
	const int32_t gx = vx / TPMN_TILE_ASPECT;
	const int32_t gy = vy / TPMN_TILE_ASPECT;

	//calc subtile shift
	const int32_t sx = vx % TPMN_TILE_ASPECT;
	const int32_t sy = vy % TPMN_TILE_ASPECT;

	for (int32_t y = 0; y < TPMN_TILES_Y; ++y)
	{
		for (int32_t x = 0; x < TPMN_TILES_X; ++x)
		{
			const tpmn_tile_t& tile = tpmn_model_get_tile(gx + x, gy + y, model);
			uint32_t index = tile.index;

			if (replace)
			{
				switch (index)
				{
				case TPMN_LOGIC_INDEX_CHECKPOINT:
					if (tpmn_hero_is_checkpoint(gx + x, gy + y, model.hero))
						index = TPMN_LOGIC_INDEX_CHECKPOINT_CURRENT;
					break;

				case TPMN_LOGIC_INDEX_SPIKYGREEN:
				case TPMN_LOGIC_INDEX_ICICLE:
				case TPMN_LOGIC_INDEX_SANDBLOCK:
				case TPMN_LOGIC_INDEX_BLUEBLOB:
				case TPMN_LOGIC_INDEX_BROWNBLOB:
				case TPMN_LOGIC_INDEX_PLANT:
				case TPMN_LOGIC_INDEX_FIREDUDE:
				case TPMN_LOGIC_INDEX_SERVER:
				case TPMN_LOGIC_INDEX_BAT:
				case TPMN_LOGIC_INDEX_PENGUIN:
				case TPMN_LOGIC_INDEX_SCORPION:
					index = TPMN_LOGIC_INDEX_AIR;
					break;

				case TPMN_LOGIC_INDEX_KEY0:
				case TPMN_LOGIC_INDEX_KEY0BLOCK:
					if (model.hero.keys & TPMN_HERO_KEY0)
						index = TPMN_LOGIC_INDEX_AIR;
					break;

				case TPMN_LOGIC_INDEX_KEY1:
				case TPMN_LOGIC_INDEX_KEY1BLOCK:
					if (model.hero.keys & TPMN_HERO_KEY1)
						index = TPMN_LOGIC_INDEX_AIR;
					break;

				case TPMN_LOGIC_INDEX_KEY2:
				case TPMN_LOGIC_INDEX_KEY2BLOCK:
					if (model.hero.keys & TPMN_HERO_KEY2)
						index = TPMN_LOGIC_INDEX_AIR;
					break;
				}
			}

			//animated
			assert(index < TPMN_MAX_TILE);
			__draw_tile(false, assets, x * TPMN_TILE_ASPECT - sx, y * TPMN_TILE_ASPECT - sy, controller.current_tiles[index], controller.canvas);
		}
	}
}

static void __draw_deaths(
	const int32_t vx, const int32_t vy,
	tpmn_controller_t& controller)
{
	tpmn_death_t* d = controller.deaths;
	while (d < controller.deaths + controller.num_deaths)
	{
		d->s += TPMN_GRAVITY * TPMN_SECONDS_PER_TICK;

		d->y += d->s * TPMN_SECONDS_PER_TICK;

		const int32_t SX = int32_t(d->x - vx);
		const int32_t SY = int32_t(d->y - vy);

		if (SX >= (-d->w / 2) && SY >= (-d->h / 2) && SX < (TPMN_CANVAS_WIDTH + d->w / 2) && SY < (TPMN_CANVAS_HEIGHT + d->h / 2))
		{
			sd_bitmap_blit_key_clip(
				*d->bm,
				controller.canvas,
				nullptr,
				int32_t(d->x - vx - d->w / 2),
				int32_t(d->y - vy - d->h / 2),
				d->w,
				d->h,
				d->src_x,
				d->src_y
			);

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

static void __draw_enemies(
	const tpmn_model_t& model, const tpmn_assets_t& assets, const int32_t vx, const int32_t vy,
	sd_bitmap_t& canvas)
{
	const float now = tpmn_model_now(model);
	const int32_t spikygreen_num_frames = assets.spikygreen.height / assets.spikygreen.width;
	const int32_t spikygreen_frame = int32_t(now * 12) % spikygreen_num_frames;
	const int32_t firedude_frame = int32_t(now * 10) % (assets.firedude.height / FIREDUDE_FRAME_ASPECT);
	const int32_t bat_frame_height = assets.bat.height / 6;
	const int32_t blueblob_frame = int32_t(now * 10) % (assets.blueblob.height / TPMN_BLOB_FRAME_ASPECT);
	const int32_t brownblob_frame = int32_t(now * 10) % (assets.brownblob.height / TPMN_BLOB_FRAME_ASPECT);
	int32_t x_offset;
	int32_t src_x;

	for (const tpmn_enemy_t* e = model.enemy; e < model.enemy + model.num_enemy; ++e)
	{
		switch (e->type)
		{
		default:
			assert(0);
			break;

		case TPMN_LOGIC_INDEX_SPIKYGREEN:
			if (now > e->spawn_time)
			{
				sd_bitmap_blit_key_clip(
					assets.spikygreen, canvas, nullptr, int32_t(e->x - vx - assets.spikygreen.width / 2), int32_t(e->y - vy - assets.spikygreen.width / 2), assets.spikygreen.width, assets.spikygreen.width, 0,
					(e->speed > 0 ? spikygreen_num_frames - 1 - spikygreen_frame : spikygreen_frame) * assets.spikygreen.width
				);
			}
			break;

		case TPMN_LOGIC_INDEX_BLUEBLOB:
			if (now > e->spawn_time)
				sd_bitmap_blit_key_clip(assets.blueblob, canvas, nullptr, int32_t(e->x - vx - TPMN_BLOB_FRAME_ASPECT / 2), int32_t(e->y - vy - TPMN_BLOB_FRAME_ASPECT / 2), TPMN_BLOB_FRAME_ASPECT, TPMN_BLOB_FRAME_ASPECT, (e->speed > 0 ? 1 : 0) * TPMN_BLOB_FRAME_ASPECT, blueblob_frame * TPMN_BLOB_FRAME_ASPECT);
			break;

		case TPMN_LOGIC_INDEX_BROWNBLOB:
			if (now > e->spawn_time)
				sd_bitmap_blit_key_clip(assets.brownblob, canvas, nullptr, int32_t(e->x - vx - TPMN_BLOB_FRAME_ASPECT / 2), int32_t(e->y - vy - TPMN_BLOB_FRAME_ASPECT / 2), TPMN_BLOB_FRAME_ASPECT, TPMN_BLOB_FRAME_ASPECT, (e->speed > 0 ? 1 : 0) * TPMN_BLOB_FRAME_ASPECT, brownblob_frame * TPMN_BLOB_FRAME_ASPECT);
			break;

		case TPMN_LOGIC_INDEX_PLANT:
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
					src_x = TPMN_PLANT_FRAME_ASPECT;
				}
				else
				{
					x_offset = -22;
					src_x = 0;
				}
				sd_bitmap_blit_key_clip(assets.plant, canvas, nullptr, int32_t(e->x - vx) + x_offset, int32_t(e->y - vy) - 20, TPMN_PLANT_FRAME_ASPECT, TPMN_PLANT_FRAME_ASPECT, src_x, frame * TPMN_PLANT_FRAME_ASPECT);
			}
			break;

		case TPMN_LOGIC_INDEX_SCORPION:
			if (now > e->spawn_time)
				__draw_scorpion(model, *e, assets, int32_t(e->x - vx), int32_t(e->y - vy), canvas);
			break;

		case TPMN_LOGIC_INDEX_PENGUIN:
			if (now > e->spawn_time)
			{
				sd_bitmap_blit_key_clip(
					assets.penguin, canvas, nullptr,
					int32_t(e->x - vx - assets.penguin.width / 4), int32_t(e->y - vy + 2),
					assets.penguin.width / 2, assets.penguin.height / 2,
					(e->speed > 0 ? 1 : 0) * assets.penguin.width / 2, (::fabsf(e->speed) < SLIDER_PREPARE_SPEED ? 1 : 0) * assets.penguin.height / 2
				);
			}
			break;

		case TPMN_LOGIC_INDEX_FIREDUDE:
			sd_bitmap_blit_key_clip(assets.firedude, canvas, nullptr, int32_t(e->x - vx - FIREDUDE_FRAME_ASPECT / 2), int32_t(e->y - vy - FIREDUDE_FRAME_ASPECT / 2), FIREDUDE_FRAME_ASPECT, FIREDUDE_FRAME_ASPECT, (e->vector_x > 0 ? 1 : 0) * FIREDUDE_FRAME_ASPECT, firedude_frame * FIREDUDE_FRAME_ASPECT);
			break;

		case TPMN_LOGIC_INDEX_BAT:
		{
			int32_t frame;
			if (e->state == TPMN_BAT_S_PASSIVE)
				frame = 0;
			else
				frame = 1 + int32_t(now * 16) % 2;

			if (e->scared)
				frame += 3;

			sd_bitmap_blit_key_clip(assets.bat, canvas, nullptr, int32_t(e->x - vx - assets.bat.width / 2), int32_t(e->y - vy - bat_frame_height / 2), assets.bat.width, bat_frame_height, 0, frame * bat_frame_height);
		}
		break;

		case TPMN_LOGIC_INDEX_ICICLE:
		case TPMN_LOGIC_INDEX_SANDBLOCK:
			if (now > e->spawn_time)
			{
				switch (e->state)
				{
				default:
					__draw_tile(false, assets, int32_t(e->x - vx - TPMN_TILE_ASPECT / 2), int32_t(e->y - vy - TPMN_TILE_ASPECT / 2), e->type, canvas);
					break;

				case TPMN_ICICLE_S_PENDING:
					__draw_tile(false, assets, int32_t(e->x - vx - TPMN_TILE_ASPECT / 2 + tpmn_random_unit() * 2 - 1), int32_t(e->y - vy - TPMN_TILE_ASPECT / 2 + tpmn_random_unit() * 2 - 1), e->type, canvas);
					break;
				}
			}
			break;
		}
	}
}

static void __play_update(
	const micron_t& in_micron, const tpmn_assets_t& assets, tpmn_model_t& model,
	tpmn_controller_t& c)
{
	//play menu up?
	if (c.play_menu)
	{
		__text(assets, TPMN_CANVAS_HEIGHT / 3, "ESC = Quit", sd_white, c.canvas);
		if (micron_key_downflank(in_micron, MICRON_KEY_ESCAPE))
		{
			model.play_bit = 0;

			c.play_menu = false;
		}

		__text(assets, TPMN_CANVAS_HEIGHT / 3 * 2, "R = Resume", sd_white, c.canvas);
		if (micron_key_downflank(in_micron, 'R'))
		{
			c.play_menu = false;
		}
	}
	//play menu not up
	else
	{
		if (micron_key_downflank(in_micron, MICRON_KEY_ESCAPE))
		{
			c.play_menu = true;
		}
		else
		{
			uint32_t input = 0;

			//hero movement
			if (model.hero.spawn_time < 0.f)
			{
				if (micron_key_downflank(in_micron, 'S'))
					input |= TPMN_HERO_FLAGS_DOWN;

				if (micron_key_is_down(in_micron, 'A'))
					input |= TPMN_HERO_FLAGS_LEFT;

				if (micron_key_is_down(in_micron, 'D'))
					input |= TPMN_HERO_FLAGS_RIGHT;

				if (micron_key_is_down(in_micron, 'K'))
					input |= TPMN_HERO_FLAGS_JUMP;

				if (micron_key_downflank(in_micron, 'J'))
					input |= TPMN_HERO_FLAGS_WHIP;
			}

			//always set
			model.hero.input = input;
		}
	}

	//doOutput();
	{
		const int32_t VPX = __hero_view_position_x(model);
		const int32_t VPY = __hero_view_position_y(model);

		__draw_farplane(in_micron, model, assets, VPX, VPY, c);
		__draw_tiles(model, assets, VPX, VPY, true, c);
		__draw_portals(model, assets, VPX, VPY, c.canvas);
		__draw_servers(model, assets, VPX, VPY, c.canvas);
		__draw_enemies(model, assets, VPX, VPY, c.canvas);
		__draw_deaths(VPX, VPY, c);

		if (model.hero.spawn_time < 0.f)
			__draw_hero(model, assets, VPX, VPY, c.canvas);

		__draw_foreground(in_micron, model, assets, VPX, VPY, c);

		//drawInfos();
		{
			const tpmn_info_t* info = model_info_for_offset(tpmn_world_to_offset(model.hero.x, model.hero.y), model);
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
			sd_bitmap_blit_key(assets.gui_server_fixed, c.canvas, 0, 0);
			{
				char str[4];
				::sprintf_s(str, "%u", model.hero.fixed_servers.count);
				if (model.hero.fixed_servers.count < 10)
					sd_fontv_print(assets.font, 6, 6, str, c.canvas);
				else
					sd_fontv_print(assets.font, 1, 6, str, c.canvas);
			}

			//keys gui
			int32_t x = TPMN_TILE_ASPECT;
			if (model.hero.keys & TPMN_HERO_KEY0)
				__draw_tile(false, assets, x, 0, TPMN_LOGIC_INDEX_KEY0, c.canvas);
			if (model.hero.keys & TPMN_HERO_KEY1)
				__draw_tile(false, assets, x += TPMN_TILE_ASPECT, 0, TPMN_LOGIC_INDEX_KEY1, c.canvas);
			if (model.hero.keys & TPMN_HERO_KEY2)
				__draw_tile(false, assets, x += TPMN_TILE_ASPECT, 0, TPMN_LOGIC_INDEX_KEY2, c.canvas);

			//servers to fix (in current level)
			const uint32_t ns = __num_broken_servers(model);
			for (uint32_t i = 0; i < ns; ++i)
				sd_bitmap_blit_key(assets.gui_server_broken, c.canvas, TPMN_CANVAS_WIDTH - i * 14 - 20, 0);
		}

		if (c.play_menu)
		{
			__text(assets, TPMN_CANVAS_HEIGHT / 3, "ESC = Quit", sd_white, c.canvas);
			__text(assets, TPMN_CANVAS_HEIGHT / 3 * 2, "R = Resume", sd_white, c.canvas);
		}
	}
}

static tpmn_app_event_t __idle_update(
	const micron_t& in_micron, const tpmn_assets_t& assets,
	tpmn_controller_t& controller)
{
#if 0
	bitmap_blit(assets.myBackgrounds[1], controller.canvas, 0, 0);
	__draw_dust_pass(false, model_now(controller._remove_model), 100, 20, assets.dust_far, controller.canvas);
	__draw_dust_pass(true, model_now(controller._remove_model), 400, 50, assets.dust_near, controller.canvas);
	return app_event_t::nothing;
#else
	sd_bitmap_blit(assets.idle, controller.canvas, 0, 0);

	{
		int32_t y;

		__text(assets, y = 8, "TP-Man Nightmare", TPMN_TITLE_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "(c)2012-2024 nornware AB", TPMN_TEXT_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING * 2, "Talent", TPMN_TITLE_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "Saga Velander", TPMN_TEXT_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "Michael Awakim Manaz", TPMN_TEXT_COLOR, controller.canvas);
		__text(assets, y += TPMN_TEXT_SPACING, "Johannes 'johno' Norneby", TPMN_TEXT_COLOR, controller.canvas);

		__text(assets, y += TPMN_TEXT_SPACING * 4, "P = Play", sd_green, controller.canvas);
		if (micron_key_downflank(in_micron, 'P'))
			return tpmn_app_event_t::START_NEW_GAME;

		__text(assets, y += TPMN_TEXT_SPACING, "ESC = Quit", sd_green, controller.canvas);
		if (micron_key_downflank(in_micron, MICRON_KEY_ESCAPE))
			return tpmn_app_event_t::EXIT_APPLICATION;
	}

#if 1
	sd_bitmap_cross(controller.canvas, in_micron.canvas_cursor_x, in_micron.canvas_cursor_y, 8, 0xffff);
#endif

	return tpmn_app_event_t::NOTHING;
#endif
}

//public
//public
//public
//public

tpmn_app_event_t tpmn_controller_tick(
	const tpmn_assets_t& in_assets,
	tpmn_model_t& out_model, tpmn_controller_t& out_controller, micron_t& out_micron)
{
	switch (out_model.level.music_track)
	{
	default:	out_micron.music = "johno_Jungle_2012.ogg";	break;
	case 1:		out_micron.music = "johno_Minimal_2012.ogg";	break;
	case 2:		out_micron.music = "johno_Outspaced_Remix_2012.ogg";	break;
	case 3:		out_micron.music = "johno_Sunshower_2012.ogg";	break;
	case 4:		out_micron.music = "johno_They_Took_Their_Planet_Back_2012.ogg";	break;
	case 5:		out_micron.music = "johno_Sevens_2012.ogg";	break;
	}

	//tile animations
	{
		out_controller.tile_anim_tick += TILE_FPS * TPMN_SECONDS_PER_TICK;
		while (out_controller.tile_anim_tick > 1)
		{
			for (uint32_t t = 0; t < TPMN_MAX_TILE; ++t)
			{
				const uint32_t TILE = out_controller.current_tiles[t];
				out_controller.current_tiles[t] = in_assets.anim_target[TILE];
			}

			out_controller.tile_anim_tick -= 1;
		}
	}

	//this is single pass IMGUI, so input and output are intertwined
	tpmn_app_event_t result = tpmn_app_event_t::NOTHING;
	if (out_model.play_bit)
		__play_update(out_micron, in_assets, out_model, out_controller);
	else
		result = __idle_update(out_micron, in_assets, out_controller);

#if 0
	//display the number of dropped frames (60hz)
	{
		char str[16];
		::sprintf_s(str, "%u", dx9::state.app->_frame_drops);
		__text(assets, TPMN_CANVAS_HEIGHT - assets.font.height, str, softdraw::red, controller.canvas);
	}
#endif

	return result;
}

void tpmn_controller_on_load_new_world(tpmn_controller_t& out_controller, micron_t& out_micron)
{
	out_micron.sound_plays.push_back(TPMN_SND_SPAWN);

	out_controller.play_menu = false;

	for (
		tpmn_snowflake_t* f = out_controller.flakes;
		f < out_controller.flakes + _countof(out_controller.flakes);
		++f
		)
	{
		f->x = tpmn_random_unit() * TPMN_CANVAS_WIDTH;
		f->y = tpmn_random_unit() * TPMN_CANVAS_HEIGHT;
		f->sx = tpmn_random_unit() * 50 - 25;
		f->sy = tpmn_random_unit() * 75 + 25;
		f->t = uint32_t(tpmn_random_unit() * 6);
	}
}

void tpmn_controller_death_create(
	const sd_bitmap_t* aBitmap, const float aX, const float aY, const int32_t aWidth, const int32_t aHeight, const int32_t aSrcX, const int32_t aSrcY,
	tpmn_controller_t& controller)
{
	assert(controller.num_deaths < _countof(controller.deaths));
	tpmn_death_t* d = controller.deaths + controller.num_deaths++;
	d->bm = aBitmap;
	d->x = aX;
	d->y = aY;
	d->s = -350;
	d->w = aWidth;
	d->h = aHeight;
	d->src_x = aSrcX;
	d->src_y = aSrcY;
}
