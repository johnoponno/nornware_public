#include "stdafx.h"

#include "../microlib/c_math.h"
#include "m_mutable.h"
#include "m_work.h"

#if 0

#define GROUND_ACC 10000.f
#define JUMP_X 125.f
#define JUMP_Y -350.f
#define AIR_ACC 750.f

#else

#define GROUND_ACC 5000.f
#define JUMP_X 100.f
#define JUMP_Y -260.f
#define AIR_ACC 175.f

#endif

#define GROUND_FRICTION_X -30.f
#define SPRINT_GROUND_FRICTION_X -20.f
#define DEAD_TIME 1.f
#define ABDUCT_BRAKE 10.f
#define ABDUCT_SPEED -100.f
#define GRAVITY 1300.f
#define HEAVY_MELEE_PAUSE .5f
#define HEAVY_PUSH 4.f

namespace mlm
{
#if 0
	static const MAP_F32(GROUND_ACC, "acceleration on ground", 10000.f);
	static const MAP_F32(GROUND_FRICTION_X, "friction in x when on ground", -30.f);
	static const MAP_F32(SPRINT_GROUND_FRICTION_X, "friction in x when on ground and sprinting", -20.f);
	static const MAP_F32(AIR_ACC, "acceleration in air", 750.000000);
	static const MAP_F32(JUMP_X, "jump impulse in x", 125.f);
	static const MAP_F32(JUMP_Y, "jump impulse in y", -350.f);
	static const MAP_F32(DEAD_TIME, "time between death and respawn", 1.f);
	static const MAP_F32(ABDUCT_BRAKE, "braking friction while being abducted", 10.f);
	static const MAP_F32(ABDUCT_SPEED, "vertical speed of abduction", -100.f);
	static const MAP_F32(GRAVITY, "gravity", 1300.f);
	static const MAP_F32(HEAVY_MELEE_PAUSE, nullptr, .5f);
	static const MAP_F32(HEAVY_PUSH, nullptr, 4.f);
#endif

	static bool __mob_in_char(const m_mob_t& in_mob, const m_mutable_t& in_mu)
	{
		return
			in_mob.position.x > in_mu.hero_position.x - MLM_M_CHAR_HALF_WIDTH &&
			in_mob.position.x < in_mu.hero_position.x + MLM_M_CHAR_HALF_WIDTH &&
			in_mob.position.y > in_mu.hero_position.y - MLM_M_CHAR_HALF_HEIGHT &&
			in_mob.position.y < in_mu.hero_position.y + MLM_M_CHAR_HALF_HEIGHT;
	}

	static uint32_t __bounds_tests(m_mutable_t& out_mu)
	{
		//bounds test
		if (out_mu.hero_position.x < 0)
		{
			out_mu.hero_speed.x = -out_mu.hero_speed.x;
			out_mu.hero_position.x = 0;
		}
		else if (out_mu.hero_position.x >= MLM_M_WORLD_WIDTH * MLM_M_TILE_ASPECT)
		{
			out_mu.hero_speed.x = -out_mu.hero_speed.x;
			out_mu.hero_position.x = MLM_M_WORLD_WIDTH * MLM_M_TILE_ASPECT - 1;
		}

		if (out_mu.hero_position.y < 0)
		{
			out_mu.hero_speed.y = 0.f;
			out_mu.hero_position.y = 0;
		}
		else if (out_mu.hero_position.y >= MLM_M_WORLD_HEIGHT * MLM_M_TILE_ASPECT)
		{
			return M_COD_OUT_OF_BOUNDS;
		}

		return M_NUM_COD;
	}

	static bool __is_solid_left(const m_immutable_t& in_im, const m_mutable_t& in_mu)
	{
		c_vec2f_t test = m_character_get_test_left(false, in_mu);
		bool solid = M_TYPE_SOLID == m_tile_type_at_position(test, true, in_im, in_mu);
		if (!solid)
		{
			test = m_character_get_test_left(true, in_mu);
			solid = M_TYPE_SOLID == m_tile_type_at_position(test, true, in_im, in_mu);
		}

		return solid;
	}

	static bool __is_solid_below(const m_immutable_t& in_im, const m_mutable_t& in_mu, const c_vec2f_t& in_position)
	{
		if (in_mu.hero_speed.y >= 0.f)
		{
			switch (m_tile_type_at_position(in_position, true, in_im, in_mu))
			{
			case M_TYPE_SOLID:
			case M_TYPE_PLATFORM:
				return true;

			default:
				return false;
			}
		}

		return M_TYPE_SOLID == m_tile_type_at_position(in_position, true, in_im, in_mu);
	}

	static bool __is_solid_below(const m_immutable_t& in_im, const m_mutable_t& in_mu)
	{
		bool solid;

		solid = __is_solid_below(in_im, in_mu, m_character_test_below(false, in_mu));
		if (!solid)
			solid = __is_solid_below(in_im, in_mu, m_character_test_below(true, in_mu));

		return solid;
	}

	static void __world_collision_tests(
		const m_immutable_t& in_im,
		m_mutable_t& out_mu, m_events_t& out_events)
	{
		//below test
		if (__is_solid_below(in_im, out_mu))
		{
			if (out_mu.hero_air)
			{
				m_events_push(out_mu.hero_position + c_vec2f_t{ 0.f, (float)MLM_M_CHAR_HALF_HEIGHT }, M_NUM_COD, M_EVT_HERO_LANDED, out_events);
				out_mu.hero_air = false;
			}
			out_mu.hero_speed.y = 0.f;
			out_mu.hero_position.y = (float)((int32_t)out_mu.hero_position.y / MLM_M_TILE_ASPECT);
			out_mu.hero_position.y *= MLM_M_TILE_ASPECT;
			out_mu.hero_position.y += MLM_M_TILE_ASPECT - MLM_M_CHAR_HALF_HEIGHT;
		}
		else
		{
			out_mu.hero_air = true;
		}

		//above test
		if (M_TYPE_SOLID == m_tile_type_at_position(m_character_test_above(out_mu), true, in_im, out_mu))
		{
			out_mu.hero_speed.y = 0.f;
			out_mu.hero_position.y = (float)((int32_t)out_mu.hero_position.y / MLM_M_TILE_ASPECT);
			out_mu.hero_position.y *= MLM_M_TILE_ASPECT;
			out_mu.hero_position.y += MLM_M_CHAR_HALF_HEIGHT;
		}

		//left test (only if moving left)
		if (out_mu.hero_speed.x < 0.f && __is_solid_left(in_im, out_mu))
		{
			out_mu.hero_speed.x = 0.f;
			out_mu.hero_position.x = (float)((int32_t)out_mu.hero_position.x / MLM_M_TILE_ASPECT);
			out_mu.hero_position.x *= MLM_M_TILE_ASPECT;
			out_mu.hero_position.x += MLM_M_CHAR_HALF_WIDTH;
		}

		//right test (only if moving right)
		if (out_mu.hero_speed.x > 0.f && m_character_is_solid_right(in_im, out_mu))
		{
			out_mu.hero_speed.x = 0.f;
			out_mu.hero_position.x = (float)((int32_t)out_mu.hero_position.x / MLM_M_TILE_ASPECT);
			out_mu.hero_position.x *= MLM_M_TILE_ASPECT;
			out_mu.hero_position.x += MLM_M_TILE_ASPECT - MLM_M_CHAR_HALF_WIDTH;
		}
	}

	static uint32_t __mob_collision_tests(m_mutable_t& out_mu)
	{
		for (m_mob_t& mob : out_mu.mobs)
		{
			switch (mob.type)
			{
			case M_LOGIC_WALKER:
				if (__mob_in_char(mob, out_mu))
					return M_COD_WALKER;
				break;

			case M_LOGIC_WALKER2:
				if (__mob_in_char(mob, out_mu))
					return M_COD_WALKER2;
				break;

			case M_LOGIC_JUMPER:
				if (__mob_in_char(mob, out_mu))
					return M_COD_JUMPER;
				break;

			case M_LOGIC_JUMPER2:
				if (__mob_in_char(mob, out_mu))
					return M_COD_JUMPER2;
				break;

			case M_LOGIC_BULLET:
				if (__mob_in_char(mob, out_mu))
				{
					mob.prune = 1;
					return M_COD_WALKER;
				}
			}
		}

		return M_NUM_COD;
	}

	static bool __pressed_input(const uint8_t in_flag, const m_mutable_t& in_mu)
	{
		return
			0 != (in_mu.hero_input & in_flag) &&
			0 == (in_mu.hero_last_input & in_flag);
	}

	static bool __left_input(const m_mutable_t& in_mu)
	{
		return MLM_M_FLAGS_LEFT & in_mu.hero_input;
	}

	static bool __right_input(const m_mutable_t& in_mu)
	{
		return MLM_M_FLAGS_RIGHT & in_mu.hero_input;
	}

	static void __limit_speed(
		const float in_max,
		float& out_speed)
	{
		if (out_speed > in_max)
			out_speed = in_max;
		else if (out_speed < -in_max)
			out_speed = -in_max;
	}

	static bool __is_solid_above(const m_immutable_t& in_im, const m_mutable_t& in_mu)
	{
		c_vec2f_t test = m_character_test_above(in_mu);
		test.y -= 1.f;
		return M_TYPE_SOLID == m_tile_type_at_position(test, true, in_im, in_mu);
	}

	static void __melee_mobs(
		const uint32_t in_tick, const m_immutable_t& in_im,
		m_mutable_t& out_mu, m_events_t& out_events)
	{
		const float X_OFFSET = out_mu.hero_mirror ? 0.f : -MLM_M_MELEE_RANGE_X;
		const float PUSH = out_mu.hero_mirror ? HEAVY_PUSH : -HEAVY_PUSH;
		const c_vec2f_t MELEE_MIN{ out_mu.hero_position.x + X_OFFSET, out_mu.hero_position.y - MLM_M_MELEE_RANGE_Y };
		const c_vec2f_t MELEE_MAX{ out_mu.hero_position.x + X_OFFSET + MLM_M_MELEE_RANGE_X, out_mu.hero_position.y + MLM_M_MELEE_RANGE_Y };

		for (m_mob_t& mob : out_mu.mobs)
		{
			if (c_inside_rectf(mob.position, MELEE_MIN, MELEE_MAX))
			{
				switch (mob.type)
				{
				default:
					mob.prune = 1;
					break;

				case M_LOGIC_WALKER:
				case M_LOGIC_WALKER2:
					if (mob.walker.health > 1)
					{
						--mob.walker.health;

						mob.walker.burst = 0;
						mob.walker.fire_time = in_tick * M_SECONDS_PER_TICK + HEAVY_MELEE_PAUSE;

						if (
							out_mu.hero_mirror && mob.walker.speed > 0.f ||
							!out_mu.hero_mirror && mob.walker.speed < 0.f
							)
							mob.walker.speed *= -1.f;
						{
							const float x = mob.position.x;
							mob.position.x += PUSH;
							if (M_TYPE_SOLID == m_tile_type_at_position(mob.position, true, in_im, out_mu))
								mob.position.x = x;
						}

						m_events_push(mob.position, M_NUM_COD, M_EVT_MELEE_HIT, out_events);
					}
					else
					{
						mob.prune = 1;
					}
					break;
				}
			}
		}
	}

	static void __normal_update(
		const m_immutable_t& in_im, const uint32_t in_tick,
		m_mutable_t& out_mu, m_events_t& out_events)
	{
		//switches
		if (__pressed_input(MLM_M_FLAGS_DOWN, out_mu))
		{
			m_do_switches(in_im, out_mu, out_events);
		}

		//adjust physics for liquid
		const bool IN_LIQUID = M_TYPE_LIQUID == m_tile_type_at_position(out_mu.hero_position, true, in_im, out_mu);
		float current_max_air_speed_x = M_CHAR_MAX_AIR_SPEED_X;
		float current_max_air_speed_y = M_CHAR_MAX_AIR_SPEED_Y;
		float current_gravity = GRAVITY;
		float current_ground_friction_x = GROUND_FRICTION_X;
		if (IN_LIQUID)
		{
			current_max_air_speed_x *= .25f;
			current_max_air_speed_y *= .25f;
			current_gravity *= .25f;
			current_ground_friction_x *= 1.5f;
		}
		else
		{
			if (MLM_M_FLAGS_SPRINT & out_mu.hero_input)
				current_ground_friction_x = SPRINT_GROUND_FRICTION_X;
		}

		//x acceleration
		{
			float acceleration = 0.f;
			if (
				__left_input(out_mu) &&
				!__right_input(out_mu)
				)
			{
				out_mu.hero_mirror = 0;
				acceleration = -1.f;
			}
			else if (
				__right_input(out_mu) &&
				!__left_input(out_mu)
				)
			{
				out_mu.hero_mirror = 1;
				acceleration = 1.f;
			}
			if (acceleration != 0.f)
			{
				//accelerate
				if (out_mu.hero_air)
					out_mu.hero_speed.x += (acceleration * AIR_ACC * M_SECONDS_PER_TICK);
				else
					out_mu.hero_speed.x += (acceleration * GROUND_ACC * M_SECONDS_PER_TICK);
			}
		}

		//x ground brake / limit x speed
		if (out_mu.hero_air)
		{
			__limit_speed(current_max_air_speed_x, out_mu.hero_speed.x);
		}
		else
		{
			out_mu.hero_speed.x += out_mu.hero_speed.x * current_ground_friction_x * M_SECONDS_PER_TICK;
		}

		//jumping
		//if (M_FLAGS_JUMP & mu.hero_input)	//auto-jump
		if (__pressed_input(MLM_M_FLAGS_JUMP, out_mu))	//not auto-jump
		{
			if (out_mu.hero_air)
			{
				if (out_mu.hero_may_wall_jump)
				{
					if (m_character_is_solid_left_more(in_im, out_mu))
					{
						out_mu.hero_speed = { JUMP_X, JUMP_Y };
						m_events_push(out_mu.hero_position, M_NUM_COD, M_EVT_HERO_JUMP, out_events);
					}
					else if (m_character_is_solid_right(in_im, out_mu))
					{
						out_mu.hero_speed = { -JUMP_X, JUMP_Y };
						m_events_push(out_mu.hero_position, M_NUM_COD, M_EVT_HERO_JUMP, out_events);
					}
				}

				//"swimming"
				if (
					IN_LIQUID &&
					__pressed_input(MLM_M_FLAGS_JUMP, out_mu) &&
					!__is_solid_above(in_im, out_mu)
					)
				{
					out_mu.hero_air = true;
					out_mu.hero_speed.y = JUMP_Y;
					m_events_push(out_mu.hero_position, M_NUM_COD, M_EVT_HERO_JUMP, out_events);
				}
			}
			else
			{
				if (!__is_solid_above(in_im, out_mu))
				{
					out_mu.hero_air = true;
					out_mu.hero_speed.y = JUMP_Y;
					m_events_push(out_mu.hero_position, M_NUM_COD, M_EVT_HERO_JUMP, out_events);
				}
			}
		}

		//y gravity / limit y speed
		out_mu.hero_speed.y += current_gravity * M_SECONDS_PER_TICK;
		__limit_speed(current_max_air_speed_y, out_mu.hero_speed.y);

		//movement
		out_mu.hero_position += out_mu.hero_speed * M_SECONDS_PER_TICK;

		//melee
		if (__pressed_input(MLM_M_FLAGS_MELEE, out_mu))
		{
			__melee_mobs(in_tick, in_im, out_mu, out_events);
			out_mu.hero_melee_tick = in_tick;
		}

		//special tests
		{
			uint32_t cod = M_NUM_COD;
			switch (m_tile_type_at_position(out_mu.hero_position, true, in_im, out_mu))
			{
			case M_TYPE_SPIKES:
				cod = M_COD_IMPALED;
				break;

				//case m_special_liquid:
				//	cod = m_cod_drowned;
				//	break;

			case M_TYPE_FIRE:
				cod = M_COD_BURNED;
				break;

			case M_TYPE_ELECTRICITY:
				cod = M_COD_ELECTRINATED;
				break;

			default:
			case M_TYPE_LIQUID:
				cod = __mob_collision_tests(out_mu);

				if (M_NUM_COD == cod)
					__world_collision_tests(in_im, out_mu, out_events);

				if (M_NUM_COD == cod)
					cod = __bounds_tests(out_mu);
				break;
			}

			if (M_NUM_COD != cod)
			{
				m_events_push(out_mu.hero_position, cod, M_EVT_HERO_DIED, out_events);
				out_mu.hero_spawn_time = in_tick * M_SECONDS_PER_TICK + DEAD_TIME;
			}
		}

		out_mu.hero_last_input = out_mu.hero_input;
	}

	void m_character_restart(
		const c_vec2f_t& in_position,
		m_mutable_t& out_mu)
	{
		out_mu.hero_position = in_position;
		out_mu.hero_speed = {};
		out_mu.hero_input = out_mu.hero_last_input = 0;
		out_mu.hero_air = true;
		out_mu.hero_may_wall_jump = false;
		out_mu.hero_spawn_time = -1.f;
		out_mu.hero_melee_tick = UINT32_MAX;
		out_mu.hero_mirror = 0;
	}

	bool m_character_being_abducted(const m_immutable_t& in_im, const m_mutable_t& in_mu)
	{
		return M_TYPE_ABDUCT == m_tile_type_at_position(in_mu.hero_position, true, in_im, in_mu);
	}

	void m_character_update(
		const m_immutable_t& in_im, const uint32_t in_tick,
		m_mutable_t& out_mu, m_events_t& out_events)
	{
		if (m_character_alive(out_mu))
		{
			if (m_character_being_abducted(in_im, out_mu))
			{
				out_mu.hero_speed.x -= out_mu.hero_speed.x * ABDUCT_BRAKE * M_SECONDS_PER_TICK;
				out_mu.hero_speed.y = ABDUCT_SPEED;
				out_mu.hero_position += out_mu.hero_speed * M_SECONDS_PER_TICK;
			}
			else
			{
				__normal_update(in_im, in_tick, out_mu, out_events);
			}
		}
		else
		{
			if (out_mu.hero_spawn_time < in_tick * M_SECONDS_PER_TICK)
			{
				//aTarget.backToCheckPoint(aTime);
				m_back_to_checkpoint(out_mu, out_events);

				out_mu.hero_spawn_time = -1.f;
			}
		}
	}

	c_vec2f_t m_character_test_below(const bool in_second, const m_mutable_t& in_mu)
	{
		c_vec2f_t p{ in_mu.hero_position.x - MLM_M_CHAR_HALF_WIDTH / 2, in_mu.hero_position.y + MLM_M_CHAR_HALF_HEIGHT };

		if (in_second)
			p.x += MLM_M_CHAR_HALF_WIDTH;

		return p;
	}

	c_vec2f_t m_character_test_above(const m_mutable_t& in_mu)
	{
		return { in_mu.hero_position.x, in_mu.hero_position.y - MLM_M_CHAR_HALF_HEIGHT };
	}

	void m_character_set_input(
		const m_immutable_t& in_im, const uint8_t in_flags,
		m_mutable_t& out_mu)
	{
		out_mu.hero_input = in_flags;

		//if just starting jump...
		if (__pressed_input(MLM_M_FLAGS_JUMP, out_mu))
		{
			//if on ground standing next to a wall, no walljump
			if (out_mu.hero_air)
				out_mu.hero_may_wall_jump = true;
			else
				out_mu.hero_may_wall_jump =
				!m_character_is_solid_left_more(in_im, out_mu) &&
				!m_character_is_solid_right(in_im, out_mu);
		}
	}

	bool m_character_alive(const m_mutable_t& out_mu)
	{
		return out_mu.hero_spawn_time < 0.f;
	}

	bool m_character_is_solid_left_more(const m_immutable_t& in_im, const m_mutable_t& in_mu)
	{
		c_vec2f_t test = m_character_get_test_left(false, in_mu);
		test.x -= 1.f;
		bool solid = M_TYPE_SOLID == m_tile_type_at_position(test, true, in_im, in_mu);
		if (!solid)
		{
			test = m_character_get_test_left(true, in_mu);
			test.x -= 1.f;
			solid = M_TYPE_SOLID == m_tile_type_at_position(test, true, in_im, in_mu);
		}

		return solid;
	}

	bool m_character_is_solid_right(const m_immutable_t& in_im, const m_mutable_t& in_mu)
	{
		c_vec2f_t test = m_character_get_test_right(false, in_mu);
		bool solid = M_TYPE_SOLID == m_tile_type_at_position(test, true, in_im, in_mu);
		if (!solid)
		{
			test = m_character_get_test_right(true, in_mu);
			solid = M_TYPE_SOLID == m_tile_type_at_position(test, true, in_im, in_mu);
		}

		return solid;
	}

	c_vec2f_t m_character_get_test_left(const bool in_second, const m_mutable_t& in_mu)
	{
		c_vec2f_t result = { in_mu.hero_position.x - MLM_M_CHAR_HALF_WIDTH, in_mu.hero_position.y - MLM_M_CHAR_HALF_HEIGHT / 2 };
		if (in_second)
			result.y += MLM_M_CHAR_HALF_HEIGHT;
		return result;
	}

	c_vec2f_t m_character_get_test_right(const bool in_second, const m_mutable_t& in_mu)
	{
		c_vec2f_t result = { in_mu.hero_position.x + MLM_M_CHAR_HALF_WIDTH, in_mu.hero_position.y - MLM_M_CHAR_HALF_HEIGHT / 2 };
		if (in_second)
			result.y += MLM_M_CHAR_HALF_HEIGHT;
		return result;
	}
}
