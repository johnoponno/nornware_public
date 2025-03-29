#pragma once

#include "../microlib/c_fixed_string.h"
#include "../microlib/c_prng.h"
#include "../microlib/c_vector.h"
#include "m_immutable.h"

struct c_blob_t;

namespace mlm
{
	struct vc_fx_t
	{
		union
		{
			struct
			{
				c_vec2f_t speed;
				float time_to_die;
				uint32_t tile;
			} tile;

			struct
			{
				c_vec2f_t speed;
				float time_to_die;
				float size;
				float time_to_live;
				uint8_t color1;
				uint8_t color2;
			} smoke;

			struct
			{
				c_vec2f_t speed;
				float time_to_die;
				uint32_t frame : 8;
				uint32_t slowing : 1;
			} blade;

			struct
			{
				c_vec2f_t speed;
				float time_to_die;
				uint8_t frame;
			} gib;

			struct
			{
				float start_time;
				const c_blob_t* bitmap;
				uint32_t ends_with_gibs : 1;
				uint32_t mirror : 1;
			} animated;
		};
		c_vec2f_t position;
		uint32_t type;
	};

	struct vc_text_t
	{
		c_path_t text;
		int32_t x;
		int32_t y;
	};

	struct vc_sound_play_t
	{
		c_vec2f_t event_position;
		uint32_t id;
	};

	struct vc_fatpack_t
	{
		explicit vc_fatpack_t();
		vc_fx_t* fx_acquire();

		float hero_anim;

		vc_text_t gui_text[16];
		uint32_t gui_num_text;

		uint8_t idle_screen;

		uint32_t last_visit_offset;

		uint32_t current_tiles[MLM_M_MAX_TILE];
		float tile_anim_tick;

		uint32_t bit_display_visited : 1;
		uint32_t bit_display_hero_tests : 1;
		uint32_t bit_camera_lerp : 1;
		uint32_t bit_display_cambits : 1;
		uint32_t bit_play_music : 1;

		std::vector<vc_fx_t> fx;

		//instead of calculating on the fly
		uint32_t cache_total_flowers;
		uint32_t cache_picked_flowers;

		//gui uses pointers to text, so all dynamic strings need memory
		char flower_count_string[16];
		char elapsed_string[256];

		c_xorshift128_t prng;
	};
}
