#include "stdafx.h"
#include "grafxkid.h"

#include "../microlib/fs.h"
#include "../microlib/paletas.h"
#include "../micron.h"

#define FASTFLOOR(x) ( ((x)>0) ? ((int32_t)x) : ((int32_t)x-1 ) )
#define FADE(t) ( t * t * t * ( t * ( t * 6 - 15 ) + 10 ) )
#define LERP(t, a, b) ((a) + (t)*((b)-(a)))

static constexpr uint8_t PERM[] =
{
	151, 160, 137, 91, 90, 15,
	131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
	190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
	88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
	77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
	102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
	135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
	5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
	223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
	129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
	251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
	49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,
	151, 160, 137, 91, 90, 15,
	131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
	190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
	88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
	77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
	102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
	135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
	5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
	223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
	129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
	251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
	49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

static uint32_t __jenkins_hash(const void* in_bytes, const size_t in_size, const uint32_t in_hash)
{
	uint32_t result = in_hash;

	const uint8_t* b = (uint8_t*)in_bytes;
	for (
		size_t i = 0;
		i < in_size;
		++i
		)
	{
		result += b[i];
		result += (result << 10);
		result ^= (result >> 6);
	}

	result += (result << 3);
	result ^= (result >> 11);
	result += (result << 15);

	return result;
}

static float __grad1(const int32_t in_hash, const float in_x)
{
	int32_t h = in_hash & 15;
	float grad = 1.0f + (h & 7);  // Gradient value 1.0, 2.0, ..., 8.0
	if (h & 8) grad = -grad;         // and a random sign for the gradient
	return (grad * in_x);           // Multiply the gradient with the distance
}

static float __sg_noise1(const float in_x)
{
	int32_t ix0, ix1;
	float fx0, fx1;
	float s, n0, n1;

	ix0 = FASTFLOOR(in_x); // Integer part of x
	fx0 = in_x - ix0;       // Fractional part of x
	fx1 = fx0 - 1.0f;
	ix1 = (ix0 + 1) & 0xff;
	ix0 = ix0 & 0xff;    // Wrap to 0..255

	s = FADE(fx0);

	n0 = __grad1(PERM[ix0], fx0);
	n1 = __grad1(PERM[ix1], fx1);
	return 0.188f * (LERP(s, n0, n1));
}

#define TILE_ASPECT 16
#define SCREEN_TILES_X 16
#define SRC_TILES_X 13
#define SRC_TILES_Y 13
#define BG_WIDTH 160
#define COLOR_KEY 15
#define TILE_HERO_JUMP 23

static constexpr int32_t TILE_HERO_IDLE[] =
{
	36, 37, 38,
	25, 25, 25, 25,
	38,
	25, 25, 25, 25, 25, 25, 25, 25,
	36, 25, 25, 25, 36,
	25, 25, 25, 25, 25, 25, 25, 25,
	25, 25, 25, 25, 25, 25, 25, 25,
	38,
	25, 25, 25, 25, 25, 25, 25, 25,
	38,
	25, 25, 25, 25,
};
static constexpr int32_t TILE_HERO_RUN[] =
{
	10, 11, 12,
};
static constexpr int32_t TILE_COIN[] =
{
	141, 142, 153, 154,
};

static void __blit_tile(
	const grafxkid_t& in_game, const int32_t in_tile, const int32_t in_x, const int32_t in_y,
	micron_t& out_micron)
{
	micron_blit_key_clip(out_micron, 15, in_game.gfx, in_x, in_y, TILE_ASPECT, TILE_ASPECT, (in_tile % SRC_TILES_X) * TILE_ASPECT, (in_tile / SRC_TILES_X) * TILE_ASPECT);
}

static bool __obs(const grafxkid_t& in_game, const int32_t in_local_x_tile)
{
	return __sg_noise1((in_game.mu_hero_x / TILE_ASPECT + in_local_x_tile) * .05f) < 0.f;
}

uint32_t __obstacle_flags(const grafxkid_t& in_game, const int32_t local_x_tile)
{
	uint32_t hash_input;
	uint32_t flags = 0;

	hash_input = in_game.mu_hero_x / TILE_ASPECT + local_x_tile - 1;
	if (
		(__jenkins_hash(&hash_input, sizeof(hash_input), 0) % 2) &&
		__obs(in_game, local_x_tile - 1)
		)
		flags |= 1;

	hash_input = in_game.mu_hero_x / TILE_ASPECT + local_x_tile;
	if (
		(__jenkins_hash(&hash_input, sizeof(hash_input), 0) % 2) &&
		__obs(in_game, local_x_tile)
		)
		flags |= 2;

	hash_input = in_game.mu_hero_x / TILE_ASPECT + local_x_tile + 1;
	if (
		(__jenkins_hash(&hash_input, sizeof(hash_input), 0) % 2) &&
		__obs(in_game, local_x_tile + 1)
		)
		flags |= 4;

	return flags;
}

static bool __brick(const grafxkid_t& in_game, const int32_t local_x_tile)
{
	if (2 & __obstacle_flags(in_game, local_x_tile))
	{
		const uint32_t HASH_INPUT = in_game.mu_hero_x / TILE_ASPECT + local_x_tile;
		const uint32_t HASH = __jenkins_hash(&HASH_INPUT, sizeof(HASH_INPUT), 0);
		return 1 == (HASH % 3);
	}

	return false;
}

static bool __input_left(const micron_t& in_micron)
{
	return
		micron_key_is_down(in_micron, 'A') ||
		in_micron.pads[0].left_stick_x < 0.f;
}

static bool __input_right(const micron_t& in_micron)
{
	return
		micron_key_is_down(in_micron, 'D') ||
		in_micron.pads[0].left_stick_x > 0.f;
}

static bool __input_sprint(const micron_t& in_micron)
{
	return
		micron_key_is_down(in_micron, 'L') ||
		in_micron.pads[0].button_x ||
		in_micron.pads[0].button_y;
}

static bool __input_low_jump(const micron_t& in_micron)
{
	return
		micron_key_is_down(in_micron, 'K') ||
		in_micron.pads[0].button_a;
}

static bool __input_high_jump(const micron_t& in_micron)
{
	return
		micron_key_is_down(in_micron, 'J') ||
		in_micron.pads[0].button_b;
}

struct fs_pcx_header_t
{
	uint8_t  manufacturer;				/* 0x0A: ZSoft */
	uint8_t  version;					/* 0x02 with palette, 0x03 without palette, 0x05 24 bit pcx or paletted */
	uint8_t  encoding;					/* 0x01: PCX RLE */
	uint8_t  bits_per_pixel;			/* number of bits per pixel 1, 2, 4, 8 */
	uint16_t x_min;						/* window */
	uint16_t y_min;
	uint16_t x_max;
	uint16_t y_max;
	uint16_t h_dpi;
	uint16_t v_dpi;
	uint8_t  colormap[48];				/* first 16 colors, R, G, B as 8 bit values */
	uint8_t  reserved;					/* 0x00 */
	uint8_t  number_of_planes;			/* number of planes */
	uint16_t bytes_per_line;			/* number of bytes that represent a scanline plane == must be an even number! */
	uint16_t palette_info;				/* 0x01 = color/bw, 0x02 = grayscale */
	uint16_t horizontal_screen_size;	/* horizontal screen size */
	uint16_t vertical_screen_size;		/* vertical screen size */
	uint8_t  filler[54];				/* padding to 128 bytes */
};

/*
static const fs_pcx_header_t* __pcx_header(const void* in_file)
{
	return (fs_pcx_header_t*)in_file;
}

static int32_t __pcx_width(const fs_pcx_header_t* in_header)
{
	return in_header->x_max - in_header->x_min + 1;
}

static int32_t __pcx_height(const fs_pcx_header_t* in_header)
{
	return in_header->y_max - in_header->y_min + 1;
}

//this assumes a 256 color palette at the end of the file, depends on file type
static const uint8_t* __pcx_palette(const void* in_native, const uint32_t in_size)
{
	return (uint8_t*)in_native + in_size - 768;
}
*/

//public
//public
//public
//public

bool grafxkid_init(micron_t& out_micron, grafxkid_t& out_game)
{
	out_game = {};

	out_game.im_bg_split_y = 72;
	out_game.im_bg_anim = 1;
	out_game.mu_bg_split = 1;

	out_micron.canvas_width = 256;
	out_micron.canvas_height = 144;

	{
		paletas_t p;
		p.item("grafxkid_grassland.tga", out_game.gfx);
		if (!p.calculate(32, out_micron, out_game.gfx_memory))
			return false;
	}

	assert(SCREEN_TILES_X == out_micron.canvas_width / TILE_ASPECT);
	assert(SRC_TILES_X == out_game.gfx.width / TILE_ASPECT);
	assert(SRC_TILES_Y == out_game.gfx.height / TILE_ASPECT);

#if 0
	{
		c_blob_t pcx = fs_file_contents("C:/Users/johno/Documents/MicroLoMania/ArtAssets/tilesetGroundAndSpikes.pcx");
		if (pcx.data)
		{
			const fs_pcx_header_t* header = __pcx_header(pcx.data);
			::memset(&out_micron.palette, 0, sizeof(out_micron.palette));
			for (
				uint32_t i = 0;
				i < 16;
				++i
				)
			{
				out_micron.palette[i].r = header->colormap[i * 3 + 0];
				out_micron.palette[i].g = header->colormap[i * 3 + 1];
				out_micron.palette[i].b = header->colormap[i * 3 + 2];
			}
			delete[] pcx.data;
		}
	}
#endif

	return true;
}

bool grafxkid_tick(micron_t& out_micron, grafxkid_t& out_game)
{
	/*
	out_micron.canvas_width = 256;
	out_micron.canvas_height = 144;
	*/

	int32_t moving = 0;

	//update
	{
		++out_game.mu_tick;

		if (__input_left(out_micron))
		{
			--out_game.mu_hero_x;
			moving = 1;
			if (__input_sprint(out_micron))
			{
				--out_game.mu_hero_x;
				moving = 2;
			}
			out_game.mu_hero_left = 1;
			if (out_game.mu_hero_x < 0)
			{
				out_game.mu_hero_x = 0;
				moving = 0;
			}
		}

		if (__input_right(out_micron))
		{
			++out_game.mu_hero_x;
			moving = 1;
			if (__input_sprint(out_micron))
			{
				++out_game.mu_hero_x;
				moving = 2;
			}
			out_game.mu_hero_left = 0;
		}

		if (0.f == out_game.mu_hero_speed_y)
		{
			if (__input_high_jump(out_micron))
				out_game.mu_hero_speed_y = -200.f;
			if (__input_low_jump(out_micron))
				out_game.mu_hero_speed_y = -150.f;
		}

		out_game.mu_hero_speed_y += 500.f * GRAFXKID_SECONDS_PER_TICK;
		const float OLD_Y = out_game.mu_hero_position_y;
		out_game.mu_hero_position_y += out_game.mu_hero_speed_y * GRAFXKID_SECONDS_PER_TICK;
		if (out_game.mu_hero_position_y > 0.f)
		{
			out_game.mu_hero_position_y = 0.f;
			out_game.mu_hero_speed_y = 0.f;
		}
		if (
			out_game.mu_hero_speed_y > 0.f &&
			OLD_Y <= -TILE_ASPECT &&
			out_game.mu_hero_position_y > -TILE_ASPECT &&
			(2 & __obstacle_flags(out_game, SCREEN_TILES_X / 2))
			)
		{
			out_game.mu_hero_position_y = -TILE_ASPECT;
			out_game.mu_hero_speed_y = 0.f;
		}
		if (
			out_game.mu_hero_speed_y > 0.f &&
			OLD_Y <= -TILE_ASPECT * 2 &&
			out_game.mu_hero_position_y > -TILE_ASPECT * 2 &&
			__brick(out_game, SCREEN_TILES_X / 2)
			)
		{
			out_game.mu_hero_position_y = -TILE_ASPECT * 2;
			out_game.mu_hero_speed_y = 0.f;
		}
	}

	//draw
	{
		//bg
		if (!micron_key_is_down(out_micron, 'Q'))
		{
			const int32_t SCROLL_SLOW = -((out_game.mu_hero_x / 8) % BG_WIDTH);
			const int32_t SCROLL_FAST = -((out_game.mu_hero_x / 4) % BG_WIDTH);
			for (
				int32_t x = 0;
				x < 3;
				++x
				)
			{
				const int32_t X_FAST = x * BG_WIDTH + SCROLL_FAST;

				if (out_game.mu_bg_split)
				{
					const int32_t X_SLOW = x * BG_WIDTH + SCROLL_SLOW;
					micron_blit_clip(out_micron, out_game.gfx, X_SLOW, 0, BG_WIDTH, out_game.im_bg_split_y, 0, 0);
					micron_blit_clip(out_micron, out_game.gfx, X_FAST, out_game.im_bg_split_y, BG_WIDTH, out_micron.canvas_height - out_game.im_bg_split_y, 0, out_game.im_bg_split_y);
				}
				else
				{
					micron_blit_clip(out_micron, out_game.gfx, X_FAST, 0, BG_WIDTH, out_micron.canvas_height, 0, 0);
				}

				if (out_game.im_bg_anim)
				{
					if ((out_game.mu_tick / 20) % 2)
						micron_blit_key_clip(out_micron, COLOR_KEY, out_game.gfx, X_FAST, 96, BG_WIDTH, 32, 0, 144);
					else
						micron_blit_key_clip(out_micron, COLOR_KEY, out_game.gfx, X_FAST, 96, BG_WIDTH, 32, 0, 176);
				}
			}
		}
		else
		{
			micron_canvas_clear(out_micron, 7);
		}

		//trees
		if (!micron_key_is_down(out_micron, 'W'))
		{
			constexpr int32_t DIV = 3;
			const int32_t SCROLL = -((out_game.mu_hero_x / DIV) % 32);
			for (
				int32_t x = 0;
				x < (out_micron.canvas_width / 32) + 1;
				++x
				)
			{
				const int32_t HASH_INPUT = out_game.mu_hero_x / 32 / DIV + x;
				if (__jenkins_hash(&HASH_INPUT, sizeof(HASH_INPUT), 0) % 2)
					micron_blit_key_clip(out_micron, COLOR_KEY, out_game.gfx, x * 32 + SCROLL, out_micron.canvas_height - TILE_ASPECT * 3, 32, 32, 160, 48);
			}
		}

		//bushes
		if (!micron_key_is_down(out_micron, 'E'))
		{
			const int32_t SCROLL = -((out_game.mu_hero_x / 2) % TILE_ASPECT);
			for (
				int32_t x = 0;
				x < SCREEN_TILES_X + 1;
				++x
				)
			{
				const int32_t HASH_INPUT = out_game.mu_hero_x / TILE_ASPECT / 2 + x;
				if (__jenkins_hash(&HASH_INPUT, sizeof(HASH_INPUT), 0) % 2)
					__blit_tile(out_game, 64, x * TILE_ASPECT + SCROLL, out_micron.canvas_height - TILE_ASPECT * 2, out_micron);
			}
		}

		//tiles
		if (!micron_key_is_down(out_micron, 'R'))
		{
			const int32_t SCROLL = -(out_game.mu_hero_x % TILE_ASPECT);
			for (
				int32_t x = 0;
				x < SCREEN_TILES_X + 1;
				++x
				)
			{
				const uint32_t OBSTACLE_FLAGS = __obstacle_flags(out_game, x);

				//bottom
				__blit_tile(out_game, 127, x * TILE_ASPECT + SCROLL, out_micron.canvas_height - TILE_ASPECT, out_micron);

				//obstacles / decorations
				{

					switch (OBSTACLE_FLAGS)
					{
						//fence
#if 1
					default:
						__blit_tile(out_game, 76, x * TILE_ASPECT + SCROLL, out_micron.canvas_height - TILE_ASPECT * 2, out_micron);
						break;
#endif

					case 2:
						__blit_tile(out_game, 128, x * TILE_ASPECT + SCROLL, out_micron.canvas_height - TILE_ASPECT * 2, out_micron);
						break;

					case 3:
						__blit_tile(out_game, 90, x * TILE_ASPECT + SCROLL, out_micron.canvas_height - TILE_ASPECT * 2, out_micron);
						break;

					case 5:
						__blit_tile(out_game, 76, x * TILE_ASPECT + SCROLL, out_micron.canvas_height - TILE_ASPECT * 2, out_micron);
						break;

					case 6:
						__blit_tile(out_game, 88, x * TILE_ASPECT + SCROLL, out_micron.canvas_height - TILE_ASPECT * 2, out_micron);
						break;

					case 7:
						__blit_tile(out_game, 89, x * TILE_ASPECT + SCROLL, out_micron.canvas_height - TILE_ASPECT * 2, out_micron);
						break;
					}

					if (2 & OBSTACLE_FLAGS)
					{
						const uint32_t HASH_INPUT = out_game.mu_hero_x / TILE_ASPECT + x;
						const uint32_t HASH = __jenkins_hash(&HASH_INPUT, sizeof(HASH_INPUT), 0);
						switch (HASH % 3)
						{
						case 1:
							__blit_tile(out_game, 115, x * TILE_ASPECT + SCROLL, out_micron.canvas_height - TILE_ASPECT * 3, out_micron);
							break;

						case 2:
							__blit_tile(out_game, 64, x * TILE_ASPECT + SCROLL, out_micron.canvas_height - TILE_ASPECT * 3, out_micron);
							break;
						}
					}
				}

				//coins
				{
					const uint32_t HASH_INPUT = out_game.mu_hero_x / TILE_ASPECT + x;
					if (HASH_INPUT > SCREEN_TILES_X)
					{
						const uint32_t HASH = __jenkins_hash(&HASH_INPUT, sizeof(HASH_INPUT), 0);
						if (0 == (HASH % 6))
						{
							const uint32_t ANIM = out_game.mu_tick / 6;
							if (OBSTACLE_FLAGS)
								__blit_tile(out_game, TILE_COIN[ANIM % _countof(TILE_COIN)], x * TILE_ASPECT + SCROLL, out_micron.canvas_height - TILE_ASPECT * 5, out_micron);
							else
								__blit_tile(out_game, TILE_COIN[ANIM % _countof(TILE_COIN)], x * TILE_ASPECT + SCROLL, out_micron.canvas_height - TILE_ASPECT * 4, out_micron);
						}
					}
				}
			}
		}

		//hero
		if (!micron_key_is_down(out_micron, 'T'))
		{
			const int32_t X = int32_t((SCREEN_TILES_X * .5f - .5f) * TILE_ASPECT);
			const int32_t Y = out_micron.canvas_height - TILE_ASPECT * 2 + (int32_t)out_game.mu_hero_position_y;
			int32_t tile;
			if (out_game.mu_hero_speed_y)
			{
				tile = TILE_HERO_JUMP;
				if (out_game.mu_hero_left)
					micron_blit_key_clip_horizontal_mirror(out_micron, COLOR_KEY, out_game.gfx, X, Y, TILE_ASPECT, TILE_ASPECT, (tile % SRC_TILES_X) * TILE_ASPECT, (tile / SRC_TILES_X) * TILE_ASPECT);
				else
					__blit_tile(out_game, tile, X, Y, out_micron);
			}
			else
			{
				switch (moving)
				{
				default:
					tile = TILE_HERO_IDLE[(out_game.mu_tick / 6) % _countof(TILE_HERO_IDLE)];
					break;

				case 1:
					tile = TILE_HERO_RUN[(out_game.mu_tick / 6) % _countof(TILE_HERO_RUN)];
					break;

				case 2:
					tile = TILE_HERO_RUN[(out_game.mu_tick / 3) % _countof(TILE_HERO_RUN)];
					break;
				}
				if (out_game.mu_hero_left)
					micron_blit_key_clip_horizontal_mirror(out_micron, COLOR_KEY, out_game.gfx, X, Y, TILE_ASPECT, TILE_ASPECT, (tile % SRC_TILES_X) * TILE_ASPECT, (tile / SRC_TILES_X) * TILE_ASPECT);
				else
					__blit_tile(out_game, tile, X, Y, out_micron);
			}
		}
	}

#if 1
	//if (micron_key_is_down(out_micron, MICRON_KEY_SPACE))
		micron_test_screen(out_micron);
#endif

	return !micron_key_downflank(out_micron, MICRON_KEY_ESCAPE);
}

void grafxkid_shutdown(grafxkid_t& out_game)
{
	delete[] out_game.gfx_memory.data;
}
