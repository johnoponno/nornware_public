#include "stdafx.h"
#include "tpmn_model.h"

#include "../softdraw/fs.h"

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
#define ICICLE_HALF_ASPECT 12.f//Model.TPMN_TILE_ASPECT / 2;

#define HERO_AIR_ACC 750.f
#define HERO_NORMAL_ACCELERATION 3750.f
#define HERO_SLIPPERY_ACCELERATION 200.f
#define HERO_NORMAL_FRICTION 3000.f
#define HERO_SLIPPERY_FRICTION -.5f
#define HERO_MAX_SPEEDX 200.f
#define HERO_MAX_SPEEDY 500.f
#define HERO_HALF_WIDTH 8
#define HERO_HALF_HEIGHT 12
#define HERO_JUMP 400.f
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

static uint32_t __uint(iterator_t& it)
{
	const uint32_t result = tpmn_change_endianness(*(uint32_t*)it.it);
	it.it += sizeof(uint32_t);
	return result;
}

static const char* __utf(iterator_t& it)
{
	const uint16_t length = tpmn_change_endianness(*(uint16_t*)it.it);
	it.it += sizeof(uint16_t);

	const char* result = (char*)it.it;
	it.it += length;

	return result;
}

static float __vector_length(const float x, const float y)
{
	return ::sqrtf(x * x + y * y);
}

static void __normalize_vector(float& x, float& y)
{
	const float l = __vector_length(x, y);
	x /= l;
	y /= l;
}

static void __random_vector(float& x, float& y)
{
	x = (tpmn_random_unit() * 2 - 1);
	y = (tpmn_random_unit() * 2 - 1);
	__normalize_vector(x, y);
}

static int32_t __world_to_grid(const float aWorld)
{
	return (int32_t)(aWorld / TPMN_TILE_ASPECT);
}

static void __logic_spawn(const uint32_t anOffset, const uint32_t anIndex, tpmn_model_t& model)
{
	switch (anIndex)
	{
	case TPMN_LOGIC_INDEX_SPIKYGREEN:
	case TPMN_LOGIC_INDEX_BLUEBLOB:
	case TPMN_LOGIC_INDEX_BROWNBLOB:
	{
		assert(model.num_enemy < _countof(model.enemy));
		tpmn_enemy_t* r = model.enemy + model.num_enemy++;
		r->type = anIndex;
		r->spawn_offset = anOffset;
		r->x = tpmn_offset_to_world_x(anOffset);
		r->y = tpmn_offset_to_world_y(anOffset);
		r->spawn_time = tpmn_model_now(model);
		r->speed = -ROLLER_SPEED;
	}
	break;

	case TPMN_LOGIC_INDEX_PLANT:
	case TPMN_LOGIC_INDEX_SCORPION:
	{
		assert(model.num_enemy < _countof(model.enemy));
		tpmn_enemy_t* p = model.enemy + model.num_enemy++;
		p->type = anIndex;
		p->spawn_offset = anOffset;
		p->x = tpmn_offset_to_world_x(anOffset);
		p->y = tpmn_offset_to_world_y(anOffset);
		p->spawn_time = tpmn_model_now(model);
	}
	break;

	case TPMN_LOGIC_INDEX_PENGUIN:
	{
		assert(model.num_enemy < _countof(model.enemy));
		tpmn_enemy_t* s = model.enemy + model.num_enemy++;
		s->type = anIndex;
		s->spawn_offset = anOffset;
		s->x = tpmn_offset_to_world_x(anOffset);
		s->y = tpmn_offset_to_world_y(anOffset);
		s->spawn_time = tpmn_model_now(model);
		s->speed = 0.f;
	}
	break;

	case TPMN_LOGIC_INDEX_FIREDUDE:
	{
		assert(model.num_enemy < _countof(model.enemy));
		tpmn_enemy_t* f = model.enemy + model.num_enemy++;
		f->type = anIndex;
		f->x = tpmn_offset_to_world_x(anOffset);
		f->y = tpmn_offset_to_world_y(anOffset);
		__random_vector(f->vector_x, f->vector_y);
	}
	break;

	case TPMN_LOGIC_INDEX_BAT:
	{
		assert(model.num_enemy < _countof(model.enemy));
		tpmn_enemy_t* b = model.enemy + model.num_enemy++;
		b->type = anIndex;
		b->spawn_offset = anOffset;
		b->x = tpmn_offset_to_world_x(anOffset);
		b->y = tpmn_offset_to_world_y(anOffset);
		b->state = TPMN_BAT_S_PASSIVE;
		b->change_time = 0.f;
		b->scared = false;
		b->vector_x = 0.f;
		b->vector_y = 0.f;
	}
	break;

	case TPMN_LOGIC_INDEX_ICICLE:
	case TPMN_LOGIC_INDEX_SANDBLOCK:
	{
		assert(model.num_enemy < _countof(model.enemy));
		tpmn_enemy_t* i = model.enemy + model.num_enemy++;
		i->type = anIndex;
		i->spawn_offset = anOffset;
		i->x = tpmn_offset_to_world_x(anOffset);
		i->y = tpmn_offset_to_world_y(anOffset);
		i->spawn_time = tpmn_model_now(model);
		i->state = TPMN_ICICLE_S_IDLE;
		i->change_time = 0.f;
		i->speed = 0.f;
	}
	break;
	}
}

static bool __add_fixed_server(const char* world, const uint32_t offset, tpmn_hero_t& hero)
{
	if (!model_is_server_fixed(world, offset, hero))
	{
		assert(hero.fixed_servers.count < _countof(hero.fixed_servers.server));
		::strcpy_s(hero.fixed_servers.server[hero.fixed_servers.count].world, world);
		hero.fixed_servers.server[hero.fixed_servers.count].offset = offset;
		++hero.fixed_servers.count;
		return true;
	}

	return false;
}

static bool __in_world(const int32_t x, const int32_t y)
{
	return x >= 0 && x < TPMN_WORLD_WIDTH&& y >= 0 && y < TPMN_WORLD_HEIGHT;
}

static const tpmn_tile_t& __get_tile(const tpmn_model_t& model, const int32_t x, const int32_t y)
{
	if (__in_world(x, y))
		return model.level.tiles[x + y * TPMN_WORLD_WIDTH];

	static tpmn_tile_t null_tile{};
	return null_tile;
}

const tpmn_tile_t& __get_tile_for_position(const tpmn_model_t& model, const float x, const float y)
{
	return __get_tile(model, __world_to_grid(x), __world_to_grid(y));
}

static const tile_info_t& __get_tile_info_for_position(const float x, const float y, const tpmn_model_t& model)
{
	return model_get_tile_info(__get_tile_for_position(model, x, y), true, model);
}

static bool __on_slippery(const tpmn_model_t& model)
{
	const tile_info_t& info = __get_tile_info_for_position(model.hero.x, model.hero.y + TPMN_TILE_ASPECT, model);
	return info.special == S_SLIPPERY || info.hero_pass == HP_CLEAR;
}

static float __acceleration(const tpmn_model_t& model)
{
	if (model.hero.air_bit)
		return HERO_AIR_ACC;

	if (__on_slippery(model))
		return HERO_SLIPPERY_ACCELERATION;

	return HERO_NORMAL_ACCELERATION;
}

static void __friction(tpmn_model_t& model)
{
	if (__on_slippery(model))
	{
		model.hero.sx += model.hero.sx * HERO_SLIPPERY_FRICTION * TPMN_SECONDS_PER_TICK;
	}
	else
	{
		if (model.hero.sx > 0)
		{
			model.hero.sx -= HERO_NORMAL_FRICTION * TPMN_SECONDS_PER_TICK;
			if (model.hero.sx < 0)
				model.hero.sx = 0;
		}
		else if (model.hero.sx < 0)
		{
			model.hero.sx += HERO_NORMAL_FRICTION * TPMN_SECONDS_PER_TICK;
			if (model.hero.sx > 0)
				model.hero.sx = 0;
		}
	}
}

static bool __jumping(const tpmn_hero_t& hero)
{
	return (hero.input & TPMN_HERO_FLAGS_JUMP) != 0;
}

static float __get_test_above_x(const tpmn_hero_t& hero)
{
	return hero.x;
}

static float __get_test_above_y(const tpmn_hero_t& hero)
{
	return hero.y - HERO_HALF_HEIGHT;
}

static bool __is_solid_above(const tpmn_model_t& model)
{
	const float x = __get_test_above_x(model.hero);

	float y = __get_test_above_y(model.hero);
	y -= 1;

	return HP_SOLID == __get_tile_info_for_position(x, y, model).hero_pass;
}

static bool __in_box(const float x, const float y, const float box_center_x, const float box_center_y, const float half_aspect)
{
	return
		x > (box_center_x - half_aspect) && x < (box_center_x + half_aspect) &&
		y >(box_center_y - half_aspect) && y < (box_center_y + half_aspect);
}

static float __get_test_left_x(const tpmn_hero_t& hero)
{
	return hero.x - HERO_HALF_WIDTH;
}

static float __get_test_right_x(const tpmn_hero_t& hero)
{
	return hero.x + HERO_HALF_WIDTH;
}

static bool __self_in_box(const float box_center_x, const float box_center_y, const float half_aspect, const tpmn_hero_t& hero)
{
	return
		__in_box(__get_test_left_x(hero), hero.y, box_center_x, box_center_y, half_aspect) ||
		__in_box(hero.x, hero.y, box_center_x, box_center_y, half_aspect) ||
		__in_box(__get_test_right_x(hero), hero.y, box_center_x, box_center_y, half_aspect);
}

static bool __whip_collide(const float x, const float y, const float half_aspect, const tpmn_hero_t& hero)
{
	const float MIN_X = tpmn_hero_whip_min_x(hero);
	const float MAX_X = MIN_X + HERO_WHIP_LENGTH;
	const float Y = tpmn_hero_whip_y(hero);

	if (Y > (y - half_aspect) && Y < (y + half_aspect))
	{
		if (MIN_X < (x + half_aspect) && MAX_X >(x - half_aspect))
		{
			return true;
		}
	}

	return false;
}

static tpmn_events_t __test_roller(
	const tpmn_events_t& input, const float now, const float old_y,
	tpmn_model_t& model, tpmn_enemy_t* r)
{
	tpmn_events_t result = input;

	//crush test
	if (__self_in_box(r->x, r->y, ROLLER_HALF_ASPECT, model.hero))
	{
		if (model.hero.y > r->y)
		{
			result.bits |= TPMN_EVENT_BIT_HERO_DIE;
			return result;
		}
		else
		{
			switch (r->type)
			{
			case TPMN_LOGIC_INDEX_BROWNBLOB:
				//bounce
				model.hero.y = old_y;
				model.hero.sx *= .5f;
				model.hero.sy *= -1.f;
				break;

			case TPMN_LOGIC_INDEX_BLUEBLOB:
				//bounce
				model.hero.y = old_y;
				model.hero.sx *= .5f;
				model.hero.sy *= -.5f;

				//kill roller
				result.bits = TPMN_EVENT_BIT_ROLLER_DIE;
				result.roller_type = r->type;
				result.roller_x = r->x;
				result.roller_y = r->y;

				r->x = tpmn_offset_to_world_x(r->spawn_offset);
				r->y = tpmn_offset_to_world_y(r->spawn_offset);
				r->spawn_time = now + ENEMY_RESPAWN_TIME;

				break;

			default:
				//kill player
				result.bits |= TPMN_EVENT_BIT_HERO_DIE;
				return result;
			}
		}
	}
	//whip test
	else if (r->type != TPMN_LOGIC_INDEX_BLUEBLOB && tpmn_hero_whip_extended(now, model.hero) && __whip_collide(r->x, r->y, ROLLER_HALF_ASPECT, model.hero))
	{
		//kill roller
		result.bits |= TPMN_EVENT_BIT_ROLLER_DIE;
		result.roller_type = r->type;
		result.roller_x = r->x;
		result.roller_y = r->y;

		r->x = tpmn_offset_to_world_x(r->spawn_offset);
		r->y = tpmn_offset_to_world_y(r->spawn_offset);
		r->spawn_time = now + ENEMY_RESPAWN_TIME;
	}

	return result;
}

static bool __can_bite(const tpmn_enemy_t& plant, const tpmn_hero_t& hero)
{
	return tpmn_plant_hero_distance(plant, hero) < TPMN_PLANT_BITE_DISTANCE;
}

static tpmn_events_t __test_plant(
	const tpmn_events_t& input, const float now,
	tpmn_model_t& model, tpmn_enemy_t* p)
{
	tpmn_events_t result = input;

	//bite player?
	if (__can_bite(*p, model.hero))
	{
		//player death
		result.bits |= TPMN_EVENT_BIT_HERO_DIE;
		return result;
	}
	else if (tpmn_hero_whip_extended(now, model.hero) && __whip_collide(p->x, p->y, PLANT_HALF_ASPECT, model.hero))
	{
		//plant death / respawn
		result.bits |= TPMN_EVENT_BIT_PLANT_DIE;
		result.plant_type = p->type;
		result.plant_x = p->x;
		result.plant_y = p->y;

		p->x = tpmn_offset_to_world_x(p->spawn_offset);
		p->y = tpmn_offset_to_world_y(p->spawn_offset);
		p->spawn_time = now + ENEMY_RESPAWN_TIME;
	}

	return result;
}

static uint32_t __test_bat(
	const float now,
	tpmn_model_t& model, tpmn_enemy_t* b)
{
	uint32_t result = 0;

	if (!b->scared)
	{
		//collide
		if (__self_in_box(b->x, b->y, BAT_RADIUS, model.hero))
		{
			//push hero around
			model.hero.sx += b->vector_x * BAT_SPEED;
			model.hero.sy += b->vector_y * BAT_SPEED;
		}
		//whip test
		else if (tpmn_hero_whip_extended(now, model.hero) && __whip_collide(b->x, b->y, BAT_RADIUS, model.hero))
		{
			b->scared = true;
			b->state = TPMN_BAT_S_FLEEING;
			b->change_time = now + BAT_SCARED_TIME;

			result |= TPMN_EVENT_BIT_BAT_FLEE;
		}
	}

	return result;
}

static tpmn_events_t __test_slider(
	const tpmn_events_t& input, const float now, const float old_y,
	tpmn_model_t& model, tpmn_enemy_t* s)
{
	tpmn_events_t result = input;

	if (__self_in_box(s->x, s->y, SLIDER_RADIUS, model.hero))
	{
		//if player below slider, he dies
		if (model.hero.y > s->y)
		{
			result.bits |= TPMN_EVENT_BIT_HERO_DIE;
			return result;
		}
		else
		{
			//bounce
			model.hero.y = old_y;
			model.hero.sx *= .5f;
			model.hero.sy *= -1.f;

			//kill slider
			result.bits |= TPMN_EVENT_BIT_SLIDER_DIE;
			result.slider_x = s->x;
			result.slider_y = s->y;
			result.slider_speed = s->speed;

			s->x = tpmn_offset_to_world_x(s->spawn_offset);
			s->y = tpmn_offset_to_world_y(s->spawn_offset);
			s->spawn_time = now + ENEMY_RESPAWN_TIME;
		}
	}

	return result;
}

static tpmn_events_t __enemy_collision_tests(
	const tpmn_events_t& input, const float old_y,
	tpmn_model_t& model)
{
	const float now = tpmn_model_now(model);

	tpmn_events_t result = input;

	for (tpmn_enemy_t* e = model.enemy; e < model.enemy + model.num_enemy; ++e)
	{
		switch (e->type)
		{
		default:
			assert(0);
			break;

		case TPMN_LOGIC_INDEX_SPIKYGREEN:
		case TPMN_LOGIC_INDEX_BLUEBLOB:
		case TPMN_LOGIC_INDEX_BROWNBLOB:
			if (now > e->spawn_time)
			{
				result = __test_roller(result, now, old_y, model, e);
				if (TPMN_EVENT_BIT_HERO_DIE & result.bits)
					return result;
			}
			break;

		case TPMN_LOGIC_INDEX_PLANT:
		case TPMN_LOGIC_INDEX_SCORPION:
			if (now > e->spawn_time)
			{
				result = __test_plant(result, now, model, e);
				if (TPMN_EVENT_BIT_HERO_DIE & result.bits)
					return result;
			}
			break;

		case TPMN_LOGIC_INDEX_PENGUIN:
			if (now > e->spawn_time)
			{
				result = __test_slider(result, now, old_y, model, e);
				if (TPMN_EVENT_BIT_HERO_DIE & result.bits)
					return result;
			}
			break;

		case TPMN_LOGIC_INDEX_FIREDUDE:
			if (__self_in_box(e->x, e->y, FIREDUDE_RADIUS, model.hero))
			{
				result.bits |= TPMN_EVENT_BIT_HERO_DIE;
				return result;
			}
			break;

		case TPMN_LOGIC_INDEX_BAT:
			result.bits |= __test_bat(now, model, e);
			break;

		case TPMN_LOGIC_INDEX_ICICLE:
		case TPMN_LOGIC_INDEX_SANDBLOCK:
			if (now > e->spawn_time)
			{
				if (__self_in_box(e->x, e->y, ICICLE_HALF_ASPECT, model.hero))
				{
					result.bits |= TPMN_EVENT_BIT_HERO_DIE;
					return result;
				}
			}
			break;
		}
	}

	return result;
}

static float __get_test_below(const tpmn_hero_t& hero)
{
	return hero.y + HERO_HALF_HEIGHT;
}

static float __get_test_below_x(const bool second, const tpmn_hero_t& hero)
{
	if (second)
		return hero.x - HERO_HALF_WIDTH / 2 + HERO_HALF_WIDTH;

	return hero.x - HERO_HALF_WIDTH / 2;
}

static bool __is_solid_below(const tpmn_model_t& model, const float x, const float y)
{
	const tile_info_t& info = __get_tile_info_for_position(x, y, model);

	if (model.hero.sy >= 0)
		return info.hero_pass == HP_SOLID || info.hero_pass == HP_PLATFORM;

	return info.hero_pass == HP_SOLID;
}

static bool __is_solid_below(const tpmn_model_t& model)
{
	const float Y = __get_test_below(model.hero);

	//bottom left probe
	float x = __get_test_below_x(false, model.hero);
	bool solid = __is_solid_below(model, x, Y);
	if (!solid)
	{
		//bottom right probe
		x = __get_test_below_x(true, model.hero);
		solid = __is_solid_below(model, x, Y);
	}

	return solid;
}

static float __get_test_left_right_y(const bool second, const tpmn_hero_t& hero)
{
	if (second)
		return hero.y + 4;

	return hero.y - 4;
}

static bool __is_solid_left(const tpmn_model_t& model)
{
	const float X = __get_test_left_x(model.hero);
	float y = __get_test_left_right_y(false, model.hero);
	bool solid = __get_tile_info_for_position(X, y, model).hero_pass == HP_SOLID;
	if (!solid)
	{
		y = __get_test_left_right_y(true, model.hero);
		solid = __get_tile_info_for_position(X, y, model).hero_pass == HP_SOLID;
	}

	return solid;
}

static bool __is_solid_right(const tpmn_model_t& model)
{
	//bottom right probe
	const float X = __get_test_right_x(model.hero);
	float y = __get_test_left_right_y(false, model.hero);
	bool solid = __get_tile_info_for_position(X, y, model).hero_pass == HP_SOLID;
	if (!solid)
	{
		//top right probe
		y = __get_test_left_right_y(true, model.hero);
		solid = __get_tile_info_for_position(X, y, model).hero_pass == HP_SOLID;
	}

	return solid;
}


static uint32_t __world_collision_tests(const float old_x, const float old_y, tpmn_model_t& model)
{
	uint32_t result = 0;

	//below test
	if (__is_solid_below(model))
	{
		if (model.hero.air_bit)
		{
			result |= TPMN_EVENT_BIT_HERO_LANDED;

			model.hero.air_bit = false;
		}

		model.hero.sy = 0;

		//this snapping seems to work
		model.hero.y /= TPMN_TILE_ASPECT;
		model.hero.y = ::floorf(model.hero.y);
		model.hero.y *= TPMN_TILE_ASPECT;
		model.hero.y += TPMN_TILE_ASPECT - HERO_HALF_HEIGHT;
	}
	else
	{
		model.hero.air_bit = true;
	}

	//above test
	const float X = __get_test_above_x(model.hero);
	const float Y = __get_test_above_y(model.hero);
	const bool SOLIDABOVE = __get_tile_info_for_position(X, Y, model).hero_pass == HP_SOLID;
	if (SOLIDABOVE)
	{
		model.hero.sy = 0;

		model.hero.y /= TPMN_TILE_ASPECT;
		model.hero.y = ::floorf(model.hero.y);
		model.hero.y *= TPMN_TILE_ASPECT;
		model.hero.y += HERO_HALF_HEIGHT;
	}

	//left test
	if (__is_solid_left(model))
	{
		model.hero.sx = 0;

		model.hero.x /= TPMN_TILE_ASPECT;
		model.hero.x = ::floorf(model.hero.x);
		model.hero.x *= TPMN_TILE_ASPECT;
		model.hero.x += HERO_HALF_WIDTH;
	}

	//right test
	if (__is_solid_right(model))
	{
		model.hero.sx = 0;

		model.hero.x /= TPMN_TILE_ASPECT;
		model.hero.x = ::floorf(model.hero.x);
		model.hero.x *= TPMN_TILE_ASPECT;
		model.hero.x += TPMN_TILE_ASPECT - HERO_HALF_WIDTH;
	}

	//sanity check
	const tile_info_t& info = __get_tile_info_for_position(model.hero.x, model.hero.y, model);
	if (info.hero_pass == HP_SOLID)
	{
		model.hero.x = old_x;
		model.hero.y = old_y;
	}

	return result;
}

static void __bounds_tests(tpmn_hero_t& hero)
{
	if (hero.x < 0)
	{
		hero.sx = -hero.sx;
		hero.x = 0;
	}
	else if (hero.x >= TPMN_WORLD_WIDTH * TPMN_TILE_ASPECT)
	{
		hero.sx = -hero.sx;
		hero.x = TPMN_WORLD_WIDTH * TPMN_TILE_ASPECT - 1;
	}

	if (hero.y < 0)
	{
		hero.sy = 0;
		hero.y = 0;
	}
	else if (hero.y >= TPMN_WORLD_HEIGHT * TPMN_TILE_ASPECT)
	{
		hero.sy = -hero.sy;
		hero.y = TPMN_WORLD_HEIGHT * TPMN_TILE_ASPECT - 1;
	}
}

static void __icicle_idle(const tpmn_model_t& model, tpmn_enemy_t* i)
{
	const int32_t x = __world_to_grid(i->x);
	if (model.hero.spawn_time < 0.f && __world_to_grid(model.hero.x) == x)
	{
		const int32_t hy = __world_to_grid(model.hero.y);
		int32_t y = __world_to_grid(i->y);

		while (1)
		{
			const tile_info_t& info = model_get_tile_info(tpmn_model_get_tile(x, y, model), false, model);
			if (HP_CLEAR == info.hero_pass)
			{
				if (y == hy)
				{
					i->change_time = tpmn_model_now(model) + ICICLE_PENDING_TIME;
					i->state = TPMN_ICICLE_S_PENDING;
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

static void __roller_update(const float now, const tpmn_model_t& model, tpmn_enemy_t* r)
{
	if (now > r->spawn_time)
	{
		const float old_x = r->x;

		r->x += r->speed * TPMN_SECONDS_PER_TICK;

		//must walk on HP_SOLID or HP_PLATFORM
		if (HP_CLEAR == __get_tile_info_for_position(r->x, r->y + TPMN_TILE_ASPECT, model).hero_pass)
		{
			r->x = old_x;
			r->speed = -r->speed;
		}

		//side checks
		if (r->speed < 0)
		{
			if ((r->x - 8) < 0 || HP_CLEAR != __get_tile_info_for_position(r->x - 8, r->y, model).hero_pass)
			{
				r->x = old_x;
				r->speed = -r->speed;
			}
		}
		else
		{
			if ((r->x + 8) > (TPMN_WORLD_WIDTH * TPMN_TILE_ASPECT) || HP_CLEAR != __get_tile_info_for_position(r->x + 8, r->y, model).hero_pass)
			{
				r->x = old_x;
				r->speed = -r->speed;
			}
		}
	}
}

static void __firedude_update(const tpmn_model_t& model, tpmn_enemy_t* f)
{
	const float old_x = f->x;
	const float old_y = f->y;

	f->x += f->vector_x * FIREDUDE_SPEED * TPMN_SECONDS_PER_TICK;
	f->y += f->vector_y * FIREDUDE_SPEED * TPMN_SECONDS_PER_TICK;

	if (HP_SOLID == __get_tile_info_for_position(f->x + FIREDUDE_RADIUS, f->y, model).hero_pass ||
		HP_SOLID == __get_tile_info_for_position(f->x - FIREDUDE_RADIUS, f->y, model).hero_pass ||
		HP_SOLID == __get_tile_info_for_position(f->x, f->y + FIREDUDE_RADIUS, model).hero_pass ||
		HP_SOLID == __get_tile_info_for_position(f->x, f->y - FIREDUDE_RADIUS, model).hero_pass ||
		f->x < 0 ||
		f->y < 0 ||
		f->x > TPMN_WORLD_WIDTH * TPMN_TILE_ASPECT ||
		f->y > TPMN_WORLD_HEIGHT * TPMN_TILE_ASPECT)
	{
		f->x = old_x;
		f->y = old_y;
		__random_vector(f->vector_x, f->vector_y);
	}
}

static void __bat_update(const float now, const tpmn_model_t& model, tpmn_enemy_t* b)
{
	switch (b->state)
	{
	case TPMN_BAT_S_PASSIVE:
		//if not scared anymore, look for hero
		if (b->change_time < now)
		{
			//clear damaged flag
			b->scared = false;

			//check if hero in range
			if (__vector_length(b->x - model.hero.x, b->y - model.hero.y) < BAT_SEEK_RADIUS)
			{
				//go active
				b->state = TPMN_BAT_S_ATTACKING;
				b->change_time = now;
			}
		}
		break;

	case TPMN_BAT_S_ATTACKING:
	{
		const float old_x = b->x;
		const float old_y = b->y;

		//if not avoiding, seek target
		if (b->change_time < now)
		{
			b->vector_x = model.hero.x - b->x;
			b->vector_y = model.hero.y - b->y;
			__normalize_vector(b->vector_x, b->vector_y);
		}

		//move
		b->x += b->vector_x * BAT_SPEED * TPMN_SECONDS_PER_TICK;
		b->y += b->vector_y * BAT_SPEED * TPMN_SECONDS_PER_TICK;

		//tile collisions
		if (HP_SOLID == __get_tile_info_for_position(b->x + BAT_RADIUS, b->y, model).hero_pass ||
			HP_SOLID == __get_tile_info_for_position(b->x - BAT_RADIUS, b->y, model).hero_pass ||
			HP_SOLID == __get_tile_info_for_position(b->x, b->y + BAT_RADIUS, model).hero_pass ||
			HP_SOLID == __get_tile_info_for_position(b->x, b->y - BAT_RADIUS, model).hero_pass ||

			//world bounds
			b->x < 0 ||
			b->y < 0 ||
			b->x > TPMN_WORLD_WIDTH * TPMN_TILE_ASPECT ||
			b->y > TPMN_WORLD_HEIGHT * TPMN_TILE_ASPECT)
		{
			b->x = old_x;
			b->y = old_y;

			//avoid for a time
			__random_vector(b->vector_x, b->vector_y);
			b->change_time = now + BAT_AVOID_TIME;
		}

		//if off spawn screen, go home
		if (tpmn_screen_x(tpmn_offset_to_world_x(b->spawn_offset)) != tpmn_screen_x(b->x) || tpmn_screen_y(tpmn_offset_to_world_y(b->spawn_offset)) != tpmn_screen_y(b->y))
		{
			b->scared = true;
			b->state = TPMN_BAT_S_FLEEING;
			b->change_time = now + BAT_SCARED_TIME;
		}
	}
	break;

	case TPMN_BAT_S_FLEEING:
	{
		//seek spawnpoint
		{
			b->vector_x = tpmn_offset_to_world_x(b->spawn_offset) - b->x;
			b->vector_y = tpmn_offset_to_world_y(b->spawn_offset) - b->y;
			__normalize_vector(b->vector_x, b->vector_y);
			b->x += b->vector_x * BAT_SPEED * TPMN_SECONDS_PER_TICK;
			b->y += b->vector_y * BAT_SPEED * TPMN_SECONDS_PER_TICK;
		}

		//re-attach to spawn point?
		if (__vector_length(tpmn_offset_to_world_x(b->spawn_offset) - b->x, tpmn_offset_to_world_y(b->spawn_offset) - b->y) < BAT_RADIUS)
		{
			b->x = tpmn_offset_to_world_x(b->spawn_offset);
			b->y = tpmn_offset_to_world_y(b->spawn_offset);
			b->state = TPMN_BAT_S_PASSIVE;
		}

		//not scared anymore?
		if (b->change_time < now)
		{
			b->scared = false;
			b->state = TPMN_BAT_S_ATTACKING;
		}
	}
	break;
	}
}

static tpmn_events_t __slider_update(const tpmn_events_t& input, const float now, const tpmn_model_t& model, tpmn_enemy_t* s)
{
	tpmn_events_t result = input;

	if (now > s->spawn_time)
	{
		const float old_x = s->x;

		s->speed += s->speed * SLIDER_FRICTION * TPMN_SECONDS_PER_TICK;

		if (::fabsf(s->speed) < SLIDER_MIN_SPEED)
		{
			if (s->speed < 0)
			{
				s->speed -= SLIDER_IMPULSE;
			}
			else
			{
				s->speed += SLIDER_IMPULSE;
			}
			if (tpmn_screen_x(s->x) == tpmn_screen_x(model.hero.x) && tpmn_screen_y(s->y) == tpmn_screen_y(model.hero.y))
				result.bits |= TPMN_EVENT_BIT_SLIDER_IMPULSE;
		}

		s->x += s->speed * TPMN_SECONDS_PER_TICK;

		//must walk on HP_SOLID or HP_PLATFORM
		if (HP_CLEAR == __get_tile_info_for_position(s->x, s->y + TPMN_TILE_ASPECT, model).hero_pass)
		{
			s->x = old_x;
			s->speed = -s->speed;
		}

		//side checks
		if (s->speed < 0)
		{
			if ((s->x - 8) < 0 || HP_CLEAR != __get_tile_info_for_position(s->x - 8, s->y, model).hero_pass)
			{
				s->x = old_x;
				s->speed = -s->speed;
			}
		}
		else
		{
			if ((s->x + 8) > (TPMN_WORLD_WIDTH * TPMN_TILE_ASPECT) || HP_CLEAR != __get_tile_info_for_position(s->x + 8, s->y, model).hero_pass)
			{
				s->x = old_x;
				s->speed = -s->speed;
			}
		}
	}

	return result;
}

static void __icicle_update(const float now, const tpmn_model_t& model, tpmn_enemy_t* i)
{
	if (now > i->spawn_time)
	{
		switch (i->state)
		{
		case TPMN_ICICLE_S_IDLE:
			__icicle_idle(model, i);
			break;

		case TPMN_ICICLE_S_PENDING:
			if (i->change_time < now)
			{
				i->speed = 0;
				i->state = TPMN_ICICLE_S_FALLING;
			}
			break;

		case TPMN_ICICLE_S_FALLING:
			i->speed += TPMN_GRAVITY * TPMN_SECONDS_PER_TICK;
			i->y += i->speed * TPMN_SECONDS_PER_TICK;

			if (HP_CLEAR != __get_tile_info_for_position(i->x, i->y, model).hero_pass)
			{
				i->state = TPMN_ICICLE_S_IDLE;

				i->x = tpmn_offset_to_world_x(i->spawn_offset);
				i->y = tpmn_offset_to_world_y(i->spawn_offset);
				i->spawn_time = now + ENEMY_RESPAWN_TIME;
			}
			break;
		}
	}
}

static const tpmn_portal_t* __portal_for_target_world(const char* target_world, const tpmn_model_t& model)
{
	assert(target_world);
	for (const tpmn_portal_t* p = model.level.portals; p < model.level.portals + model.level.num_portals; ++p)
	{
		if (0 == ::strcmp(p->target_world, target_world))
			return p;
	}
	return nullptr;
}

static const tpmn_portal_t* __portal_for_offset(const uint32_t offset, const tpmn_model_t& model)
{
	for (const tpmn_portal_t* p = model.level.portals; p < model.level.portals + model.level.num_portals; ++p)
	{
		if (p->offset == offset)
			return p;
	}
	return nullptr;
}

//public
//public
//public
//public

bool tpmn_model_load_world(const char* file, const bool new_game, tpmn_model_t& model)
{
	::strcpy_s(model.last_world, model.world);
	::strcpy_s(model.world, file);

	fs_blob_t contents = fs_file_contents(file);
	assert(contents.data);
	iterator_t it;
	it.it = (uint8_t*)contents.data;

	//version check
	const uint32_t VERSION = __uint(it);

	if (FILE_VERSION == VERSION)
	{
		model.level = {};
		model.level.magic = 'n' << 24 | 'm' << 16 | 'p' << 8 | 't';

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

				if (tile_offset < TPMN_WORLD_SIZE)	//bad level size fix!
					model.level.tiles[tile_offset].index = tile_index;
			}
			break;

			case CHUNK_START:
			{
				const uint32_t start = __uint(it);

				model.level.start = start;
			}
			break;

			case CHUNK_BACKGROUND:
			{
				const uint32_t screen_offset = __uint(it);
				const uint32_t screen_index = __uint(it);

				model.level.screens[screen_offset] = screen_index;
			}
			break;

			//legacy
			case CHUNK_PORTAL:
			{
				const uint32_t portal_offset = __uint(it);
				const char* portal_target_world = __utf(it);

				assert(model.level.num_portals < _countof(model.level.portals));
				tpmn_portal_t* p = model.level.portals + model.level.num_portals++;
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

				assert(model.level.num_portals < _countof(model.level.portals));
				tpmn_portal_t* p = model.level.portals + model.level.num_portals++;
				p->offset = portal_offset;
				p->server_count = portal_server_count;
				::strcpy_s(p->target_world, portal_target_world);
			}
			break;

			case CHUNK_TRACK:
			{
				const uint32_t music_track = __uint(it);

				model.level.music_track = music_track;
			}
			break;

			case CHUNK_INFO:
			{
				const uint32_t info_offset = __uint(it);
				const uint32_t info_id = __uint(it);

				assert(model.level.num_infos < _countof(model.level.infos));
				tpmn_info_t* i = model.level.infos + model.level.num_infos++;
				i->offset = info_offset;
				i->id = info_id;
			}
			break;
			}
		}

		model.play_bit = true;

		model.num_enemy = 0;
		for (uint32_t i = 0; i < TPMN_WORLD_SIZE; ++i)
			__logic_spawn(i, model.level.tiles[i].index, model);

		{
			const tpmn_portal_t* p = nullptr;
			if (model.last_world[0])
				p = __portal_for_target_world(model.last_world, model);
			if (p)
			{
				model.hero.x = tpmn_offset_to_world_x(p->offset);
				model.hero.y = tpmn_offset_to_world_y(p->offset);
				model.hero.checkpoint = p->offset;
			}
			else
			{
				model.hero.x = tpmn_offset_to_world_x(model.level.start);
				model.hero.y = tpmn_offset_to_world_y(model.level.start);
				model.hero.checkpoint = model.level.start;
			}
		}
		model.hero.sx = 0.f;
		model.hero.sy = 0.f;
		model.hero.input = 0;
		model.hero.spawn_time = -1.;
		model.hero.keys = 0;
		model.hero.checkpoint_keys = 0;
		model.hero.right_bit = false;
		model.hero.air_bit = false;
		if (new_game)
		{
			model.hero.fixed_servers = {};
			model.hero.checkpoint_fixed_servers = {};
		}
		model.hero.whip_time = -100.;
	}

	delete[] contents.data;

	return FILE_VERSION == VERSION;
}

bool tpmn_model_init(tpmn_model_t& model)
{
	const fs_blob_t CONTENTS = fs_file_contents(ASSET_COMMON_INFO);
	const bool RESULT = CONTENTS.data && sizeof(model.tile_info) == CONTENTS.size;
	if (RESULT)
	{
		::memcpy(&model.tile_info, CONTENTS.data, CONTENTS.size);
		for (tile_info_t* i = model.tile_info; i < model.tile_info + TPMN_MAX_TILE; ++i)
		{
			i->hero_pass = tpmn_change_endianness(i->hero_pass);
			i->special = tpmn_change_endianness(i->special);
		}
	}
	delete[] CONTENTS.data;
	return RESULT;
}

tpmn_events_t tpmn_model_update(tpmn_model_t& model)
{
	++model.tick;
	if (!model.play_bit)
		return {};	//do not load a new world

	const float now = tpmn_model_now(model);

	tpmn_events_t result{};

	for (tpmn_enemy_t* e = model.enemy; e < model.enemy + model.num_enemy; ++e)
	{
		switch (e->type)
		{
		default:
			assert(0);
			break;

		case TPMN_LOGIC_INDEX_SPIKYGREEN:
		case TPMN_LOGIC_INDEX_BLUEBLOB:
		case TPMN_LOGIC_INDEX_BROWNBLOB:
			__roller_update(now, model, e);
			break;

		case TPMN_LOGIC_INDEX_PLANT:
		case TPMN_LOGIC_INDEX_SCORPION:
			break;

		case TPMN_LOGIC_INDEX_PENGUIN:
			result = __slider_update(result, now, model, e);
			break;

		case TPMN_LOGIC_INDEX_FIREDUDE:
			__firedude_update(model, e);
			break;

		case TPMN_LOGIC_INDEX_BAT:
			__bat_update(now, model, e);
			break;

		case TPMN_LOGIC_INDEX_ICICLE:
		case TPMN_LOGIC_INDEX_SANDBLOCK:
			__icicle_update(now, model, e);
			break;
		}
	}

	if (model.hero.spawn_time < 0.f)
	{
		float acc = 0.f;

		//facing
		if (tpmn_hero_left_input(model.hero))
			model.hero.right_bit = false;
		else if (tpmn_hero_right_input(model.hero))
			model.hero.right_bit = true;

		//switches
		if (TPMN_HERO_FLAGS_DOWN & model.hero.input)
		{
			const tpmn_portal_t* p = __portal_for_offset(tpmn_world_to_offset(model.hero.x, model.hero.y), model);
			if (p && model.hero.fixed_servers.count >= p->server_count)
				//if (p)
				return { p->target_world };
		}

		//whip
		if (TPMN_HERO_FLAGS_WHIP & model.hero.input)
		{
			model.hero.whip_time = now;
			result.bits |= TPMN_EVENT_BIT_HERO_WHIP;
		}

		//x acceleration
		if (tpmn_hero_left_input(model.hero) && !tpmn_hero_right_input(model.hero))
			acc = -1;
		else if (tpmn_hero_right_input(model.hero) && !tpmn_hero_left_input(model.hero))
			acc = 1;
		if (acc != 0)
			model.hero.sx += acc * __acceleration(model) * TPMN_SECONDS_PER_TICK;

		//ground friction
		if (!model.hero.air_bit)
			__friction(model);

		//limits
		if (model.hero.sx > HERO_MAX_SPEEDX)
			model.hero.sx = HERO_MAX_SPEEDX;
		else if (model.hero.sx < -HERO_MAX_SPEEDX)
			model.hero.sx = -HERO_MAX_SPEEDX;

		//jumping
		if (__jumping(model.hero))
		{
			if (!model.hero.air_bit && !__is_solid_above(model))
			{
				model.hero.air_bit = true;
				model.hero.sy = -HERO_JUMP;
				result.bits |= TPMN_EVENT_BIT_HERO_JUMP;
			}
		}

		//gravity
		model.hero.sy += TPMN_GRAVITY * TPMN_SECONDS_PER_TICK;
		if (model.hero.sy > HERO_MAX_SPEEDY)
			model.hero.sy = HERO_MAX_SPEEDY;
		else if (model.hero.sy < -HERO_MAX_SPEEDY)
			model.hero.sy = -HERO_MAX_SPEEDY;

		//movement
		const float old_x = model.hero.x;
		const float old_y = model.hero.y;
		model.hero.x += model.hero.sx * TPMN_SECONDS_PER_TICK;
		model.hero.y += model.hero.sy * TPMN_SECONDS_PER_TICK;

		//a bunch of tests...
		if (S_LETHAL == __get_tile_info_for_position(model.hero.x, model.hero.y, model).special)
		{
			result.bits |= TPMN_EVENT_BIT_HERO_DIE;
		}
		else
		{
			result = __enemy_collision_tests(result, old_y, model);
			result.bits |= __world_collision_tests(old_x, old_y, model);
			__bounds_tests(model.hero);
		}
		if (TPMN_EVENT_BIT_HERO_DIE & result.bits)
		{
			model.hero.spawn_time = now + HERO_DEAD_TIME;
		}
	}//hero alive
	else
	{
		if (now > model.hero.spawn_time)
		{
			//restore state
			model.hero.x = tpmn_offset_to_world_x(model.hero.checkpoint);
			model.hero.y = tpmn_offset_to_world_y(model.hero.checkpoint);
			model.hero.sx = 0.f;
			model.hero.sy = 0;
			model.hero.input = 0;
			model.hero.air_bit = false;
			model.hero.spawn_time = -1.;
			model.hero.keys = model.hero.checkpoint_keys;
			model.hero.fixed_servers = model.hero.checkpoint_fixed_servers;
			model.hero.whip_time = -100.;

			result.bits |= TPMN_EVENT_BIT_BACK_TO_CHECKPOINT;
		}
	}

	//tile events
	if (model.hero.spawn_time < 0.f)
	{
		const tpmn_tile_t& tile = __get_tile_for_position(model, model.hero.x, model.hero.y);
		switch (tile.index)
		{
		case TPMN_LOGIC_INDEX_CHECKPOINT:
			//store state
			model.hero.checkpoint = tpmn_world_to_offset(model.hero.x, model.hero.y);
			model.hero.checkpoint_keys = model.hero.keys;
			model.hero.checkpoint_fixed_servers = model.hero.fixed_servers;

			result.bits |= TPMN_EVENT_BIT_CHECKPOINT;
			break;

		case TPMN_LOGIC_INDEX_SERVER:
			if (__add_fixed_server(model.world, tpmn_world_to_offset(model.hero.x, model.hero.y), model.hero))
				result.bits |= TPMN_EVENT_BIT_FIXED_SERVER;
			break;

		case TPMN_LOGIC_INDEX_KEY0:
			if (0 == (model.hero.keys & TPMN_HERO_KEY0))
			{
				model.hero.keys |= TPMN_HERO_KEY0;
				result.bits |= TPMN_EVENT_BIT_KEY;
			}
			break;

		case TPMN_LOGIC_INDEX_KEY1:
			if (0 == (model.hero.keys & TPMN_HERO_KEY1))
			{
				model.hero.keys |= TPMN_HERO_KEY1;
				result.bits |= TPMN_EVENT_BIT_KEY;
			}
			break;

		case TPMN_LOGIC_INDEX_KEY2:
			if (0 == (model.hero.keys & TPMN_HERO_KEY2))
			{
				model.hero.keys |= TPMN_HERO_KEY2;
				result.bits |= TPMN_EVENT_BIT_KEY;
			}
			break;
		}
	}//tile events when player is alive

	return result;
}//void model_update(tpmn_model_t& model)

const tpmn_tile_t& tpmn_model_get_tile(const int32_t x, const int32_t y, const tpmn_model_t& model)
{
	return __get_tile(model, x, y);
}

float tpmn_model_now(const tpmn_model_t& model)
{
	return model.tick * TPMN_SECONDS_PER_TICK;
}

bool model_is_server_fixed(const char* world, const uint32_t offset, const tpmn_hero_t& hero)
{
	for (const tpmn_server_t* fs = hero.fixed_servers.server; fs < hero.fixed_servers.server + hero.fixed_servers.count; ++fs)
	{
		if (0 == ::strcmp(fs->world, world) && fs->offset == offset)
			return true;
	}

	return false;
}

const tpmn_info_t* model_info_for_offset(const uint32_t offset, const tpmn_model_t& model)
{
	for (const tpmn_info_t* i = model.level.infos; i < model.level.infos + model.level.num_infos; ++i)
	{
		if (i->offset == offset)
			return i;
	}
	return nullptr;
}

uint32_t model_screen(const int32_t x, const int32_t y, const tpmn_model_t& model)
{
	return model.level.screens[x + y * TPMN_SCREENS_X];
}

const tile_info_t& model_get_tile_info(const tpmn_tile_t& tile, const bool replace, const tpmn_model_t& model)
{
	if (replace)
	{
		switch (tile.index)
		{
		case TPMN_LOGIC_INDEX_KEY0BLOCK:
			if (model.hero.keys & TPMN_HERO_KEY0)
				return model.tile_info[TPMN_LOGIC_INDEX_AIR];
			break;

		case TPMN_LOGIC_INDEX_KEY1BLOCK:
			if (model.hero.keys & TPMN_HERO_KEY1)
				return model.tile_info[TPMN_LOGIC_INDEX_AIR];
			break;

		case TPMN_LOGIC_INDEX_KEY2BLOCK:
			if (model.hero.keys & TPMN_HERO_KEY2)
				return model.tile_info[TPMN_LOGIC_INDEX_AIR];
			break;
		}
	}

	return model.tile_info[tile.index];
}

bool tpmn_hero_is_checkpoint(const int32_t x, const int32_t y, const tpmn_hero_t& hero)
{
	return (uint32_t)(x + y * TPMN_WORLD_WIDTH) == hero.checkpoint;
}

bool tpmn_hero_left_input(const tpmn_hero_t& hero)
{
	return (hero.input & TPMN_HERO_FLAGS_LEFT) != 0;
}

bool tpmn_hero_right_input(const tpmn_hero_t& hero)
{
	return (hero.input & TPMN_HERO_FLAGS_RIGHT) != 0;
}

bool tpmn_hero_whipping(const float now, const tpmn_hero_t& hero)
{
	return hero.whip_time > (now - HERO_WHIP_TIME);
}

bool tpmn_hero_whip_extended(const float now, const tpmn_hero_t& hero)
{
	return tpmn_hero_whipping(now, hero) && now > (hero.whip_time + HERO_WHIP_TIME * .5f);
}

float tpmn_hero_whip_min_x(const tpmn_hero_t& hero)
{
	if (hero.right_bit)
		return hero.x + HERO_WHIP_X_OFFSET;

	return hero.x - HERO_WHIP_X_OFFSET - HERO_WHIP_LENGTH;
}

float tpmn_hero_whip_y(const tpmn_hero_t& hero)
{
	return hero.y + HERO_WHIP_Y_OFFSET;
}

uint32_t tpmn_world_to_offset(const float aWorldX, const float aWorldY)
{
	return tpmn_grid_to_offset(__world_to_grid(aWorldX), __world_to_grid(aWorldY));
}

int32_t tpmn_screen_x(const float x)
{
	return (int32_t)(x / (TPMN_TILE_ASPECT * TPMN_TILES_X));
}

int32_t tpmn_screen_y(const float y)
{
	return (int32_t)(y / (TPMN_TILE_ASPECT * TPMN_TILES_Y));
}

float tpmn_offset_to_world_x(const uint32_t anOffset)
{
	return (anOffset % TPMN_WORLD_WIDTH) * TPMN_TILE_ASPECT + TPMN_TILE_ASPECT * .5f;
}

float tpmn_offset_to_world_y(const uint32_t anOffset)
{
	return (anOffset / TPMN_WORLD_WIDTH) * TPMN_TILE_ASPECT + TPMN_TILE_ASPECT * .5f;
}

uint32_t tpmn_grid_to_offset(const int32_t grid_x, const int32_t grid_y)
{
	return grid_x + grid_y * TPMN_WORLD_WIDTH;
}

float tpmn_random_unit()
{
	return (float)::rand() / (float)RAND_MAX;
}

float tpmn_plant_hero_distance(const tpmn_enemy_t& plant, const tpmn_hero_t& hero)
{
	return __vector_length(hero.x - plant.x, hero.y - plant.y);
}

uint16_t tpmn_change_endianness(const uint16_t in)
{
	uint16_t out;
	uint8_t* src = (uint8_t*)&in;
	uint8_t* dst = (uint8_t*)&out;
	dst[0] = src[1];
	dst[1] = src[0];
	return out;
}

uint32_t tpmn_change_endianness(const uint32_t in)
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
