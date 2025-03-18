#include "stdafx.h"
#include "plax.h"

#include "../micron.h"
#include "../microlib/paletas.h"

//public
//public
//public
//public

bool plax_init(micron_t& out_micron, plax_t& out_game)
{
	out_game = {};

	out_micron.canvas_width = 600;
	out_micron.canvas_height = 240;

	{
		paletas_t p;
		p.item("plax01.tga", out_game.gfx);
		if (!p.calculate(16, out_micron, out_game.gfx_memory))
			return false;
	}

	out_game.layer[0].draw = 1;
	out_game.layer[0].height = 141;

	out_game.layer[1].draw = 1;
	out_game.layer[1].dst = 108;
	out_game.layer[1].height = 40;

	out_game.layer[2].draw = 1;
	out_game.layer[2].dst = 131;
	out_game.layer[2].height = 37;

	out_game.layer[3].draw = 1;
	out_game.layer[3].dst = 149;
	out_game.layer[3].height = 91;

	out_game.selection = UINT32_MAX;//not editing

	return true;
}

bool plax_tick(micron_t& out_micron, plax_t& out_game)
{
	++out_game.tick;

	//for editing layers
	if (out_game.selection < _countof(out_game.layer))
	{
		if (micron_key_downflank(out_micron, '1'))
			++out_game.selection %= _countof(out_game.layer);

		if (micron_key_downflank(out_micron, '2'))
			--out_game.layer[out_game.selection].dst;
		if (micron_key_downflank(out_micron, '3'))
			++out_game.layer[out_game.selection].dst;

		if (micron_key_downflank(out_micron, '4'))
			--out_game.layer[out_game.selection].height;
		if (micron_key_downflank(out_micron, '5'))
			++out_game.layer[out_game.selection].height;

		if (micron_key_downflank(out_micron, '0'))
			out_game.layer[out_game.selection].draw ^= 1;

		if (micron_key_downflank(out_micron, 'E'))
			out_game.selection = UINT32_MAX;
	}
	else
	{
		if (micron_key_downflank(out_micron, 'E'))
			out_game.selection = 0;
	}

	//draw
	micron_canvas_clear(out_micron, 15);
	{
		int32_t src = 0;
		for (
			uint32_t i = 0;
			i < _countof(out_game.layer);
			++i
			)
		{
			if (out_game.layer[i].draw)
			{
				const float fx = out_game.tick * .1f * (1 + i);
				const int32_t ix = int32_t(fx) % out_game.gfx.width;
				micron_blit_key_clip(out_micron, 7, out_game.gfx, -ix, out_game.layer[i].dst, 0, out_game.layer[i].height, 0, src);
				micron_blit_key_clip(out_micron, 7, out_game.gfx, -ix + out_game.gfx.width, out_game.layer[i].dst, 0, out_game.layer[i].height, 0, src);
			}
			src += out_game.layer[i].height;
		}
	}
	//micron_canvas_visualize_palette(out_micron, 2);

	//for editing layers
	if (out_game.selection < _countof(out_game.layer))
	{
		char slask[256];
		for (
			uint32_t i = 0;
			i < _countof(out_game.layer);
			++i
			)
		{
			::sprintf_s(slask, "(KEY1)layer%u (KEY0)draw=%u (KEY2/KEY3)dst=%d (KEY4/KEY5)height=%d", i, out_game.layer[i].draw, out_game.layer[i].dst, out_game.layer[i].height);
			micron_canvas_atascii_print(out_micron, slask, 0, i == out_game.selection ? 15 : 7, 0, out_micron.canvas_height - (_countof(out_game.layer) - i) * 8);
		}
		micron_canvas_atascii_print(out_micron, "Press E to toggle layer editing...", 0, 3, 0, out_micron.canvas_height - (_countof(out_game.layer) + 1) * 8);
	}
	else
	{
		micron_canvas_atascii_print(out_micron, "Press E to toggle layer editing...", 0, 3, 0, out_micron.canvas_height - 8);
	}

	return !micron_key_downflank(out_micron, MICRON_KEY_ESCAPE);
}

void plax_shutdown(plax_t& out_game)
{
	delete[] out_game.gfx_memory.data;
}
