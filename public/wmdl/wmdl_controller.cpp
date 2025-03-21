#include "stdafx.h"
#include "wmdl_controller.h"

#include "../micron.h"
#include "wmdl_assets.h"

#define ATASCII_TEST 0

#define SCROLL_X 0
#define SCROLL_Y 0

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

void wmdl_map(
	const wmdl_model_t& in_model,
	micron_t& out_micron);

static int32_t __hero_view_position_x(const wmdl_model_t& in_model)
{
	int32_t x = (int32_t)in_model.hero.x;

	//scrolling
	if (SCROLL_X)
	{
		x -= WMDL_TILE_ASPECT * WMDL_TILES_X / 2;
	}
	//fixed to screen boundaries
	else
	{
		x = x / (WMDL_TILE_ASPECT * WMDL_TILES_X);
		x *= WMDL_TILE_ASPECT * WMDL_TILES_X;
	}

	//bounds
	if (x < 0)
		x = 0;
#if 0
	else if (x > (WMDL_WORLD_WIDTH * WMDL_TILE_ASPECT - out_micron.canvas_width))
		x = WMDL_WORLD_WIDTH * WMDL_TILE_ASPECT - out_micron.canvas_width;
#else
	else if (x > (WMDL_WORLD_WIDTH - WMDL_TILES_X) * WMDL_TILE_ASPECT)
		x = (WMDL_WORLD_WIDTH - WMDL_TILES_X) * WMDL_TILE_ASPECT;
#endif

	return x;
}

static int32_t __hero_view_position_y(const wmdl_model_t& in_model)
{
	int32_t y = (int32_t)in_model.hero.y;

	//scrolling
	if (SCROLL_Y)
	{
		y -= WMDL_TILE_ASPECT * WMDL_TILES_Y / 2;
	}
	//fixed to screen boundaries
	else
	{
		y = y / (WMDL_TILE_ASPECT * WMDL_TILES_Y);
		y *= WMDL_TILE_ASPECT * WMDL_TILES_Y;
	}

	//bounds
	if (y < 0)
		y = 0;
#if 0
	else if (y > (WMDL_WORLD_HEIGHT * WMDL_TILE_ASPECT - out_micron.canvas_height))
		y = WMDL_WORLD_HEIGHT * WMDL_TILE_ASPECT - out_micron.canvas_height;
#else
	else if (y > (WMDL_WORLD_HEIGHT - WMDL_TILES_Y) * WMDL_TILE_ASPECT)
		y = (WMDL_WORLD_HEIGHT - WMDL_TILES_Y) * WMDL_TILE_ASPECT;
#endif

	return y;
}

static void __draw_portals(
	const wmdl_model_t& in_model, const wmdl_assets_t& in_assets, const int32_t in_vx, const int32_t in_vy,
	micron_t& out_micron)
{
	for (
		const wmdl_portal_t* PORTAL = in_model.level.portals;
		PORTAL < in_model.level.portals + in_model.level.num_portals;
		++PORTAL
		)
	{
		const int32_t X = (int32_t)wmdl_offset_to_world_x(PORTAL->offset) - in_vx - in_assets.portal.width / 4;
		const int32_t Y = (int32_t)wmdl_offset_to_world_y(PORTAL->offset) - in_vy - in_assets.portal.height + WMDL_TILE_ASPECT / 2;

		//frame based on accessability
		int32_t frame;
		if (in_model.hero.fixed_servers.count < PORTAL->server_count)
			frame = 1;
		else
			frame = 0;

		micron_blit_key_clip(out_micron, in_assets.key_index, in_assets.portal, X, Y, WMDL_TILE_ASPECT, in_assets.portal.height, frame * in_assets.portal.width / 2, 0);

		//if not accessible, draw number
		if (in_model.hero.fixed_servers.count < PORTAL->server_count)
		{
			char slask[4];
			::sprintf_s(slask, "%u", PORTAL->server_count);
			if (PORTAL->server_count >= 10)
				micron_print(out_micron, in_assets.key_index, in_assets.font, X + 1, Y + 4, slask);
			else
				micron_print(out_micron, in_assets.key_index, in_assets.font, X + 6, Y + 4, slask);
		}
	}
}

static void __draw_servers(
	const wmdl_model_t& in_model, const wmdl_assets_t& in_assets, const int32_t in_vx, const int32_t in_vy,
	micron_t& out_micron)
{
	//calc starting grid
	const int32_t GX = in_vx / WMDL_TILE_ASPECT;
	const int32_t GY = in_vy / WMDL_TILE_ASPECT;

	//calc subtile shift
	const int32_t SX = in_vx % WMDL_TILE_ASPECT;
	const int32_t SY = in_vy % WMDL_TILE_ASPECT;

	for (
		int32_t y = 0;
#if SCROLL_Y
		y < WMDL_TILES_Y + 1;
#else
		y < WMDL_TILES_Y;
#endif
		++y
		)
	{
		for (
			int32_t x = 0;
#if SCROLL_X
			x < WMDL_TILES_X + 1;
#else
			x < WMDL_TILES_X;
#endif
			++x
			)
		{
			if (WMDL_LOGIC_INDEX_SERVER == wmdl_model_get_tile(in_model, GX + x, GY + y).index)
			{
				const bool FIXED = wmdl_model_is_server_fixed(in_model.world, wmdl_grid_to_offset(GX + x, GY + y), in_model.hero);
				int32_t frame;

				//main rep / fixed state
				{
					int32_t ox;
					int32_t oy;
					if (FIXED)
					{
						frame = 2;
						ox = oy = 0;
					}
					else
					{
						frame = int32_t(wmdl_model_now(in_model) * 10) % 2;
						ox = int32_t(wmdl_random_unit() * 2) - 1;
						oy = int32_t(wmdl_random_unit() * 2) - 1;
					}

					micron_blit_key_clip(
						out_micron,
						in_assets.key_index,
						in_assets.server,
						x * WMDL_TILE_ASPECT - SX + ox,
						y * WMDL_TILE_ASPECT - SY - WMDL_TILE_ASPECT + oy,
						WMDL_TILE_ASPECT,
						in_assets.server.height,
						frame * WMDL_TILE_ASPECT,
						0
					);
				}

				//arcs
				if (!FIXED)
				{
					frame = int32_t(wmdl_model_now(in_model) * 16) % 4;

					micron_blit_key_clip(
						out_micron,
						0,//2024-12-02: apparently this index works (since the original was additive / on black background)
						in_assets.arcs,
						x * WMDL_TILE_ASPECT - SX - 4,
						y * WMDL_TILE_ASPECT - SY - WMDL_TILE_ASPECT - 4 + int32_t(wmdl_random_unit() * WMDL_TILE_ASPECT),
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
	const wmdl_assets_t& in_assets, const int32_t in_x, const int32_t in_y, const uint32_t in_tile,
	micron_t& out_micron)
{
	const int32_t TILES_ON_SOURCE_X = in_assets.tiles.width / WMDL_TILE_ASPECT;
	const int32_t TILES_ON_SOURCE_Y = in_assets.tiles.height / WMDL_TILE_ASPECT;
	if (in_tile < uint32_t(TILES_ON_SOURCE_X * TILES_ON_SOURCE_Y))
	{
		const int32_t SRC_X = (in_tile % TILES_ON_SOURCE_X) * WMDL_TILE_ASPECT;
		const int32_t SRC_Y = in_tile / TILES_ON_SOURCE_X * WMDL_TILE_ASPECT;
		micron_blit_key_clip(
			out_micron,
			in_assets.key_index,
			in_assets.tiles,
			in_x, in_y,
			WMDL_TILE_ASPECT, WMDL_TILE_ASPECT,
			SRC_X,
			SRC_Y
		);
	}
	else
	{
		micron_canvas_clear(out_micron, 255, in_x, in_y, WMDL_TILE_ASPECT, WMDL_TILE_ASPECT);
	}
}

static uint32_t __plant_state(const wmdl_hero_t& in_hero, const wmdl_enemy_t& in_plant)
{
	const float DISTANCE = wmdl_plant_hero_distance(in_plant, in_hero);
	if (DISTANCE < WMDL_PLANT_BITE_DISTANCE)
		return PLANT_S_BITE;

	if (DISTANCE < PLANT_POISE_DISTANCE)
		return PLANT_S_POISED;

	if (DISTANCE < PLANT_SUSPECT_DISTANCE)
		return PLANT_S_SUSPICIOUS;

	return PLANT_S_PASSIVE;
}

static void __draw_scorpion(
	const wmdl_model_t& in_model, const wmdl_enemy_t& in_plant, const wmdl_assets_t& in_assets, const int32_t in_x, const int32_t in_y,
	micron_t& out_micron)
{
	int32_t frame;
	switch (__plant_state(in_model.hero, in_plant))
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
	int32_t x_offset;
	int32_t src_x;
	if (in_model.hero.x > in_plant.x)
	{
		x_offset = -11;
		src_x = in_assets.scorpion.width / 2;
	}
	else
	{
		x_offset = -26;
		src_x = 0;
	}

	micron_blit_key_clip(
		out_micron,
		in_assets.key_index,
		in_assets.scorpion,
		in_x + x_offset, in_y - 14,
		in_assets.scorpion.width / 2, in_assets.scorpion.height / 6,
		src_x,
		frame * in_assets.scorpion.height / 6
	);
}

static const char* __draw_text(const uint32_t in_id, const uint32_t in_line)
{
	switch (in_id)
	{
	case 0:
		switch (in_line)
		{
		case 0:		return "Welcome Whip Man,";
		case 1:		return "use ADK to get around!";
		}
		break;

	case 1:
		switch (in_line)
		{
		case 0:		return "Press S";
		case 1:		return "to pass through doors!";
		}
		break;

	case 2:
		switch (in_line)
		{
		case 0:		return "You need to JUMP(K) over these blocks!";
		}
		break;

	case 3:
		switch (in_line)
		{
		case 0:		return "Beware of enemies!";
		case 1:		return "You need to JUMP(K) on this kind to defeat it!";
		}
		break;

	case 4:
		switch (in_line)
		{
		case 0:		return "This is a broken server. You need to";
		case 1:		return "fix them to proceed through doors.";
		}
		break;

	case 5:
		switch (in_line)
		{
		case 0:		return "Some enemies must be whipped,";
		case 1:		return "use J!";
		}
		break;

	case 6:
		switch (in_line)
		{
		case 0:		return "Keys (of various colors)";
		case 1:		return "will open LOCKED BLOCKS!";
		}
		break;

	case 7:
		switch (in_line)
		{
		case 0:		return "Things will get more challenging";
		case 1:		return "the further you proceed!";
		}
		break;

	case 8:
		switch (in_line)
		{
		case 0:		return "This concludes the tutorial,";
		case 1:		return "good luck in your quest!";
		}
		break;
	}

	return nullptr;
}

static uint32_t __num_broken_servers(const wmdl_model_t& in_model)
{
	uint32_t count = 0;

	for (
		uint32_t offset = 0;
		offset < WMDL_WORLD_SIZE;
		++offset
		)
	{
		if (in_model.level.tiles[offset].index == WMDL_LOGIC_INDEX_SERVER)
		{
			if (false == wmdl_model_is_server_fixed(in_model.world, offset, in_model.hero))
				++count;
		}
	}

	return count;
}

/*
static void __draw_dust_pass(
	const minyin_t& in_micron, const bool add, const float now, const float speed_x, const float speed_y, const sd_bitmap_t& bitmap,
	minyin_bitmap_t& out_micron)
{
	if (in_micron.key_is_down('P'))
		return;

	const int32_t X = int32_t(now * speed_x) % bitmap.width;
	const int32_t Y = int32_t(now * speed_y) % bitmap.height;
	if (add)
	{
		minyin_blit_add_key_clip(out_micron, bitmap, -X, -Y);
		minyin_blit_add_key_clip(out_micron, bitmap, bitmap.width - X, -Y);
		minyin_blit_add_key_clip(out_micron, bitmap, -X, bitmap.height - Y);
		minyin_blit_add_key_clip(out_micron, bitmap, bitmap.width - X, bitmap.height - Y);
	}
	else
	{
		minyin_blit_half_key_clip(out_micron, bitmap, -X, -Y);
		minyin_blit_half_key_clip(out_micron, bitmap, bitmap.width - X, -Y);
		minyin_blit_half_key_clip(out_micron, bitmap, -X, bitmap.height - Y);
		minyin_blit_half_key_clip(out_micron, bitmap, bitmap.width - X, bitmap.height - Y);
	}
}
*/

static void __draw_text(
	const wmdl_assets_t& in_assets, const int in_dst_y, const char* in_string, const uint8_t in_color,
	micron_t& out_micron)
{
	micron_print_colorize(out_micron, in_assets.key_index, in_assets.text_edge_index, in_assets.font, in_color, (out_micron.canvas_width - micron_font_string_width(in_assets.font, in_string)) / 2, in_dst_y, in_string);
}

static void __draw_foreground(
	const wmdl_model_t& in_model, const wmdl_assets_t& in_assets, const int32_t in_vx, const int32_t in_vy,
	wmdl_controller_t& out_controller, micron_t& out_micron)
{
	const int32_t SX = wmdl_screen_x((float)in_vx);
	const int32_t SY = wmdl_screen_y((float)in_vy);

	const uint32_t INDEX = wmdl_model_screen(in_model, SX, SY);
	switch (INDEX)
	{
	case 1:
		//in_micron;
		//__draw_dust_pass(in_micron, true, wmdl_model_now(model), 400, 50, assets.dust_near, controller.canvas);
		break;

	case 7:
	{
		if (out_controller.credits_start_time < 0.)
			out_controller.credits_start_time = wmdl_model_now(in_model);

		int32_t y;

		__draw_text(in_assets, y = out_micron.canvas_height + int32_t((wmdl_model_now(in_model) - out_controller.credits_start_time) * -16.), "Congratulations!", WMDL_TITLE_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "You made it to the end of the game!", WMDL_TEXT_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "We wanted to put an epic boss fight", WMDL_TEXT_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "here, but we couldn't quite", WMDL_TEXT_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "find the time.", WMDL_TEXT_COLOR, out_micron);

		__draw_text(in_assets, y += WMDL_TEXT_SPACING * 2, "Feel free to explore the game", WMDL_TEXT_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "world further, or play it again,", WMDL_TEXT_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "or whatever you like!", WMDL_TEXT_COLOR, out_micron);

		__draw_text(in_assets, y += WMDL_TEXT_SPACING * 2, "If you're interested in playing", WMDL_TEXT_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "with the level editor,", WMDL_TEXT_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "or playing with the code base,", WMDL_TEXT_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "or have any other questions", WMDL_TEXT_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "or comments, please contact:", WMDL_TEXT_COLOR, out_micron);

		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "contact@nornware.com", WMDL_TITLE_COLOR, out_micron);

		__draw_text(in_assets, y += WMDL_TEXT_SPACING * 2, "Thanks for playing!", WMDL_TEXT_COLOR, out_micron);

		__draw_text(in_assets, y += WMDL_TEXT_SPACING * 4, "Whip Man Danger Land", WMDL_TITLE_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "(c)2012-2024 nornware AB", WMDL_TEXT_COLOR, out_micron);

		__draw_text(in_assets, y += WMDL_TEXT_SPACING * 2, "Art / Design", WMDL_TITLE_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "Saga Velander", WMDL_TEXT_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "Michael Awakim Manaz", WMDL_TEXT_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "Mats Persson", WMDL_TEXT_COLOR, out_micron);

		__draw_text(in_assets, y += WMDL_TEXT_SPACING * 2, "Code / Music / Sound", WMDL_TITLE_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "Johannes 'johno' Norneby", WMDL_TEXT_COLOR, out_micron);

		if (y < -32)
			out_controller.credits_start_time = wmdl_model_now(in_model);
	}
	break;
	}
}

static void __draw_farplane(
	const wmdl_model_t& in_model, const wmdl_assets_t& in_assets, const int32_t in_vx, const int32_t in_vy,
	wmdl_controller_t& out_controller, micron_t& out_micron)
{
	const float NOW = wmdl_model_now(in_model);

	const int32_t SX = wmdl_screen_x((float)in_vx);
	const int32_t SY = wmdl_screen_y((float)in_vy);

	uint32_t index = wmdl_model_screen(in_model, SX, SY);

	//this mirrors how the flash version worked; there was indirection going in in a single case
	if (7 == index)
		index = 2;

	assert(index < _countof(in_assets.backgrounds));

	switch (index)
	{
	default:
		micron_blit(out_micron, in_assets.backgrounds[index], 0, 0);
		break;

	case 1:
		micron_blit(out_micron, in_assets.backgrounds[index], 0, 0);
		//in_micron;
		//__draw_dust_pass(in_micron, false, now, 100, 20, assets.dust_far, controller.canvas);
		break;

	case 3:
		micron_blit(out_micron, in_assets.backgrounds[index], 0, 0);
		for (
			wmdl_snowflake_t* FLAKE = out_controller.flakes;
			FLAKE < out_controller.flakes + _countof(out_controller.flakes);
			++FLAKE
			)
		{
			FLAKE->x += FLAKE->sx * WMDL_SECONDS_PER_TICK;
			FLAKE->x += ::sinf(FLAKE->t + NOW * 5) * 16 * WMDL_SECONDS_PER_TICK;
			FLAKE->y += FLAKE->sy * WMDL_SECONDS_PER_TICK;

			if (FLAKE->x < -FLAKE_FRAME_ASPECT)
			{
				FLAKE->x = (float)out_micron.canvas_width;
			}
			else if (FLAKE->x >= out_micron.canvas_width)
			{
				FLAKE->x = -FLAKE_FRAME_ASPECT;
			}
			if (FLAKE->y >= out_micron.canvas_height)
			{
				FLAKE->y = -FLAKE_FRAME_ASPECT;
			}

			//minyin_blit_half_key_clip(controller.canvas, assets.flake, (int32_t)FLAKE->x, (int32_t)FLAKE->y, FLAKE_FRAME_ASPECT, FLAKE_FRAME_ASPECT, (FLAKE->t % 3) * FLAKE_FRAME_ASPECT, FLAKE->t / 3 * FLAKE_FRAME_ASPECT);
			micron_blit_key_clip(out_micron, in_assets.key_index, in_assets.flake, (int32_t)FLAKE->x, (int32_t)FLAKE->y, FLAKE_FRAME_ASPECT, FLAKE_FRAME_ASPECT, (FLAKE->t % 3) * FLAKE_FRAME_ASPECT, FLAKE->t / 3 * FLAKE_FRAME_ASPECT);
		}
		break;

	case 5:
		micron_blit(out_micron, in_assets.backgrounds[index], 0, 0);
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
	const wmdl_model_t& in_model, const wmdl_assets_t& in_assets, const int32_t in_vx, const int32_t in_vy,
	micron_t& out_micron)
{
	//figure out state / frame
	int32_t state;
	int32_t frame;
	if (wmdl_hero_whipping(in_model.hero, wmdl_model_now(in_model)))
	{
		state = HS_WHIP;
		frame = 0;
	}
	else
	{
		if (in_model.hero.air_bit)
		{
			state = HS_AIR;
			frame = 0;
		}
		else if (wmdl_hero_left_input(in_model.hero) || wmdl_hero_right_input(in_model.hero))
		{
			state = HS_RUN;
			frame = int(wmdl_model_now(in_model) * 15) % 2;
		}
		else
		{
			state = HS_STAND;
			frame = 0;
		}
	}

	//blit
	micron_blit_key_clip(
		out_micron,
		in_assets.key_index,
		in_assets.hero,
		int32_t(in_model.hero.x - in_vx - WMDL_HERO_WIDTH / 2), int32_t(in_model.hero.y - in_vy - WMDL_HERO_HEIGHT / 2),
		WMDL_HERO_WIDTH, WMDL_HERO_HEIGHT,
		(frame + (in_model.hero.right_bit ? 1 : 0) * 2) * WMDL_HERO_WIDTH, state * WMDL_HERO_HEIGHT
	);

	//whip
	if (state == HS_WHIP)
	{
		micron_blit_key_clip(
			out_micron,
			in_assets.key_index,
			in_assets.whip,
			int32_t(wmdl_hero_whip_min_x(in_model.hero) - in_vx),
			int32_t(wmdl_hero_whip_y(in_model.hero) - in_vy - 22),
			in_assets.whip.width / 2,
			in_assets.whip.height / 2,
			in_model.hero.right_bit ? in_assets.whip.width / 2 : 0,
			wmdl_hero_whip_extended(in_model.hero, wmdl_model_now(in_model)) ? in_assets.whip.height / 2 : 0
		);
	}
}

static void __draw_tiles(
	const wmdl_model_t& in_model, const wmdl_assets_t& in_assets, const int32_t in_vx, const int32_t in_vy, const bool in_replace,
	wmdl_controller_t& out_controller, micron_t& out_micron)
{
	//calc starting grid
	const int32_t GRID_X = in_vx / WMDL_TILE_ASPECT;
	const int32_t GRID_Y = in_vy / WMDL_TILE_ASPECT;

	//calc subtile shift
	const int32_t SHIFT_X = in_vx % WMDL_TILE_ASPECT;
	const int32_t SHIFT_Y = in_vy % WMDL_TILE_ASPECT;

	for (
		int32_t y = 0;
#if SCROLL_Y
		y < WMDL_TILES_Y + 1;
#else
		y < WMDL_TILES_Y;
#endif
		++y
		)
	{
		for (
			int32_t x = 0;
#if SCROLL_X
			x < WMDL_TILES_X + 1;
#else
			x < WMDL_TILES_X;
#endif
			++x
			)
		{
			const wmdl_tile_t& TILE = wmdl_model_get_tile(in_model, GRID_X + x, GRID_Y + y);
			uint32_t index = TILE.index;

			if (in_replace)
			{
				switch (index)
				{
				case WMDL_LOGIC_INDEX_CHECKPOINT:
					if (wmdl_hero_is_checkpoint(in_model.hero, GRID_X + x, GRID_Y + y))
						index = WMDL_LOGIC_INDEX_CHECKPOINT_CURRENT;
					break;

				case WMDL_LOGIC_INDEX_SPIKYGREEN:
				case WMDL_LOGIC_INDEX_ICICLE:
				case WMDL_LOGIC_INDEX_SANDBLOCK:
				case WMDL_LOGIC_INDEX_BLUEBLOB:
				case WMDL_LOGIC_INDEX_BROWNBLOB:
				case WMDL_LOGIC_INDEX_PLANT:
				case WMDL_LOGIC_INDEX_FIREDUDE:
				case WMDL_LOGIC_INDEX_SERVER:
				case WMDL_LOGIC_INDEX_BAT:
				case WMDL_LOGIC_INDEX_PENGUIN:
				case WMDL_LOGIC_INDEX_SCORPION:
					index = WMDL_LOGIC_INDEX_AIR;
					break;

				case WMDL_LOGIC_INDEX_KEY0:
				case WMDL_LOGIC_INDEX_KEY0BLOCK:
					if (in_model.hero.keys & WMDL_HERO_KEYBITS_0)
						index = WMDL_LOGIC_INDEX_AIR;
					break;

				case WMDL_LOGIC_INDEX_KEY1:
				case WMDL_LOGIC_INDEX_KEY1BLOCK:
					if (in_model.hero.keys & WMDL_HERO_KEYBITS_1)
						index = WMDL_LOGIC_INDEX_AIR;
					break;

				case WMDL_LOGIC_INDEX_KEY2:
				case WMDL_LOGIC_INDEX_KEY2BLOCK:
					if (in_model.hero.keys & WMDL_HERO_KEYBITS_2)
						index = WMDL_LOGIC_INDEX_AIR;
					break;
				}
			}

			//animated
			assert(index < WMDL_MAX_TILE);
			if (WMDL_LOGIC_INDEX_AIR != index)
				__draw_tile(in_assets, x * WMDL_TILE_ASPECT - SHIFT_X, y * WMDL_TILE_ASPECT - SHIFT_Y, out_controller.current_tiles[index], out_micron);
		}
	}
}

static void __draw_deaths(
	const wmdl_assets_t& in_assets, const int32_t in_vx, const int32_t in_vy,
	wmdl_controller_t& out_controller, micron_t& out_micron)
{
	wmdl_death_t* death = out_controller.deaths;
	while (death < out_controller.deaths + out_controller.num_deaths)
	{
		death->s += WMDL_GRAVITY * WMDL_SECONDS_PER_TICK;

		death->y += death->s * WMDL_SECONDS_PER_TICK;

		const int32_t SX = int32_t(death->x - in_vx);
		const int32_t SY = int32_t(death->y - in_vy);

		if (SX >= (-death->w / 2) && SY >= (-death->h / 2) && SX < (out_micron.canvas_width + death->w / 2) && SY < (out_micron.canvas_height + death->h / 2))
		{
			micron_blit_key_clip(
				out_micron,
				in_assets.key_index,
				*death->bm,
				int32_t(death->x - in_vx - death->w / 2),
				int32_t(death->y - in_vy - death->h / 2),
				death->w,
				death->h,
				death->src_x,
				death->src_y
			);

			//advance iterator
			++death;
		}
		else
		{
			//copy back last, don't advance iterator
			*death = out_controller.deaths[out_controller.num_deaths - 1];
			--out_controller.num_deaths;
		}
	}
}

static void __draw_enemies(
	const wmdl_model_t& in_model, const wmdl_assets_t& in_assets, const int32_t in_vx, const int32_t in_vy,
	micron_t& out_micron)
{
	const float NOW = wmdl_model_now(in_model);
	const int32_t spikygreen_num_frames = in_assets.spikygreen.height / in_assets.spikygreen.width;
	const int32_t spikygreen_frame = int32_t(NOW * 12) % spikygreen_num_frames;
	const int32_t firedude_frame = int32_t(NOW * 10) % (in_assets.firedude.height / FIREDUDE_FRAME_ASPECT);
	const int32_t bat_frame_height = in_assets.bat.height / 6;
	const int32_t blueblob_frame = int32_t(NOW * 10) % (in_assets.blueblob.height / WMDL_BLOB_FRAME_ASPECT);
	const int32_t brownblob_frame = int32_t(NOW * 10) % (in_assets.brownblob.height / WMDL_BLOB_FRAME_ASPECT);
	int32_t x_offset;
	int32_t src_x;

	for (
		const wmdl_enemy_t* ENEMY = in_model.enemy;
		ENEMY < in_model.enemy + in_model.num_enemy;
		++ENEMY
		)
	{
		switch (ENEMY->type)
		{
		default:
			assert(0);
			break;

		case WMDL_LOGIC_INDEX_SPIKYGREEN:
			if (NOW > ENEMY->spawn_time)
			{
				micron_blit_key_clip(
					out_micron,
					in_assets.key_index,
					in_assets.spikygreen,
					int32_t(ENEMY->x - in_vx - in_assets.spikygreen.width / 2),
					int32_t(ENEMY->y - in_vy - in_assets.spikygreen.width / 2),
					in_assets.spikygreen.width,
					in_assets.spikygreen.width,
					0,
					(ENEMY->speed > 0 ? spikygreen_num_frames - 1 - spikygreen_frame : spikygreen_frame) * in_assets.spikygreen.width
				);
			}
			break;

		case WMDL_LOGIC_INDEX_BLUEBLOB:
			if (NOW > ENEMY->spawn_time)
				micron_blit_key_clip(
					out_micron,
					in_assets.key_index,
					in_assets.blueblob,
					int32_t(ENEMY->x - in_vx - WMDL_BLOB_FRAME_ASPECT / 2),
					int32_t(ENEMY->y - in_vy - WMDL_BLOB_FRAME_ASPECT / 2),
					WMDL_BLOB_FRAME_ASPECT,
					WMDL_BLOB_FRAME_ASPECT,
					(ENEMY->speed > 0 ? 1 : 0) * WMDL_BLOB_FRAME_ASPECT,
					blueblob_frame * WMDL_BLOB_FRAME_ASPECT
				);
			break;

		case WMDL_LOGIC_INDEX_BROWNBLOB:
			if (NOW > ENEMY->spawn_time)
				micron_blit_key_clip(
					out_micron,
					in_assets.key_index,
					in_assets.brownblob,
					int32_t(ENEMY->x - in_vx - WMDL_BLOB_FRAME_ASPECT / 2),
					int32_t(ENEMY->y - in_vy - WMDL_BLOB_FRAME_ASPECT / 2),
					WMDL_BLOB_FRAME_ASPECT,
					WMDL_BLOB_FRAME_ASPECT,
					(ENEMY->speed > 0 ? 1 : 0) * WMDL_BLOB_FRAME_ASPECT,
					brownblob_frame * WMDL_BLOB_FRAME_ASPECT
				);
			break;

		case WMDL_LOGIC_INDEX_PLANT:
			if (NOW > ENEMY->spawn_time)
			{
				int32_t frame;
				switch (__plant_state(in_model.hero, *ENEMY))
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

				if (in_model.hero.x > ENEMY->x)
				{
					x_offset = -9;
					src_x = WMDL_PLANT_FRAME_ASPECT;
				}
				else
				{
					x_offset = -22;
					src_x = 0;
				}
				micron_blit_key_clip(
					out_micron,
					in_assets.key_index,
					in_assets.plant,
					int32_t(ENEMY->x - in_vx) + x_offset,
					int32_t(ENEMY->y - in_vy) - 20,
					WMDL_PLANT_FRAME_ASPECT,
					WMDL_PLANT_FRAME_ASPECT,
					src_x,
					frame * WMDL_PLANT_FRAME_ASPECT
				);
			}
			break;

		case WMDL_LOGIC_INDEX_SCORPION:
			if (NOW > ENEMY->spawn_time)
				__draw_scorpion(in_model, *ENEMY, in_assets, int32_t(ENEMY->x - in_vx), int32_t(ENEMY->y - in_vy), out_micron);
			break;

		case WMDL_LOGIC_INDEX_PENGUIN:
			if (NOW > ENEMY->spawn_time)
			{
				micron_blit_key_clip(
					out_micron,
					in_assets.key_index,
					in_assets.penguin,
					int32_t(ENEMY->x - in_vx - in_assets.penguin.width / 4),
					int32_t(ENEMY->y - in_vy + 2),
					in_assets.penguin.width / 2,
					in_assets.penguin.height / 2,
					(ENEMY->speed > 0 ? 1 : 0) * in_assets.penguin.width / 2,
					(::fabsf(ENEMY->speed) < SLIDER_PREPARE_SPEED ? 1 : 0) * in_assets.penguin.height / 2
				);
			}
			break;

		case WMDL_LOGIC_INDEX_FIREDUDE:
			micron_blit_key_clip(
				out_micron,
				in_assets.key_index,
				in_assets.firedude,
				int32_t(ENEMY->x - in_vx - FIREDUDE_FRAME_ASPECT / 2),
				int32_t(ENEMY->y - in_vy - FIREDUDE_FRAME_ASPECT / 2),
				FIREDUDE_FRAME_ASPECT,
				FIREDUDE_FRAME_ASPECT,
				(ENEMY->vector_x > 0 ? 1 : 0) * FIREDUDE_FRAME_ASPECT,
				firedude_frame * FIREDUDE_FRAME_ASPECT
			);
			break;

		case WMDL_LOGIC_INDEX_BAT:
		{
			int32_t frame;
			if (ENEMY->state == WMDL_BAT_S_PASSIVE)
				frame = 0;
			else
				frame = 1 + int32_t(NOW * 16) % 2;

			if (ENEMY->scared)
				frame += 3;

			micron_blit_key_clip(
				out_micron,
				in_assets.key_index,
				in_assets.bat,
				int32_t(ENEMY->x - in_vx - in_assets.bat.width / 2),
				int32_t(ENEMY->y - in_vy - bat_frame_height / 2),
				in_assets.bat.width,
				bat_frame_height,
				0,
				frame * bat_frame_height
			);
		}
		break;

		case WMDL_LOGIC_INDEX_ICICLE:
		case WMDL_LOGIC_INDEX_SANDBLOCK:
			if (NOW > ENEMY->spawn_time)
			{
				switch (ENEMY->state)
				{
				default:
					__draw_tile(
						in_assets,
						int32_t(ENEMY->x - in_vx - WMDL_TILE_ASPECT / 2),
						int32_t(ENEMY->y - in_vy - WMDL_TILE_ASPECT / 2),
						ENEMY->type,
						out_micron
					);
					break;

				case WMDL_ICICLE_S_PENDING:
					__draw_tile(
						in_assets,
						int32_t(ENEMY->x - in_vx - WMDL_TILE_ASPECT / 2 + wmdl_random_unit() * 2 - 1),
						int32_t(ENEMY->y - in_vy - WMDL_TILE_ASPECT / 2 + wmdl_random_unit() * 2 - 1),
						ENEMY->type,
						out_micron
					);
					break;
				}
			}
			break;
		}
	}
}


static void __play_update(
	const micron_t& in_micron, const wmdl_assets_t& in_assets, wmdl_model_t& in_model,
	wmdl_controller_t& out_controller, micron_t& out_micron)
{
	//play menu up?
	if (out_controller.play_menu)
	{
		if (micron_key_downflank(in_micron, MICRON_KEY_ESCAPE))
		{
			in_model.play_bit = 0;

			out_controller.play_menu = false;
		}

		if (micron_key_downflank(in_micron, 'R'))
		{
			out_controller.play_menu = false;
		}
	}
	//play menu not up
	else
	{
		if (micron_key_downflank(in_micron, MICRON_KEY_ESCAPE))
		{
			out_controller.play_menu = true;
		}
		else
		{
			uint32_t input = 0;

			//hero movement
			if (in_model.hero.spawn_time < 0.f)
			{
				if (micron_key_downflank(in_micron, 'S'))
					input |= WMDL_HERO_FLAGS_DOWN;

				if (micron_key_is_down(in_micron, 'A'))
					input |= WMDL_HERO_FLAGS_LEFT;

				if (micron_key_is_down(in_micron, 'D'))
					input |= WMDL_HERO_FLAGS_RIGHT;

				if (micron_key_is_down(in_micron, 'K'))
					input |= WMDL_HERO_FLAGS_JUMP;

				if (micron_key_downflank(in_micron, 'J'))
					input |= WMDL_HERO_FLAGS_WHIP;
			}

			//always set
			in_model.hero.input = input;
		}
	}//input

	{//output
		const int32_t VPX = __hero_view_position_x(in_model);
		const int32_t VPY = __hero_view_position_y(in_model);

		__draw_farplane(in_model, in_assets, VPX, VPY, out_controller, out_micron);
		__draw_tiles(in_model, in_assets, VPX, VPY, true, out_controller, out_micron);
		__draw_portals(in_model, in_assets, VPX, VPY, out_micron);
		__draw_servers(in_model, in_assets, VPX, VPY, out_micron);
		__draw_enemies(in_model, in_assets, VPX, VPY, out_micron);
		__draw_deaths(in_assets, VPX, VPY, out_controller, out_micron);

		if (in_model.hero.spawn_time < 0.f)
			__draw_hero(in_model, in_assets, VPX, VPY, out_micron);

		__draw_foreground(in_model, in_assets, VPX, VPY, out_controller, out_micron);

		//draw infos
		{
			const wmdl_info_t* INFO = wmdl_model_info_for_offset(in_model, wmdl_world_to_offset(in_model.hero.x, in_model.hero.y));
			if (INFO)
			{
				const char* s;

				s = __draw_text(INFO->id, 0);
				if (s)
					__draw_text(in_assets, 0, s, WMDL_PROMPT_COLOR, out_micron);

				s = __draw_text(INFO->id, 1);
				if (s)
					__draw_text(in_assets, in_assets.font.height, s, WMDL_PROMPT_COLOR, out_micron);
			}
		}

		//draw gui
		{
			//total servers fixed
			micron_blit_key(out_micron, in_assets.key_index, in_assets.gui_server_fixed, 0, 0);
			{
				char slask[4];
				::sprintf_s(slask, "%u", in_model.hero.fixed_servers.count);
				if (in_model.hero.fixed_servers.count < 10)
					micron_print(out_micron, in_assets.key_index, in_assets.font, 6, 6, slask);
				else
					micron_print(out_micron, in_assets.key_index, in_assets.font, 1, 6, slask);
			}

			//keys gui
			int32_t x = WMDL_TILE_ASPECT;
			if (in_model.hero.keys & WMDL_HERO_KEYBITS_0)
				__draw_tile(in_assets, x, 0, WMDL_LOGIC_INDEX_KEY0, out_micron);
			if (in_model.hero.keys & WMDL_HERO_KEYBITS_1)
				__draw_tile(in_assets, x += WMDL_TILE_ASPECT, 0, WMDL_LOGIC_INDEX_KEY1, out_micron);
			if (in_model.hero.keys & WMDL_HERO_KEYBITS_2)
				__draw_tile(in_assets, x += WMDL_TILE_ASPECT, 0, WMDL_LOGIC_INDEX_KEY2, out_micron);

			//servers to fix (in current level)
			const uint32_t NUM_BROKEN_SERVERS = __num_broken_servers(in_model);
			for (
				uint32_t i = 0;
				i < NUM_BROKEN_SERVERS;
				++i
				)
			{
				micron_blit_key(out_micron, in_assets.key_index, in_assets.gui_server_broken, out_micron.canvas_width - i * 14 - 20, 0);
			}
		}

		if (out_controller.play_menu)
		{
			__draw_text(in_assets, out_micron.canvas_height / 3, "ESC = Quit", WMDL_PROMPT_COLOR, out_micron);
			__draw_text(in_assets, out_micron.canvas_height / 3 * 2, "R = Resume", WMDL_PROMPT_COLOR, out_micron);
		}

		wmdl_map(in_model, out_micron);
	}//output
}

static wmdl_app_event_t __idle_update(
	const wmdl_assets_t& in_assets,
	micron_t& out_micron)
{
#if 0
	{
		static uint32_t src_offset = 0;
		const uint8_t* src = (uint8_t*)in_assets.bitmap_memory.data + src_offset;
		uint8_t* dst = out_micron.canvas;
		while (dst < out_micron.canvas + out_micron.canvas_width * out_micron.canvas_height)
		{
			*dst++ = *src++;
		}
		src_offset += out_micron.canvas_width;
	}
#endif

	micron_blit(out_micron, in_assets.idle, 0, 0);

#if 0
	{//show palette
		constexpr int32_t CELL = 4;
		for (uint32_t i = 0; i < 256; ++i)
		{
			for (int32_t y = 0; y < CELL; ++y)
			{
				for (int32_t x = 0; x < CELL; ++x)
				{
					out_micron.canvas[(CELL * (i % 16) + x) + (CELL * (i / 16) + y) * out_micron.canvas_width] = (uint8_t)i;
				}
			}
		}
	}//show palette
#endif

	{
		int32_t y;

		__draw_text(in_assets, y = 8, "Whip Man Danger Land", WMDL_TITLE_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "(c)2012-2025 nornware AB", WMDL_TEXT_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING * 2, "Talent", WMDL_TITLE_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "Saga Velander", WMDL_TEXT_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "Michael Awakim Manaz", WMDL_TEXT_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "Mats Persson", WMDL_TEXT_COLOR, out_micron);
		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "Johannes 'johno' Norneby", WMDL_TEXT_COLOR, out_micron);

		__draw_text(in_assets, y += WMDL_TEXT_SPACING * 4, "P = Play", WMDL_PROMPT_COLOR, out_micron);
		if (micron_key_downflank(out_micron, 'P'))
			return wmdl_app_event_t::START_NEW_GAME;

		__draw_text(in_assets, y += WMDL_TEXT_SPACING, "ESC = Quit", WMDL_PROMPT_COLOR, out_micron);
		if (micron_key_downflank(out_micron, MICRON_KEY_ESCAPE))
			return wmdl_app_event_t::EXIT_APPLICATION;
	}

	//cursor test
#if 1
	micron_canvas_cross(out_micron, out_micron.canvas_cursor_x, out_micron.canvas_cursor_y, 8, 0xff);
#endif

	return wmdl_app_event_t::NOTHING;
}

#if ATASCII_TEST
static uint32_t __prng()
{
	static uint32_t prng = 0;
	prng = 1664525l * prng + 1013904223l;
	return 0x3f800000 | (prng & 0x007fffff);
}

static void __atascii_test(
	const wmdl_model_t& in_model,
	micron_t& out_micron)
{
#if 0
	{
		uint8_t c;
		int32_t x;
		int32_t y;
		micron_canvas_atascii_print(out_micron, "\x80Hello ATASCII!?\x80", c = 128, c - 32, x = 32, y = 128);
		micron_canvas_atascii_print(out_micron, "Are you a villain?", c += 16, c - 32, x, y += 8);
		micron_canvas_atascii_print(out_micron, "Are you a vixen?", c += 16, c - 32, x, y += 8);
		micron_canvas_atascii_print(out_micron, "Are you a viking valyrie?", c += 16, c - 32, x, y += 8);
		micron_canvas_atascii_print(out_micron, "On penicillin?", c += 16, c - 32, x, y += 8);
		//micron_canvas_atascii_print(out_micron, "With Richard Nixon?", c += 16, c - 32, x, y += 8);
		micron_canvas_atascii_print(out_micron, "In a robotic fantasy?", c += 16, c - 32, x, y += 8);
	}
#endif

	//all chars
	for (int32_t y = 0; y < 16; ++y)
	{
		for (int32_t x = 0; x < 16; ++x)
		{
			micron_canvas_atascii_char_key(out_micron, uint8_t(x + y * 16), 255, x * 8, y * 8);
		}
	}

	{
		static struct sprite_t
		{
			int32_t x;
			int32_t xs;
			int32_t y;
			int32_t ys;
		} sprites[256];
		static bool init = 1;
		if (init)
		{
			for (int32_t i = 0; i < _countof(sprites); ++i)
			{
				sprites[i].x = __prng() % (out_micron.canvas_width - 8);
				sprites[i].y = __prng() % (240 - 8);

				sprites[i].xs = __prng() % 3;
				if (0 == sprites[i].xs)
					sprites[i].xs = 1;
				sprites[i].ys = __prng() % 3;
				if (0 == sprites[i].ys)
					sprites[i].ys = 1;
			}
			init = 0;
		}
		for (
			sprite_t& s : sprites
			)
		{
			s.x += s.xs;
			if (s.x < 0)
			{
				s.x = 0;
				s.xs = -s.xs;
			}
			else if (s.x > out_micron.canvas_width - 8)
			{
				s.x = out_micron.canvas_width - 8;
				s.xs = -s.xs;
			}
			s.y += s.ys;
			if (s.y < 0)
			{
				s.y = 0;
				s.ys = -s.ys;
			}
			else if (s.y > 240 - 8)
			{
				s.y = 240 - 8;
				s.ys = -s.ys;
			}
			micron_canvas_atascii_char_key(out_micron, 128, uint8_t(&s - sprites) + (uint8_t)in_model.tick, s.x, s.y);
		}
	}
}
#endif

//public
//public
//public
//public

wmdl_app_event_t wmdl_controller_tick(
	const wmdl_assets_t& in_assets,
	wmdl_model_t& out_model, wmdl_controller_t& out_controller, micron_t& out_micron)
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
		out_controller.tile_anim_tick += TILE_FPS * WMDL_SECONDS_PER_TICK;
		while (out_controller.tile_anim_tick > 1)
		{
			for (
				uint32_t tile = 0;
				tile < WMDL_MAX_TILE;
				++tile
				)
			{
				const uint32_t TILE = out_controller.current_tiles[tile];
				out_controller.current_tiles[tile] = in_assets.anim_target[TILE];
			}

			out_controller.tile_anim_tick -= 1;
		}
	}

	//this is single pass IMGUI, so input and output are intertwined
	wmdl_app_event_t result = wmdl_app_event_t::NOTHING;
	if (out_model.play_bit)
		__play_update(out_micron, in_assets, out_model, out_controller, out_micron);
	else
		result = __idle_update(in_assets, out_micron);

#if ATASCII_TEST
	__atascii_test(out_model, out_micron);
#endif

	return result;
}

void wmdl_controller_on_load_new_world(wmdl_controller_t& out_controller, micron_t& out_micron)
{
	out_micron.sound_plays.push_back(WMDL_SND_SPAWN);

	out_controller.play_menu = false;

	for (
		wmdl_snowflake_t* flake = out_controller.flakes;
		flake < out_controller.flakes + _countof(out_controller.flakes);
		++flake
		)
	{
		flake->x = wmdl_random_unit() * out_micron.canvas_width;
		flake->y = wmdl_random_unit() * out_micron.canvas_height;
		flake->sx = wmdl_random_unit() * 50 - 25;
		flake->sy = wmdl_random_unit() * 75 + 25;
		flake->t = uint32_t(wmdl_random_unit() * 6);
	}
}

void wmdl_controller_death_create(
	const micron_bitmap_t* in_bitmap, const float in_x, const float in_y, const int32_t in_width, const int32_t in_height, const int32_t in_src_x, const int32_t in_src_y,
	wmdl_controller_t& out_controller)
{
	assert(out_controller.num_deaths < _countof(out_controller.deaths));
	wmdl_death_t* death = out_controller.deaths + out_controller.num_deaths++;
	death->bm = in_bitmap;
	death->x = in_x;
	death->y = in_y;
	death->s = -350;
	death->w = in_width;
	death->h = in_height;
	death->src_x = in_src_x;
	death->src_y = in_src_y;
}
