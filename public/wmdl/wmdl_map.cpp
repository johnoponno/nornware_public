#include "stdafx.h"

#include "../minyin/minyin.h"
#include "wmdl_model.h"

static void __map_grid(
	const int32_t in_x, const int32_t in_y, const uint8_t in_color,
	minyin_bitmap_t& out_canvas)
{
	uint8_t* p = out_canvas.pixels + (out_canvas.width - WMDL_WORLD_WIDTH * 2) / 2 + 2 * in_x + (out_canvas.height - 2 * WMDL_WORLD_HEIGHT + 2 * in_y) * out_canvas.width;
	p[0] = in_color;
	p[1] = in_color;
	p[0 + out_canvas.width] = in_color;
	p[1 + out_canvas.width] = in_color;
}

static void __map_offset(
	const uint32_t in_offset, const uint8_t in_color,
	minyin_bitmap_t& out_canvas)
{
	const int32_t X = in_offset % WMDL_WORLD_WIDTH;
	const int32_t Y = in_offset / WMDL_WORLD_WIDTH;
	__map_grid(X, Y, in_color, out_canvas);
}

static void __map_worldspace(
	const float in_world_x, const float in_world_y, const uint8_t in_color,
	minyin_bitmap_t& out_canvas)
{
	__map_offset(wmdl_world_to_offset(in_world_x, in_world_y), in_color, out_canvas);
}

//public
//public
//public
//public

void wmdl_map(
	const wmdl_model_t& in_model,
	minyin_bitmap_t& out_canvas)
{
	struct
	{
		int32_t x;
		int32_t y;
	} checkpoints[16];
	uint32_t checkpoint_count = 0;

	//tiles
	for (int32_t y = 0; y < WMDL_WORLD_HEIGHT; ++y)
	{
		for (int32_t x = 0; x < WMDL_WORLD_WIDTH; ++x)
		{
			const wmdl_tile_t& TILE = wmdl_model_get_tile(in_model, x, y);
			switch (TILE.index)
			{
			default:
				if (wmdl_model_get_tile_info(in_model, TILE, true).hero_pass)
					__map_grid(x, y, 15, out_canvas);
				else
					__map_grid(x, y, 0, out_canvas);
				break;

			case WMDL_LOGIC_INDEX_SERVER:
				if (wmdl_model_is_server_fixed(in_model.world, wmdl_grid_to_offset(x, y), in_model.hero))
				{
					__map_grid(x, y, 95, out_canvas);
					__map_grid(x, y - 1, 95, out_canvas);
				}
				else
				{
					__map_grid(x, y, 63, out_canvas);
					__map_grid(x, y - 1, 63, out_canvas);
				}
				break;

			case WMDL_LOGIC_INDEX_KEY0:
			case WMDL_LOGIC_INDEX_KEY0BLOCK:
				if (in_model.hero.keys & WMDL_HERO_KEYBITS_0)
					__map_grid(x, y, 0, out_canvas);
				else
					__map_grid(x, y, 63, out_canvas);
				break;

			case WMDL_LOGIC_INDEX_KEY1:
			case WMDL_LOGIC_INDEX_KEY1BLOCK:
				if (in_model.hero.keys & WMDL_HERO_KEYBITS_1)
					__map_grid(x, y, 0, out_canvas);
				else
					__map_grid(x, y, 95, out_canvas);
				break;

			case WMDL_LOGIC_INDEX_KEY2:
			case WMDL_LOGIC_INDEX_KEY2BLOCK:
				if (in_model.hero.keys & WMDL_HERO_KEYBITS_2)
					__map_grid(x, y, 0, out_canvas);
				else
					__map_grid(x, y, 159, out_canvas);
				break;

			case WMDL_LOGIC_INDEX_CHECKPOINT:
			case WMDL_LOGIC_INDEX_CHECKPOINT_CURRENT:
				assert(checkpoint_count < _countof(checkpoints));
				checkpoints[checkpoint_count++] = { x, y };
				__map_grid(x, y, 0, out_canvas);
				break;
			}
		}
	}

	//checkpoints
	for (
		const auto* CP = checkpoints;
		CP < checkpoints + checkpoint_count;
		++CP
		)
	{
		const bool BLINK_TICK = (in_model.tick / 6) % 2;
		const bool HERO_IS_CHECKPOINT = wmdl_hero_is_checkpoint(in_model.hero, CP->x, CP->y);
		if (BLINK_TICK && HERO_IS_CHECKPOINT)
			__map_grid(CP->x, CP->y + 1, 255, out_canvas);
		else
			__map_grid(CP->x, CP->y + 1, 255 - 32, out_canvas);
	}

	//portals
	for (
		const wmdl_portal_t* PORTAL = in_model.level.portals;
		PORTAL < in_model.level.portals + in_model.level.num_portals;
		++PORTAL
		)
	{
		if (in_model.hero.fixed_servers.count >= PORTAL->server_count)
		{
			__map_offset(PORTAL->offset, 255, out_canvas);
			__map_offset(PORTAL->offset - WMDL_WORLD_WIDTH, 255, out_canvas);
		}
		else
		{
			__map_offset(PORTAL->offset, 79, out_canvas);
			__map_offset(PORTAL->offset - WMDL_WORLD_WIDTH, 79, out_canvas);
		}
	}

	//enemies
	for (
		const wmdl_enemy_t* E = in_model.enemy;
		E < in_model.enemy + in_model.num_enemy;
		++E
		)
	{
		__map_worldspace(E->x, E->y, 255 - 1 * 16, out_canvas);
	}

	//hero
	__map_worldspace(in_model.hero.x, in_model.hero.y, 255, out_canvas);
}
