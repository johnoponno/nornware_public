#include "stdafx.h"
#include "wmdl_model.h"

#include "../micron/fs.h"

#define ASSET_COMMON_INFO "model.common"

#define FILE_VERSION 0x00
#define CHUNK_TILE 0
#define CHUNK_START 1
#define CHUNK_BACKGROUND 2
#define CHUNK_PORTAL 3
#define CHUNK_TRACK 4
#define CHUNK_PORTAL_2 5
#define CHUNK_INFO 6

#define HP_CLEAR 0
#define HP_SOLID 1
#define HP_PLATFORM 2
#define S_NOTHING 0
#define S_LETHAL 1
#define S_SLIPPERY 2

#define ROLLER_SPEED 40.f
#define ROLLER_HALF_ASPECT 8

#define PLANT_HALF_ASPECT 8.f

#define FIREDUDE_SPEED 64.f
#define FIREDUDE_RADIUS 12.f

#define BAT_SEEK_RADIUS 120.f
#define BAT_SPEED 96.f
#define BAT_RADIUS 12.f
#define BAT_AVOID_TIME .5f
#define BAT_SCARED_TIME 5.f

#define SLIDER_FRICTION -.5f
#define SLIDER_MIN_SPEED 35.f
#define SLIDER_IMPULSE 200.f
#define SLIDER_RADIUS 8.f

#define ICICLE_PENDING_TIME .5f
#define ICICLE_HALF_ASPECT 12.f//Model.WMDL_TILE_ASPECT / 2;

#define HERO_AIR_ACC 750.f
#define HERO_NORMAL_ACCELERATION 3750.f
#define HERO_SLIPPERY_ACCELERATION 200.f
#define HERO_NORMAL_FRICTION 3000.f
#define HERO_SLIPPERY_FRICTION -.5f
#define HERO_MAX_SPEEDX 200.f
#define HERO_MAX_SPEEDY 500.f
#define HERO_HALF_WIDTH 8
#define HERO_HALF_HEIGHT 12

#define HERO_JUMP -400.f

#define HERO_DEAD_TIME 2.f
#define HERO_WHIP_TIME .1f
#define HERO_WHIP_LENGTH 32
#define HERO_WHIP_X_OFFSET 6.f
#define HERO_WHIP_Y_OFFSET 2.f

#define ENEMY_RESPAWN_TIME 15.f

struct iterator_t
{
	uint8_t* it;
};

static uint32_t __uint(iterator_t& out_iterator)
{
	const uint32_t result = wmdl_change_endianness(*(uint32_t*)out_iterator.it);
	out_iterator.it += sizeof(uint32_t);
	return result;
}

static const char* __utf(iterator_t& out_iterator)
{
	const uint16_t length = wmdl_change_endianness(*(uint16_t*)out_iterator.it);
	out_iterator.it += sizeof(uint16_t);

	const char* result = (char*)out_iterator.it;
	out_iterator.it += length;

	return result;
}

static float __vector_length(const float in_x, const float in_y)
{
	return ::sqrtf(in_x * in_x + in_y * in_y);
}

static void __normalize_vector(float& out_x, float& out_y)
{
	const float l = __vector_length(out_x, out_y);
	out_x /= l;
	out_y /= l;
}

static void __random_vector(float& out_x, float& out_y)
{
	out_x = (wmdl_random_unit() * 2 - 1);
	out_y = (wmdl_random_unit() * 2 - 1);
	__normalize_vector(out_x, out_y);
}

static int32_t __world_to_grid(const float in_world)
{
	return (int32_t)(in_world / WMDL_TILE_ASPECT);
}

static void __logic_spawn(
	const uint32_t in_offset, const uint32_t in_index,
	wmdl_model_t& out_model)
{
	switch (in_index)
	{
	case WMDL_LOGIC_INDEX_SPIKYGREEN:
	case WMDL_LOGIC_INDEX_BLUEBLOB:
	case WMDL_LOGIC_INDEX_BROWNBLOB:
	{
		assert(out_model.num_enemy < _countof(out_model.enemy));
		wmdl_enemy_t* r = out_model.enemy + out_model.num_enemy++;
		r->type = in_index;
		r->spawn_offset = in_offset;
		r->x = wmdl_offset_to_world_x(in_offset);
		r->y = wmdl_offset_to_world_y(in_offset);
		r->spawn_time = wmdl_model_now(out_model);
		r->speed = -ROLLER_SPEED;
	}
	break;

	case WMDL_LOGIC_INDEX_PLANT:
	case WMDL_LOGIC_INDEX_SCORPION:
	{
		assert(out_model.num_enemy < _countof(out_model.enemy));
		wmdl_enemy_t* p = out_model.enemy + out_model.num_enemy++;
		p->type = in_index;
		p->spawn_offset = in_offset;
		p->x = wmdl_offset_to_world_x(in_offset);
		p->y = wmdl_offset_to_world_y(in_offset);
		p->spawn_time = wmdl_model_now(out_model);
	}
	break;

	case WMDL_LOGIC_INDEX_PENGUIN:
	{
		assert(out_model.num_enemy < _countof(out_model.enemy));
		wmdl_enemy_t* s = out_model.enemy + out_model.num_enemy++;
		s->type = in_index;
		s->spawn_offset = in_offset;
		s->x = wmdl_offset_to_world_x(in_offset);
		s->y = wmdl_offset_to_world_y(in_offset);
		s->spawn_time = wmdl_model_now(out_model);
		s->speed = 0.f;
	}
	break;

	case WMDL_LOGIC_INDEX_FIREDUDE:
	{
		assert(out_model.num_enemy < _countof(out_model.enemy));
		wmdl_enemy_t* f = out_model.enemy + out_model.num_enemy++;
		f->type = in_index;
		f->x = wmdl_offset_to_world_x(in_offset);
		f->y = wmdl_offset_to_world_y(in_offset);
		__random_vector(f->vector_x, f->vector_y);
	}
	break;

	case WMDL_LOGIC_INDEX_BAT:
	{
		assert(out_model.num_enemy < _countof(out_model.enemy));
		wmdl_enemy_t* b = out_model.enemy + out_model.num_enemy++;
		b->type = in_index;
		b->spawn_offset = in_offset;
		b->x = wmdl_offset_to_world_x(in_offset);
		b->y = wmdl_offset_to_world_y(in_offset);
		b->state = WMDL_BAT_S_PASSIVE;
		b->change_time = 0.f;
		b->scared = false;
		b->vector_x = 0.f;
		b->vector_y = 0.f;
	}
	break;

	case WMDL_LOGIC_INDEX_ICICLE:
	case WMDL_LOGIC_INDEX_SANDBLOCK:
	{
		assert(out_model.num_enemy < _countof(out_model.enemy));
		wmdl_enemy_t* i = out_model.enemy + out_model.num_enemy++;
		i->type = in_index;
		i->spawn_offset = in_offset;
		i->x = wmdl_offset_to_world_x(in_offset);
		i->y = wmdl_offset_to_world_y(in_offset);
		i->spawn_time = wmdl_model_now(out_model);
		i->state = WMDL_ICICLE_S_IDLE;
		i->change_time = 0.f;
		i->speed = 0.f;
	}
	break;
	}
}

static bool __add_fixed_server(
	const char* in_world, const uint32_t in_offset,
	wmdl_hero_t& out_hero)
{
	if (!wmdl_model_is_server_fixed(in_world, in_offset, out_hero))
	{
		assert(out_hero.fixed_servers.count < _countof(out_hero.fixed_servers.server));
		::strcpy_s(out_hero.fixed_servers.server[out_hero.fixed_servers.count].world, in_world);
		out_hero.fixed_servers.server[out_hero.fixed_servers.count].offset = in_offset;
		++out_hero.fixed_servers.count;
		return true;
	}

	return false;
}

static bool __in_world(const int32_t in_x, const int32_t in_y)
{
	return in_x >= 0 && in_x < WMDL_WORLD_WIDTH && in_y >= 0 && in_y < WMDL_WORLD_HEIGHT;
}

static const wmdl_tile_t& __get_tile(const wmdl_model_t& out_model, const int32_t in_x, const int32_t in_y)
{
	if (__in_world(in_x, in_y))
		return out_model.level.tiles[in_x + in_y * WMDL_WORLD_WIDTH];

	static wmdl_tile_t null_tile{};
	return null_tile;
}

const wmdl_tile_t& __get_tile_for_position(const wmdl_model_t& in_model, const float in_x, const float in_y)
{
	return __get_tile(in_model, __world_to_grid(in_x), __world_to_grid(in_y));
}

static const wmdl_tile_info_t& __get_tile_info_for_position(const wmdl_model_t& in_model, const float in_x, const float in_y)
{
	return wmdl_model_get_tile_info(in_model, __get_tile_for_position(in_model, in_x, in_y), true);
}

static bool __on_slippery(const wmdl_model_t& in_model)
{
	const wmdl_tile_info_t& INFO = __get_tile_info_for_position(in_model, in_model.hero.x, in_model.hero.y + WMDL_TILE_ASPECT);
	return INFO.special == S_SLIPPERY || INFO.hero_pass == HP_CLEAR;
}

static float __acceleration(const wmdl_model_t& in_model)
{
	if (in_model.hero.air_bit)
		return HERO_AIR_ACC;

	if (__on_slippery(in_model))
		return HERO_SLIPPERY_ACCELERATION;

	return HERO_NORMAL_ACCELERATION;
}

static void __friction(wmdl_model_t& out_model)
{
	if (__on_slippery(out_model))
	{
		out_model.hero.sx += out_model.hero.sx * HERO_SLIPPERY_FRICTION * WMDL_SECONDS_PER_TICK;
	}
	else
	{
		if (out_model.hero.sx > 0)
		{
			out_model.hero.sx -= HERO_NORMAL_FRICTION * WMDL_SECONDS_PER_TICK;
			if (out_model.hero.sx < 0)
				out_model.hero.sx = 0;
		}
		else if (out_model.hero.sx < 0)
		{
			out_model.hero.sx += HERO_NORMAL_FRICTION * WMDL_SECONDS_PER_TICK;
			if (out_model.hero.sx > 0)
				out_model.hero.sx = 0;
		}
	}
}

static bool __jumping(const wmdl_hero_t& in_hero)
{
	return WMDL_HERO_FLAGS_JUMP & in_hero.input;
}

static float __get_test_above_x(const wmdl_hero_t& in_hero)
{
	return in_hero.x;
}

static float __get_test_above_y(const wmdl_hero_t& in_hero)
{
	return in_hero.y - HERO_HALF_HEIGHT;
}

static bool __is_solid_above(const wmdl_model_t& in_model)
{
	return HP_SOLID == __get_tile_info_for_position(in_model, __get_test_above_x(in_model.hero), __get_test_above_y(in_model.hero) - 1).hero_pass;
}

static bool __in_box(const float in_x, const float in_y, const float in_box_center_x, const float in_box_center_y, const float in_half_aspect)
{
	return
		in_x > (in_box_center_x - in_half_aspect) && in_x < (in_box_center_x + in_half_aspect) &&
		in_y >(in_box_center_y - in_half_aspect) && in_y < (in_box_center_y + in_half_aspect);
}

static float __get_test_left_x(const wmdl_hero_t& in_hero)
{
	return in_hero.x - HERO_HALF_WIDTH;
}

static float __get_test_right_x(const wmdl_hero_t& in_hero)
{
	return in_hero.x + HERO_HALF_WIDTH;
}

static bool __self_in_box(const float in_box_center_x, const float in_box_center_y, const float in_half_aspect, const wmdl_hero_t& in_hero)
{
	return
		__in_box(__get_test_left_x(in_hero), in_hero.y, in_box_center_x, in_box_center_y, in_half_aspect) ||
		__in_box(in_hero.x, in_hero.y, in_box_center_x, in_box_center_y, in_half_aspect) ||
		__in_box(__get_test_right_x(in_hero), in_hero.y, in_box_center_x, in_box_center_y, in_half_aspect);
}

static bool __whip_collide(const float in_x, const float in_y, const float in_half_aspect, const wmdl_hero_t& in_hero)
{
	const float MIN_X = wmdl_hero_whip_min_x(in_hero);
	const float MAX_X = MIN_X + HERO_WHIP_LENGTH;
	const float Y = wmdl_hero_whip_y(in_hero);

	if (Y > (in_y - in_half_aspect) && Y < (in_y + in_half_aspect))
	{
		if (MIN_X < (in_x + in_half_aspect) && MAX_X >(in_x - in_half_aspect))
		{
			return true;
		}
	}

	return false;
}

static wmdl_events_t __test_roller(
	const wmdl_events_t& in_input, const float in_now, const float in_old_y,
	wmdl_model_t& out_model, wmdl_enemy_t* out_roller)
{
	wmdl_events_t result = in_input;

	//crush test
	if (__self_in_box(out_roller->x, out_roller->y, ROLLER_HALF_ASPECT, out_model.hero))
	{
		if (out_model.hero.y > out_roller->y)
		{
			result.bits |= WMDL_EVENT_BIT_HERO_DIE;
			return result;
		}
		else
		{
			switch (out_roller->type)
			{
			case WMDL_LOGIC_INDEX_BROWNBLOB:
				//bounce
				out_model.hero.y = in_old_y;
				out_model.hero.sx *= .5f;
				out_model.hero.sy *= -1.f;
				break;

			case WMDL_LOGIC_INDEX_BLUEBLOB:
				//bounce
				out_model.hero.y = in_old_y;
				out_model.hero.sx *= .5f;
				out_model.hero.sy *= -.5f;

				//kill roller
				result.bits = WMDL_EVENT_BIT_ROLLER_DIE;
				result.roller_type = out_roller->type;
				result.roller_x = out_roller->x;
				result.roller_y = out_roller->y;

				out_roller->x = wmdl_offset_to_world_x(out_roller->spawn_offset);
				out_roller->y = wmdl_offset_to_world_y(out_roller->spawn_offset);
				out_roller->spawn_time = in_now + ENEMY_RESPAWN_TIME;

				break;

			default:
				//kill player
				result.bits |= WMDL_EVENT_BIT_HERO_DIE;
				return result;
			}
		}
	}
	//whip test
	else if (out_roller->type != WMDL_LOGIC_INDEX_BLUEBLOB && wmdl_hero_whip_extended(out_model.hero, in_now) && __whip_collide(out_roller->x, out_roller->y, ROLLER_HALF_ASPECT, out_model.hero))
	{
		//kill roller
		result.bits |= WMDL_EVENT_BIT_ROLLER_DIE;
		result.roller_type = out_roller->type;
		result.roller_x = out_roller->x;
		result.roller_y = out_roller->y;

		out_roller->x = wmdl_offset_to_world_x(out_roller->spawn_offset);
		out_roller->y = wmdl_offset_to_world_y(out_roller->spawn_offset);
		out_roller->spawn_time = in_now + ENEMY_RESPAWN_TIME;
	}

	return result;
}

static bool __can_bite(const wmdl_enemy_t& in_plant, const wmdl_hero_t& in_hero)
{
	return wmdl_plant_hero_distance(in_plant, in_hero) < WMDL_PLANT_BITE_DISTANCE;
}

static wmdl_events_t __test_plant(
	const wmdl_events_t& in_input, const float in_now,
	wmdl_model_t& out_model, wmdl_enemy_t* out_plant)
{
	wmdl_events_t result = in_input;

	//bite player?
	if (__can_bite(*out_plant, out_model.hero))
	{
		//player death
		result.bits |= WMDL_EVENT_BIT_HERO_DIE;
		return result;
	}
	else if (wmdl_hero_whip_extended(out_model.hero, in_now) && __whip_collide(out_plant->x, out_plant->y, PLANT_HALF_ASPECT, out_model.hero))
	{
		//plant death / respawn
		result.bits |= WMDL_EVENT_BIT_PLANT_DIE;
		result.plant_type = out_plant->type;
		result.plant_x = out_plant->x;
		result.plant_y = out_plant->y;

		out_plant->x = wmdl_offset_to_world_x(out_plant->spawn_offset);
		out_plant->y = wmdl_offset_to_world_y(out_plant->spawn_offset);
		out_plant->spawn_time = in_now + ENEMY_RESPAWN_TIME;
	}

	return result;
}

static uint32_t __test_bat(
	const float in_now,
	wmdl_model_t& out_model, wmdl_enemy_t* out_bat)
{
	uint32_t result = 0;

	if (!out_bat->scared)
	{
		//collide
		if (__self_in_box(out_bat->x, out_bat->y, BAT_RADIUS, out_model.hero))
		{
			//push hero around
			out_model.hero.sx += out_bat->vector_x * BAT_SPEED;
			out_model.hero.sy += out_bat->vector_y * BAT_SPEED;
		}
		//whip test
		else if (wmdl_hero_whip_extended(out_model.hero, in_now) && __whip_collide(out_bat->x, out_bat->y, BAT_RADIUS, out_model.hero))
		{
			out_bat->scared = true;
			out_bat->state = WMDL_BAT_S_FLEEING;
			out_bat->change_time = in_now + BAT_SCARED_TIME;

			result |= WMDL_EVENT_BIT_BAT_FLEE;
		}
	}

	return result;
}

static wmdl_events_t __test_slider(
	const wmdl_events_t& in_input, const float in_now, const float in_old_y,
	wmdl_model_t& out_model, wmdl_enemy_t* out_slider)
{
	wmdl_events_t result = in_input;

	if (__self_in_box(out_slider->x, out_slider->y, SLIDER_RADIUS, out_model.hero))
	{
		//if player below slider, he dies
		if (out_model.hero.y > out_slider->y)
		{
			result.bits |= WMDL_EVENT_BIT_HERO_DIE;
			return result;
		}
		else
		{
			//bounce
			out_model.hero.y = in_old_y;
			out_model.hero.sx *= .5f;
			out_model.hero.sy *= -1.f;

			//kill slider
			result.bits |= WMDL_EVENT_BIT_SLIDER_DIE;
			result.slider_x = out_slider->x;
			result.slider_y = out_slider->y;
			result.slider_speed = out_slider->speed;

			out_slider->x = wmdl_offset_to_world_x(out_slider->spawn_offset);
			out_slider->y = wmdl_offset_to_world_y(out_slider->spawn_offset);
			out_slider->spawn_time = in_now + ENEMY_RESPAWN_TIME;
		}
	}

	return result;
}

static wmdl_events_t __enemy_collision_tests(
	const wmdl_events_t& in_input, const float in_old_y,
	wmdl_model_t& out_model)
{
	const float NOW = wmdl_model_now(out_model);

	wmdl_events_t result = in_input;

	for (
		wmdl_enemy_t* enemy = out_model.enemy;
		enemy < out_model.enemy + out_model.num_enemy;
		++enemy
		)
	{
		switch (enemy->type)
		{
		default:
			assert(0);
			break;

		case WMDL_LOGIC_INDEX_SPIKYGREEN:
		case WMDL_LOGIC_INDEX_BLUEBLOB:
		case WMDL_LOGIC_INDEX_BROWNBLOB:
			if (NOW > enemy->spawn_time)
			{
				result = __test_roller(result, NOW, in_old_y, out_model, enemy);
				if (WMDL_EVENT_BIT_HERO_DIE & result.bits)
					return result;
			}
			break;

		case WMDL_LOGIC_INDEX_PLANT:
		case WMDL_LOGIC_INDEX_SCORPION:
			if (NOW > enemy->spawn_time)
			{
				result = __test_plant(result, NOW, out_model, enemy);
				if (WMDL_EVENT_BIT_HERO_DIE & result.bits)
					return result;
			}
			break;

		case WMDL_LOGIC_INDEX_PENGUIN:
			if (NOW > enemy->spawn_time)
			{
				result = __test_slider(result, NOW, in_old_y, out_model, enemy);
				if (WMDL_EVENT_BIT_HERO_DIE & result.bits)
					return result;
			}
			break;

		case WMDL_LOGIC_INDEX_FIREDUDE:
			if (__self_in_box(enemy->x, enemy->y, FIREDUDE_RADIUS, out_model.hero))
			{
				result.bits |= WMDL_EVENT_BIT_HERO_DIE;
				return result;
			}
			break;

		case WMDL_LOGIC_INDEX_BAT:
			result.bits |= __test_bat(NOW, out_model, enemy);
			break;

		case WMDL_LOGIC_INDEX_ICICLE:
		case WMDL_LOGIC_INDEX_SANDBLOCK:
			if (NOW > enemy->spawn_time)
			{
				if (__self_in_box(enemy->x, enemy->y, ICICLE_HALF_ASPECT, out_model.hero))
				{
					result.bits |= WMDL_EVENT_BIT_HERO_DIE;
					return result;
				}
			}
			break;
		}
	}

	return result;
}

static float __get_test_below(const wmdl_hero_t& in_hero)
{
	return in_hero.y + HERO_HALF_HEIGHT;
}

static float __get_test_below_x(const bool in_second, const wmdl_hero_t& in_hero)
{
	if (in_second)
		return in_hero.x - HERO_HALF_WIDTH / 2 + HERO_HALF_WIDTH;

	return in_hero.x - HERO_HALF_WIDTH / 2;
}

static bool __is_solid_below(const wmdl_model_t& in_model, const float in_x, const float in_y)
{
	const wmdl_tile_info_t& INFO = __get_tile_info_for_position(in_model, in_x, in_y);

	if (in_model.hero.sy >= 0)
		return INFO.hero_pass == HP_SOLID || INFO.hero_pass == HP_PLATFORM;

	return INFO.hero_pass == HP_SOLID;
}

static bool __is_solid_below(const wmdl_model_t& in_model)
{
	const float Y = __get_test_below(in_model.hero);

	//bottom left probe
	float x = __get_test_below_x(false, in_model.hero);
	bool solid = __is_solid_below(in_model, x, Y);
	if (!solid)
	{
		//bottom right probe
		x = __get_test_below_x(true, in_model.hero);
		solid = __is_solid_below(in_model, x, Y);
	}

	return solid;
}

static float __get_test_left_right_y(const bool in_second, const wmdl_hero_t& in_hero)
{
	if (in_second)
		return in_hero.y + 4;

	return in_hero.y - 4;
}

static bool __is_solid_left(const wmdl_model_t& in_model)
{
	const float X = __get_test_left_x(in_model.hero);
	float y = __get_test_left_right_y(false, in_model.hero);
	bool solid = HP_SOLID == __get_tile_info_for_position(in_model, X, y).hero_pass;
	if (!solid)
	{
		y = __get_test_left_right_y(true, in_model.hero);
		solid = HP_SOLID == __get_tile_info_for_position(in_model, X, y).hero_pass;
	}

	return solid;
}

static bool __is_solid_right(const wmdl_model_t& in_model)
{
	//bottom right probe
	const float X = __get_test_right_x(in_model.hero);
	float y = __get_test_left_right_y(false, in_model.hero);
	bool solid = HP_SOLID == __get_tile_info_for_position(in_model, X, y).hero_pass;
	if (!solid)
	{
		//top right probe
		y = __get_test_left_right_y(true, in_model.hero);
		solid = HP_SOLID == __get_tile_info_for_position(in_model, X, y).hero_pass;
	}

	return solid;
}


static uint32_t __world_collision_tests(
	const float in_old_x, const float in_old_y,
	wmdl_model_t& out_model)
{
	uint32_t result = 0;

	//below test
	if (__is_solid_below(out_model))
	{
		if (out_model.hero.air_bit)
		{
			result |= WMDL_EVENT_BIT_HERO_LANDED;

			out_model.hero.air_bit = false;
		}

		out_model.hero.sy = 0;

		//this snapping seems to work
		out_model.hero.y /= WMDL_TILE_ASPECT;
		out_model.hero.y = ::floorf(out_model.hero.y);
		out_model.hero.y *= WMDL_TILE_ASPECT;
		out_model.hero.y += WMDL_TILE_ASPECT - HERO_HALF_HEIGHT;
	}
	else
	{
		out_model.hero.air_bit = true;
	}

	//above test
	const float X = __get_test_above_x(out_model.hero);
	const float Y = __get_test_above_y(out_model.hero);
	const bool SOLIDABOVE = HP_SOLID == __get_tile_info_for_position(out_model, X, Y).hero_pass;
	if (SOLIDABOVE)
	{
		out_model.hero.sy = 0;

		out_model.hero.y /= WMDL_TILE_ASPECT;
		out_model.hero.y = ::floorf(out_model.hero.y);
		out_model.hero.y *= WMDL_TILE_ASPECT;
		out_model.hero.y += HERO_HALF_HEIGHT;
	}

	//left test
	if (__is_solid_left(out_model))
	{
		out_model.hero.sx = 0;

		out_model.hero.x /= WMDL_TILE_ASPECT;
		out_model.hero.x = ::floorf(out_model.hero.x);
		out_model.hero.x *= WMDL_TILE_ASPECT;
		out_model.hero.x += HERO_HALF_WIDTH;
	}

	//right test
	if (__is_solid_right(out_model))
	{
		out_model.hero.sx = 0;

		out_model.hero.x /= WMDL_TILE_ASPECT;
		out_model.hero.x = ::floorf(out_model.hero.x);
		out_model.hero.x *= WMDL_TILE_ASPECT;
		out_model.hero.x += WMDL_TILE_ASPECT - HERO_HALF_WIDTH;
	}

	//sanity check
	const wmdl_tile_info_t& INFO = __get_tile_info_for_position(out_model, out_model.hero.x, out_model.hero.y);
	if (INFO.hero_pass == HP_SOLID)
	{
		out_model.hero.x = in_old_x;
		out_model.hero.y = in_old_y;
	}

	return result;
}

static void __bounds_tests(wmdl_hero_t& out_hero)
{
	if (out_hero.x < 0)
	{
		out_hero.sx = -out_hero.sx;
		out_hero.x = 0;
	}
	else if (out_hero.x >= WMDL_WORLD_WIDTH * WMDL_TILE_ASPECT)
	{
		out_hero.sx = -out_hero.sx;
		out_hero.x = WMDL_WORLD_WIDTH * WMDL_TILE_ASPECT - 1;
	}

	if (out_hero.y < 0)
	{
		out_hero.sy = 0;
		out_hero.y = 0;
	}
	else if (out_hero.y >= WMDL_WORLD_HEIGHT * WMDL_TILE_ASPECT)
	{
		out_hero.sy = -out_hero.sy;
		out_hero.y = WMDL_WORLD_HEIGHT * WMDL_TILE_ASPECT - 1;
	}
}

static void __icicle_idle(
	const wmdl_model_t& in_model,
	wmdl_enemy_t* out_icicle)
{
	const int32_t x = __world_to_grid(out_icicle->x);
	if (in_model.hero.spawn_time < 0.f && __world_to_grid(in_model.hero.x) == x)
	{
		const int32_t HERO_Y = __world_to_grid(in_model.hero.y);
		int32_t y = __world_to_grid(out_icicle->y);

		while (1)
		{
			if (HP_CLEAR == wmdl_model_get_tile_info(in_model, wmdl_model_get_tile(in_model, x, y), false).hero_pass)
			{
				if (y == HERO_Y)
				{
					out_icicle->change_time = wmdl_model_now(in_model) + ICICLE_PENDING_TIME;
					out_icicle->state = WMDL_ICICLE_S_PENDING;
					return;
				}
			}
			else
			{
				return;
			}
			++y;
		}
	}
}

static void __roller_update(
	const float in_now, const wmdl_model_t& in_model,
	wmdl_enemy_t* out_roller)
{
	if (in_now > out_roller->spawn_time)
	{
		const float OLD_X = out_roller->x;

		out_roller->x += out_roller->speed * WMDL_SECONDS_PER_TICK;

		//must walk on HP_SOLID or HP_PLATFORM
		if (HP_CLEAR == __get_tile_info_for_position(in_model, out_roller->x, out_roller->y + WMDL_TILE_ASPECT).hero_pass)
		{
			out_roller->x = OLD_X;
			out_roller->speed = -out_roller->speed;
		}

		//side checks
		if (out_roller->speed < 0)
		{
			if ((out_roller->x - 8) < 0 || HP_CLEAR != __get_tile_info_for_position(in_model, out_roller->x - 8, out_roller->y).hero_pass)
			{
				out_roller->x = OLD_X;
				out_roller->speed = -out_roller->speed;
			}
		}
		else
		{
			if ((out_roller->x + 8) > (WMDL_WORLD_WIDTH * WMDL_TILE_ASPECT) || HP_CLEAR != __get_tile_info_for_position(in_model, out_roller->x + 8, out_roller->y).hero_pass)
			{
				out_roller->x = OLD_X;
				out_roller->speed = -out_roller->speed;
			}
		}
	}
}

static void __firedude_update(
	const wmdl_model_t& in_model,
	wmdl_enemy_t* out_firedude)
{
	const float OLD_X = out_firedude->x;
	const float OLD_Y = out_firedude->y;

	out_firedude->x += out_firedude->vector_x * FIREDUDE_SPEED * WMDL_SECONDS_PER_TICK;
	out_firedude->y += out_firedude->vector_y * FIREDUDE_SPEED * WMDL_SECONDS_PER_TICK;

	if (HP_SOLID == __get_tile_info_for_position(in_model, out_firedude->x + FIREDUDE_RADIUS, out_firedude->y).hero_pass ||
		HP_SOLID == __get_tile_info_for_position(in_model, out_firedude->x - FIREDUDE_RADIUS, out_firedude->y).hero_pass ||
		HP_SOLID == __get_tile_info_for_position(in_model, out_firedude->x, out_firedude->y + FIREDUDE_RADIUS).hero_pass ||
		HP_SOLID == __get_tile_info_for_position(in_model, out_firedude->x, out_firedude->y - FIREDUDE_RADIUS).hero_pass ||
		out_firedude->x < 0 ||
		out_firedude->y < 0 ||
		out_firedude->x > WMDL_WORLD_WIDTH * WMDL_TILE_ASPECT ||
		out_firedude->y > WMDL_WORLD_HEIGHT * WMDL_TILE_ASPECT)
	{
		out_firedude->x = OLD_X;
		out_firedude->y = OLD_Y;
		__random_vector(out_firedude->vector_x, out_firedude->vector_y);
	}
}

static void __bat_update(
	const float in_now, const wmdl_model_t& in_model,
	wmdl_enemy_t* out_bat)
{
	switch (out_bat->state)
	{
	case WMDL_BAT_S_PASSIVE:
		//if not scared anymore, look for hero
		if (out_bat->change_time < in_now)
		{
			//clear damaged flag
			out_bat->scared = false;

			//check if hero in range
			if (__vector_length(out_bat->x - in_model.hero.x, out_bat->y - in_model.hero.y) < BAT_SEEK_RADIUS)
			{
				//go active
				out_bat->state = WMDL_BAT_S_ATTACKING;
				out_bat->change_time = in_now;
			}
		}
		break;

	case WMDL_BAT_S_ATTACKING:
	{
		const float OLD_X = out_bat->x;
		const float OLD_Y = out_bat->y;

		//if not avoiding, seek target
		if (out_bat->change_time < in_now)
		{
			out_bat->vector_x = in_model.hero.x - out_bat->x;
			out_bat->vector_y = in_model.hero.y - out_bat->y;
			__normalize_vector(out_bat->vector_x, out_bat->vector_y);
		}

		//move
		out_bat->x += out_bat->vector_x * BAT_SPEED * WMDL_SECONDS_PER_TICK;
		out_bat->y += out_bat->vector_y * BAT_SPEED * WMDL_SECONDS_PER_TICK;

		//tile collisions
		if (HP_SOLID == __get_tile_info_for_position(in_model, out_bat->x + BAT_RADIUS, out_bat->y).hero_pass ||
			HP_SOLID == __get_tile_info_for_position(in_model, out_bat->x - BAT_RADIUS, out_bat->y).hero_pass ||
			HP_SOLID == __get_tile_info_for_position(in_model, out_bat->x, out_bat->y + BAT_RADIUS).hero_pass ||
			HP_SOLID == __get_tile_info_for_position(in_model, out_bat->x, out_bat->y - BAT_RADIUS).hero_pass ||

			//world bounds
			out_bat->x < 0 ||
			out_bat->y < 0 ||
			out_bat->x > WMDL_WORLD_WIDTH * WMDL_TILE_ASPECT ||
			out_bat->y > WMDL_WORLD_HEIGHT * WMDL_TILE_ASPECT)
		{
			out_bat->x = OLD_X;
			out_bat->y = OLD_Y;

			//avoid for a time
			__random_vector(out_bat->vector_x, out_bat->vector_y);
			out_bat->change_time = in_now + BAT_AVOID_TIME;
		}

		//if off spawn screen, go home
		if (wmdl_screen_x(wmdl_offset_to_world_x(out_bat->spawn_offset)) != wmdl_screen_x(out_bat->x) || wmdl_screen_y(wmdl_offset_to_world_y(out_bat->spawn_offset)) != wmdl_screen_y(out_bat->y))
		{
			out_bat->scared = true;
			out_bat->state = WMDL_BAT_S_FLEEING;
			out_bat->change_time = in_now + BAT_SCARED_TIME;
		}
	}
	break;

	case WMDL_BAT_S_FLEEING:
	{
		//seek spawnpoint
		{
			out_bat->vector_x = wmdl_offset_to_world_x(out_bat->spawn_offset) - out_bat->x;
			out_bat->vector_y = wmdl_offset_to_world_y(out_bat->spawn_offset) - out_bat->y;
			__normalize_vector(out_bat->vector_x, out_bat->vector_y);
			out_bat->x += out_bat->vector_x * BAT_SPEED * WMDL_SECONDS_PER_TICK;
			out_bat->y += out_bat->vector_y * BAT_SPEED * WMDL_SECONDS_PER_TICK;
		}

		//re-attach to spawn point?
		if (__vector_length(wmdl_offset_to_world_x(out_bat->spawn_offset) - out_bat->x, wmdl_offset_to_world_y(out_bat->spawn_offset) - out_bat->y) < BAT_RADIUS)
		{
			out_bat->x = wmdl_offset_to_world_x(out_bat->spawn_offset);
			out_bat->y = wmdl_offset_to_world_y(out_bat->spawn_offset);
			out_bat->state = WMDL_BAT_S_PASSIVE;
		}

		//not scared anymore?
		if (out_bat->change_time < in_now)
		{
			out_bat->scared = false;
			out_bat->state = WMDL_BAT_S_ATTACKING;
		}
	}
	break;
	}
}

static wmdl_events_t __slider_update(
	const wmdl_events_t& in_input, const float in_now, const wmdl_model_t& in_model,
	wmdl_enemy_t* out_slider)
{
	wmdl_events_t result = in_input;

	if (in_now > out_slider->spawn_time)
	{
		const float OLD_X = out_slider->x;

		out_slider->speed += out_slider->speed * SLIDER_FRICTION * WMDL_SECONDS_PER_TICK;

		if (::fabsf(out_slider->speed) < SLIDER_MIN_SPEED)
		{
			if (out_slider->speed < 0)
			{
				out_slider->speed -= SLIDER_IMPULSE;
			}
			else
			{
				out_slider->speed += SLIDER_IMPULSE;
			}
			if (wmdl_screen_x(out_slider->x) == wmdl_screen_x(in_model.hero.x) && wmdl_screen_y(out_slider->y) == wmdl_screen_y(in_model.hero.y))
				result.bits |= WMDL_EVENT_BIT_SLIDER_IMPULSE;
		}

		out_slider->x += out_slider->speed * WMDL_SECONDS_PER_TICK;

		//must walk on HP_SOLID or HP_PLATFORM
		if (HP_CLEAR == __get_tile_info_for_position(in_model, out_slider->x, out_slider->y + WMDL_TILE_ASPECT).hero_pass)
		{
			out_slider->x = OLD_X;
			out_slider->speed = -out_slider->speed;
		}

		//side checks
		if (out_slider->speed < 0)
		{
			if ((out_slider->x - 8) < 0 || HP_CLEAR != __get_tile_info_for_position(in_model, out_slider->x - 8, out_slider->y).hero_pass)
			{
				out_slider->x = OLD_X;
				out_slider->speed = -out_slider->speed;
			}
		}
		else
		{
			if ((out_slider->x + 8) > (WMDL_WORLD_WIDTH * WMDL_TILE_ASPECT) || HP_CLEAR != __get_tile_info_for_position(in_model, out_slider->x + 8, out_slider->y).hero_pass)
			{
				out_slider->x = OLD_X;
				out_slider->speed = -out_slider->speed;
			}
		}
	}

	return result;
}

static void __icicle_update(
	const float in_now, const wmdl_model_t& in_model,
	wmdl_enemy_t* out_icicle)
{
	if (in_now > out_icicle->spawn_time)
	{
		switch (out_icicle->state)
		{
		case WMDL_ICICLE_S_IDLE:
			__icicle_idle(in_model, out_icicle);
			break;

		case WMDL_ICICLE_S_PENDING:
			if (out_icicle->change_time < in_now)
			{
				out_icicle->speed = 0;
				out_icicle->state = WMDL_ICICLE_S_FALLING;
			}
			break;

		case WMDL_ICICLE_S_FALLING:
			out_icicle->speed += WMDL_GRAVITY * WMDL_SECONDS_PER_TICK;
			out_icicle->y += out_icicle->speed * WMDL_SECONDS_PER_TICK;

			if (HP_CLEAR != __get_tile_info_for_position(in_model, out_icicle->x, out_icicle->y).hero_pass)
			{
				out_icicle->state = WMDL_ICICLE_S_IDLE;

				out_icicle->x = wmdl_offset_to_world_x(out_icicle->spawn_offset);
				out_icicle->y = wmdl_offset_to_world_y(out_icicle->spawn_offset);
				out_icicle->spawn_time = in_now + ENEMY_RESPAWN_TIME;
			}
			break;
		}
	}
}

static const wmdl_portal_t* __portal_for_target_world(const wmdl_model_t& in_model, const char* in_target_world)
{
	assert(in_target_world);
	for (
		const wmdl_portal_t* PORTAL = in_model.level.portals;
		PORTAL < in_model.level.portals + in_model.level.num_portals;
		++PORTAL
		)
	{
		if (0 == ::strcmp(PORTAL->target_world, in_target_world))
			return PORTAL;
	}
	return nullptr;
}

static const wmdl_portal_t* __portal_for_offset(const wmdl_model_t& in_model, const uint32_t in_offset)
{
	for (
		const wmdl_portal_t* PORTAL = in_model.level.portals;
		PORTAL < in_model.level.portals + in_model.level.num_portals;
		++PORTAL)
	{
		if (PORTAL->offset == in_offset)
			return PORTAL;
	}
	return nullptr;
}

//public
//public
//public
//public

bool wmdl_model_load_world(
	const char* in_file, const bool in_new_game,
	wmdl_model_t& out_model)
{
	::strcpy_s(out_model.last_world, out_model.world);
	::strcpy_s(out_model.world, in_file);

	fs_blob_t contents = fs_file_contents(in_file);
	assert(contents.data);
	iterator_t it;
	it.it = (uint8_t*)contents.data;

	//version check
	const uint32_t VERSION = __uint(it);

	if (FILE_VERSION == VERSION)
	{
		out_model.level = {};
		out_model.level.magic = 'n' << 24 | 'm' << 16 | 'p' << 8 | 't';

		while (it.it < (uint8_t*)contents.data + contents.size - sizeof(uint32_t))
		{
			const uint32_t chunk_type = __uint(it);

			switch (chunk_type)
			{
			default:
				assert(0);
				break;

			case CHUNK_TILE:
			{
				const uint32_t tile_offset = __uint(it);
				const uint32_t tile_index = __uint(it);

				if (tile_offset < WMDL_WORLD_SIZE)	//bad level size fix!
					out_model.level.tiles[tile_offset].index = tile_index;
			}
			break;

			case CHUNK_START:
			{
				const uint32_t start = __uint(it);

				out_model.level.start = start;
			}
			break;

			case CHUNK_BACKGROUND:
			{
				const uint32_t screen_offset = __uint(it);
				const uint32_t screen_index = __uint(it);

				out_model.level.screens[screen_offset] = screen_index;
			}
			break;

			//legacy
			case CHUNK_PORTAL:
			{
				const uint32_t portal_offset = __uint(it);
				const char* portal_target_world = __utf(it);

				assert(out_model.level.num_portals < _countof(out_model.level.portals));
				wmdl_portal_t* p = out_model.level.portals + out_model.level.num_portals++;
				p->offset = portal_offset;
				p->server_count = 0;
				::strcpy_s(p->target_world, portal_target_world);
			}
			break;

			case CHUNK_PORTAL_2:
			{
				const uint32_t portal_offset = __uint(it);
				const uint32_t portal_server_count = __uint(it);
				const char* portal_target_world = __utf(it);

				assert(out_model.level.num_portals < _countof(out_model.level.portals));
				wmdl_portal_t* p = out_model.level.portals + out_model.level.num_portals++;
				p->offset = portal_offset;
				p->server_count = portal_server_count;
				::strcpy_s(p->target_world, portal_target_world);
			}
			break;

			case CHUNK_TRACK:
			{
				const uint32_t music_track = __uint(it);

				out_model.level.music_track = music_track;
			}
			break;

			case CHUNK_INFO:
			{
				const uint32_t info_offset = __uint(it);
				const uint32_t info_id = __uint(it);

				assert(out_model.level.num_infos < _countof(out_model.level.infos));
				wmdl_info_t* i = out_model.level.infos + out_model.level.num_infos++;
				i->offset = info_offset;
				i->id = info_id;
			}
			break;
			}
		}

		out_model.play_bit = true;

		out_model.num_enemy = 0;
		for (uint32_t i = 0; i < WMDL_WORLD_SIZE; ++i)
			__logic_spawn(i, out_model.level.tiles[i].index, out_model);

		{
			const wmdl_portal_t* p = nullptr;
			if (out_model.last_world[0])
				p = __portal_for_target_world(out_model, out_model.last_world);
			if (p)
			{
				out_model.hero.x = wmdl_offset_to_world_x(p->offset);
				out_model.hero.y = wmdl_offset_to_world_y(p->offset);
				out_model.hero.checkpoint = p->offset;
			}
			else
			{
				out_model.hero.x = wmdl_offset_to_world_x(out_model.level.start);
				out_model.hero.y = wmdl_offset_to_world_y(out_model.level.start);
				out_model.hero.checkpoint = out_model.level.start;
			}
		}
		out_model.hero.sx = 0.f;
		out_model.hero.sy = 0.f;
		out_model.hero.input = 0;
		out_model.hero.spawn_time = -1.;
		out_model.hero.keys = 0;
		out_model.hero.checkpoint_keys = 0;
		out_model.hero.right_bit = false;
		out_model.hero.air_bit = false;
		if (in_new_game)
		{
			out_model.hero.fixed_servers = {};
			out_model.hero.checkpoint_fixed_servers = {};
		}
		out_model.hero.whip_time = -100.;
	}

	delete[] contents.data;

	return FILE_VERSION == VERSION;
}

bool wmdl_model_init(wmdl_model_t& out_model)
{
	const fs_blob_t CONTENTS = fs_file_contents(ASSET_COMMON_INFO);
	const bool RESULT = CONTENTS.data && sizeof(out_model.tile_info) == CONTENTS.size;
	if (RESULT)
	{
		::memcpy(&out_model.tile_info, CONTENTS.data, CONTENTS.size);
		for (
			wmdl_tile_info_t* i = out_model.tile_info; 
			i < out_model.tile_info + WMDL_MAX_TILE; 
			++i
			)
		{
			i->hero_pass = wmdl_change_endianness(i->hero_pass);
			i->special = wmdl_change_endianness(i->special);
		}
	}
	delete[] CONTENTS.data;
	return RESULT;
}

wmdl_events_t wmdl_model_update(wmdl_model_t& out_model)
{
	++out_model.tick;
	if (!out_model.play_bit)
		return {};	//do not load a new world

	const float NOW = wmdl_model_now(out_model);

	wmdl_events_t result{};

	for (
		wmdl_enemy_t* enemy = out_model.enemy;
		enemy < out_model.enemy + out_model.num_enemy;
		++enemy
		)
	{
		switch (enemy->type)
		{
		default:
			assert(0);
			break;

		case WMDL_LOGIC_INDEX_SPIKYGREEN:
		case WMDL_LOGIC_INDEX_BLUEBLOB:
		case WMDL_LOGIC_INDEX_BROWNBLOB:
			__roller_update(NOW, out_model, enemy);
			break;

		case WMDL_LOGIC_INDEX_PLANT:
		case WMDL_LOGIC_INDEX_SCORPION:
			break;

		case WMDL_LOGIC_INDEX_PENGUIN:
			result = __slider_update(result, NOW, out_model, enemy);
			break;

		case WMDL_LOGIC_INDEX_FIREDUDE:
			__firedude_update(out_model, enemy);
			break;

		case WMDL_LOGIC_INDEX_BAT:
			__bat_update(NOW, out_model, enemy);
			break;

		case WMDL_LOGIC_INDEX_ICICLE:
		case WMDL_LOGIC_INDEX_SANDBLOCK:
			__icicle_update(NOW, out_model, enemy);
			break;
		}
	}

	if (out_model.hero.spawn_time < 0.f)
	{
		float acc = 0.f;

		//facing
		if (wmdl_hero_left_input(out_model.hero))
			out_model.hero.right_bit = 0;
		else if (wmdl_hero_right_input(out_model.hero))
			out_model.hero.right_bit = 1;

		//switches
		if (WMDL_HERO_FLAGS_DOWN & out_model.hero.input)
		{
			const wmdl_portal_t* PORTAL = __portal_for_offset(out_model, wmdl_world_to_offset(out_model.hero.x, out_model.hero.y));
#if 0
			if (PORTAL && out_model.hero.fixed_servers.count >= PORTAL->server_count)//this is the intended code
#else
			if (PORTAL)//this allows passage thru portals regardless of server count
#endif
				return { PORTAL->target_world };
		}

		//whip
		if (WMDL_HERO_FLAGS_WHIP & out_model.hero.input)
		{
			out_model.hero.whip_time = NOW;
			result.bits |= WMDL_EVENT_BIT_HERO_WHIP;
		}

		//x acceleration
		if (wmdl_hero_left_input(out_model.hero) && !wmdl_hero_right_input(out_model.hero))
			acc = -1;
		else if (wmdl_hero_right_input(out_model.hero) && !wmdl_hero_left_input(out_model.hero))
			acc = 1;
		if (acc != 0)
			out_model.hero.sx += acc * __acceleration(out_model) * WMDL_SECONDS_PER_TICK;

		//ground friction
		if (!out_model.hero.air_bit)
			__friction(out_model);

		//limits
		if (out_model.hero.sx > HERO_MAX_SPEEDX)
			out_model.hero.sx = HERO_MAX_SPEEDX;
		else if (out_model.hero.sx < -HERO_MAX_SPEEDX)
			out_model.hero.sx = -HERO_MAX_SPEEDX;

		//jumping
		if (__jumping(out_model.hero))
		{
			if (!out_model.hero.air_bit && !__is_solid_above(out_model))
			{
				out_model.hero.air_bit = true;
				out_model.hero.sy = HERO_JUMP;
				result.bits |= WMDL_EVENT_BIT_HERO_JUMP;
			}
		}

		//gravity
		out_model.hero.sy += WMDL_GRAVITY * WMDL_SECONDS_PER_TICK;
		if (out_model.hero.sy > HERO_MAX_SPEEDY)
			out_model.hero.sy = HERO_MAX_SPEEDY;
		else if (out_model.hero.sy < -HERO_MAX_SPEEDY)
			out_model.hero.sy = -HERO_MAX_SPEEDY;

		//movement
		const float OLD_X = out_model.hero.x;
		const float OLD_Y = out_model.hero.y;
		out_model.hero.x += out_model.hero.sx * WMDL_SECONDS_PER_TICK;
		out_model.hero.y += out_model.hero.sy * WMDL_SECONDS_PER_TICK;

		//a bunch of tests...
		if (S_LETHAL == __get_tile_info_for_position(out_model, out_model.hero.x, out_model.hero.y).special)
		{
			result.bits |= WMDL_EVENT_BIT_HERO_DIE;
		}
		else
		{
			result = __enemy_collision_tests(result, OLD_Y, out_model);
			result.bits |= __world_collision_tests(OLD_X, OLD_Y, out_model);
			__bounds_tests(out_model.hero);
		}
		if (WMDL_EVENT_BIT_HERO_DIE & result.bits)
		{
			out_model.hero.spawn_time = NOW + HERO_DEAD_TIME;
		}
	}//hero alive
	else
	{
		if (NOW > out_model.hero.spawn_time)
		{
			//restore state
			out_model.hero.x = wmdl_offset_to_world_x(out_model.hero.checkpoint);
			out_model.hero.y = wmdl_offset_to_world_y(out_model.hero.checkpoint);
			out_model.hero.sx = 0.f;
			out_model.hero.sy = 0;
			out_model.hero.input = 0;
			out_model.hero.air_bit = false;
			out_model.hero.spawn_time = -1.;
			out_model.hero.keys = out_model.hero.checkpoint_keys;
			out_model.hero.fixed_servers = out_model.hero.checkpoint_fixed_servers;
			out_model.hero.whip_time = -100.;

			result.bits |= WMDL_EVENT_BIT_BACK_TO_CHECKPOINT;
		}
	}

	//tile events
	if (out_model.hero.spawn_time < 0.f)
	{
		switch (__get_tile_for_position(out_model, out_model.hero.x, out_model.hero.y).index)
		{
		case WMDL_LOGIC_INDEX_CHECKPOINT:
			//store state
			out_model.hero.checkpoint = wmdl_world_to_offset(out_model.hero.x, out_model.hero.y);
			out_model.hero.checkpoint_keys = out_model.hero.keys;
			out_model.hero.checkpoint_fixed_servers = out_model.hero.fixed_servers;

			result.bits |= WMDL_EVENT_BIT_CHECKPOINT;
			break;

		case WMDL_LOGIC_INDEX_SERVER:
			if (__add_fixed_server(out_model.world, wmdl_world_to_offset(out_model.hero.x, out_model.hero.y), out_model.hero))
				result.bits |= WMDL_EVENT_BIT_FIXED_SERVER;
			break;

		case WMDL_LOGIC_INDEX_KEY0:
			if (0 == (out_model.hero.keys & WMDL_HERO_KEYBITS_0))
			{
				out_model.hero.keys |= WMDL_HERO_KEYBITS_0;
				result.bits |= WMDL_EVENT_BIT_KEY;
			}
			break;

		case WMDL_LOGIC_INDEX_KEY1:
			if (0 == (out_model.hero.keys & WMDL_HERO_KEYBITS_1))
			{
				out_model.hero.keys |= WMDL_HERO_KEYBITS_1;
				result.bits |= WMDL_EVENT_BIT_KEY;
			}
			break;

		case WMDL_LOGIC_INDEX_KEY2:
			if (0 == (out_model.hero.keys & WMDL_HERO_KEYBITS_2))
			{
				out_model.hero.keys |= WMDL_HERO_KEYBITS_2;
				result.bits |= WMDL_EVENT_BIT_KEY;
			}
			break;
		}
	}//tile events when player is alive

	return result;
}//void model_update(wmdl_model_t& out_model)

const wmdl_tile_t& wmdl_model_get_tile(const wmdl_model_t& in_model, const int32_t in_x, const int32_t in_y)
{
	return __get_tile(in_model, in_x, in_y);
}

float wmdl_model_now(const wmdl_model_t& in_model)
{
	return in_model.tick * WMDL_SECONDS_PER_TICK;
}

bool wmdl_model_is_server_fixed(const char* in_world, const uint32_t in_offset, const wmdl_hero_t& in_hero)
{
	for (
		const wmdl_server_t* FIXED_SERVER = in_hero.fixed_servers.server;
		FIXED_SERVER < in_hero.fixed_servers.server + in_hero.fixed_servers.count;
		++FIXED_SERVER
		)
	{
		if (0 == ::strcmp(FIXED_SERVER->world, in_world) && FIXED_SERVER->offset == in_offset)
			return true;
	}

	return false;
}

const wmdl_info_t* wmdl_model_info_for_offset(const wmdl_model_t& in_model, const uint32_t in_offset)
{
	for (
		const wmdl_info_t* INFO = in_model.level.infos;
		INFO < in_model.level.infos + in_model.level.num_infos;
		++INFO
		)
	{
		if (INFO->offset == in_offset)
			return INFO;
	}
	return nullptr;
}

uint32_t wmdl_model_screen(const wmdl_model_t& in_model, const int32_t in_x, const int32_t in_y)
{
	return in_model.level.screens[in_x + in_y * WMDL_SCREENS_X];
}

const wmdl_tile_info_t& wmdl_model_get_tile_info(const wmdl_model_t& in_model, const wmdl_tile_t& in_tile, const bool in_replace)
{
	if (in_replace)
	{
		switch (in_tile.index)
		{
		case WMDL_LOGIC_INDEX_KEY0BLOCK:
			if (in_model.hero.keys & WMDL_HERO_KEYBITS_0)
				return in_model.tile_info[WMDL_LOGIC_INDEX_AIR];
			break;

		case WMDL_LOGIC_INDEX_KEY1BLOCK:
			if (in_model.hero.keys & WMDL_HERO_KEYBITS_1)
				return in_model.tile_info[WMDL_LOGIC_INDEX_AIR];
			break;

		case WMDL_LOGIC_INDEX_KEY2BLOCK:
			if (in_model.hero.keys & WMDL_HERO_KEYBITS_2)
				return in_model.tile_info[WMDL_LOGIC_INDEX_AIR];
			break;
		}
	}

	return in_model.tile_info[in_tile.index];
}

bool wmdl_hero_is_checkpoint(const wmdl_hero_t& in_hero, const int32_t in_x, const int32_t in_y)
{
	return (uint32_t)(in_x + in_y * WMDL_WORLD_WIDTH) == in_hero.checkpoint;
}

bool wmdl_hero_left_input(const wmdl_hero_t& in_hero)
{
	return WMDL_HERO_FLAGS_LEFT & in_hero.input;
}

bool wmdl_hero_right_input(const wmdl_hero_t& in_hero)
{
	return WMDL_HERO_FLAGS_RIGHT & in_hero.input;
}

bool wmdl_hero_whipping(const wmdl_hero_t& in_hero, const float in_now)
{
	return in_hero.whip_time > (in_now - HERO_WHIP_TIME);
}

bool wmdl_hero_whip_extended(const wmdl_hero_t& in_hero, const float in_now)
{
	return wmdl_hero_whipping(in_hero, in_now) && in_now > (in_hero.whip_time + HERO_WHIP_TIME * .5f);
}

float wmdl_hero_whip_min_x(const wmdl_hero_t& in_hero)
{
	if (in_hero.right_bit)
		return in_hero.x + HERO_WHIP_X_OFFSET;

	return in_hero.x - HERO_WHIP_X_OFFSET - HERO_WHIP_LENGTH;
}

float wmdl_hero_whip_y(const wmdl_hero_t& in_hero)
{
	return in_hero.y + HERO_WHIP_Y_OFFSET;
}

uint32_t wmdl_world_to_offset(const float in_world_x, const float in_world_y)
{
	return wmdl_grid_to_offset(__world_to_grid(in_world_x), __world_to_grid(in_world_y));
}

int32_t wmdl_screen_x(const float in_x)
{
	return (int32_t)(in_x / (WMDL_TILE_ASPECT * WMDL_TILES_X));
}

int32_t wmdl_screen_y(const float in_y)
{
	return (int32_t)(in_y / (WMDL_TILE_ASPECT * WMDL_TILES_Y));
}

float wmdl_offset_to_world_x(const uint32_t in_offset)
{
	return (in_offset % WMDL_WORLD_WIDTH) * WMDL_TILE_ASPECT + WMDL_TILE_ASPECT * .5f;
}

float wmdl_offset_to_world_y(const uint32_t in_offset)
{
	return (in_offset / WMDL_WORLD_WIDTH) * WMDL_TILE_ASPECT + WMDL_TILE_ASPECT * .5f;
}

uint32_t wmdl_grid_to_offset(const int32_t in_grid_x, const int32_t in_grid_y)
{
	return in_grid_x + in_grid_y * WMDL_WORLD_WIDTH;
}

float wmdl_random_unit()
{
	return (float)::rand() / (float)RAND_MAX;
}

float wmdl_plant_hero_distance(const wmdl_enemy_t& in_plant, const wmdl_hero_t& in_hero)
{
	return __vector_length(in_hero.x - in_plant.x, in_hero.y - in_plant.y);
}

uint16_t wmdl_change_endianness(const uint16_t in)
{
	uint16_t out;
	uint8_t* src = (uint8_t*)&in;
	uint8_t* dst = (uint8_t*)&out;
	dst[0] = src[1];
	dst[1] = src[0];
	return out;
}

uint32_t wmdl_change_endianness(const uint32_t in)
{
	uint32_t out;
	uint8_t* src = (uint8_t*)&in;
	uint8_t* dst = (uint8_t*)&out;
	dst[0] = src[3];
	dst[1] = src[2];
	dst[2] = src[1];
	dst[3] = src[0];
	return out;
}

/*
float wmdl_gravity()
{
	if (wmdl_tune_new)
		return 850.f;
	return 1300.f;
}

float wmdl_hero_jump()
{
	if (wmdl_tune_new)
		return -300.f;
	return -400.f;
}

bool wmdl_tune_new = false;
*/