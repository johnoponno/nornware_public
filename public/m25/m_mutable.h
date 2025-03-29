#pragma once

#include "../microlib/c_prng.h"
#include "../microlib/c_vector.h"
#include "m_immutable.h"

namespace m25
{
	struct m_mob_t
	{
		union
		{
			struct
			{
				float start_time;
				float start;
				float speed;
				uint32_t previous_special;
			} jumper;

			struct
			{
				c_vec2f_t speed;
				c_vec2f_t vector;
				float next_vector_time;
			} swimmer;

			struct
			{
				float speed;
				float fire_time;
				uint32_t burst;
				uint32_t health;
			} walker;

			float bullet_speed;
		};
		c_vec2f_t position;
		uint32_t type;
		uint32_t prune;
	};

	struct m_event_t
	{
		c_vec2f_t position;
		uint32_t argument;
		uint32_t type;
	};

	struct m_events_t
	{
		m_event_t events[16];
		uint32_t count;
	};

	struct m_mutable_t
	{
		explicit m_mutable_t();

		uint8_t world_visited[M_WORLD_SIZE];
		uint8_t world_null_visited;

		c_vec2f_t hero_position;
		c_vec2f_t hero_speed;
		float hero_spawn_time;
		uint32_t hero_melee_tick;
		uint32_t hero_input : 8;
		uint32_t hero_last_input : 8;
		uint32_t hero_air : 1;
		uint32_t hero_may_wall_jump : 1;
		uint32_t hero_mirror : 1;

		uint8_t bi;
		uint8_t tri;
		float exit_time;
		uint32_t checkpoint_offset;
		uint8_t checkpoint_bi;
		uint8_t checkpoint_tri;
		float start_time;
		std::vector<m_mob_t> mobs;
		c_xorshift128_t prng;
	};
}
