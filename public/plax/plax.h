#pragma once

#include "../microlib/microlib.h"

struct micron_t;

struct plax_t
{
	micron_bitmap_memory_t gfx_memory;
	micron_bitmap_t gfx;
	int32_t tick;
	struct
	{
		uint32_t draw;
		int32_t dst;
		int32_t height;
	} layer[4];
	uint32_t selection;
};

bool plax_init(micron_t& out_micron, plax_t& out_game);
bool plax_tick(micron_t& out_micron, plax_t& out_game);
void plax_shutdown(plax_t& out_game);

constexpr float PLAX_SECONDS_PER_TICK = 1.f / 60.f;
