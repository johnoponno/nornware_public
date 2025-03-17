#pragma once

#include "../microlib/microlib.h"

struct micron_t;

struct grafxkid_t
{
	int32_t im_bg_split_y;
	uint32_t im_bg_anim : 1;

	uint32_t mu_tick;
	int32_t mu_hero_x;
	float mu_hero_speed_y;
	float mu_hero_position_y;
	uint32_t mu_hero_left : 1;
	uint32_t mu_bg_split : 1;

	micron_bitmap_memory_t gfx_memory;
	micron_bitmap_t gfx;
};

bool grafxkid_init(micron_t& out_micron, grafxkid_t& out_game);
bool grafxkid_tick(micron_t& out_micron, grafxkid_t& out_game);
void grafxkid_shutdown(grafxkid_t& out_game);

constexpr float GRAFXKID_SECONDS_PER_TICK = 1.f / 60.f;
