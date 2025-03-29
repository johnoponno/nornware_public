#include "stdafx.h"
#include "m_work.h"

#include "../microlib/p_context.h"
#include "m_mutable.h"

#define ASSET_MODEL "model.txt"
#define JUMPER_GRAVITY 300.f

#define SINGLETON_KEY 1

#define LOGIC_KIND "sabk::model::Logic"

#define TILE_KIND "sabk::model::Tile"
#define TILE_INDEX "myIndex"

#define LEVEL_START "myStart"
#define LEVEL_PRINCESS "myPrincess"
#define LEVEL_KING "myKing"
#define LEVEL_ROYALTY "myRoyalty"

#define TILE_INFO_KIND "sabk::model::TileInfo"
#define TILE_INFO_HEROPASS "myHeroPass"
#define TILE_INFO_SPECIAL "mySpecial"

#define START_KIND "sabk::model::LevelInfo"

#define WALKER_RELOAD_TIME 1.f
#define WALKER_BULLET_TIME .15f
#define BULLET_SPEED 200.f
#define HEAVY_RANGE 10
#define HEAVY_HEALTH 10

namespace mlm
{
	void m_character_set_input(const m_immutable_t& im, const uint8_t flags, m_mutable_t& mu);
	void m_character_update(const m_immutable_t& im, const uint32_t tick, m_mutable_t& mu, m_events_t& events);

#if 0
	static const MAP_F32(WALKER_RELOAD_TIME, nullptr, 1.f);
	static const MAP_F32(WALKER_BULLET_TIME, nullptr, .15f);
	static const MAP_F32(BULLET_SPEED, nullptr, 200.f);
	static const MAP_U32(HEAVY_RANGE, nullptr, 10);
	static const MAP_U32(HEAVY_HEALTH, nullptr, 10);
#endif

	static void __walker_collide(
		const m_immutable_t& in_im, const m_mutable_t& in_mu, const c_vec2f_t& in_old_position, const c_vec2f_t& in_offset, const uint8_t in_test_value,
		m_mob_t& out_mob)
	{
		if (in_test_value != m_tile_type_at_position(out_mob.position + in_offset, true, in_im, in_mu))
		{
			out_mob.position = in_old_position;
			out_mob.walker.speed = -out_mob.walker.speed;
		}
	}

	static void __swimmer_new_vector(
		const uint32_t in_tick,
		c_xorshift128_t& out_prng, m_mob_t& out_mob)
	{
		out_mob.swimmer.vector = { out_prng.float32(-1.f, 1.f), out_prng.float32(-1.f, 1.f) };
		out_mob.swimmer.vector = c_normalize(out_mob.swimmer.vector);
		out_mob.swimmer.next_vector_time = in_tick * M_SECONDS_PER_TICK + out_prng.float32(MLM_M_SWIMMER_MIN_NEXT_VECTOR_TIME, MLM_M_SWIMMER_MAX_NEXT_VECTOR_TIME);
	}

	static bool __swimmer_collide(
		const m_immutable_t& in_im, const uint32_t in_tick, const c_vec2f_t& in_old_position, const c_vec2f_t& in_offset,
		m_mutable_t& out_mu, m_mob_t& out_mob)
	{
		if (M_TYPE_LIQUID != m_tile_type_at_position(out_mob.position + in_offset, false, in_im, out_mu))
		{
			out_mob.position = in_old_position;
			out_mob.swimmer.speed *= -.5f;
			__swimmer_new_vector(in_tick, out_mu.prng, out_mob);
			return true;
		}

		return false;
	}

	static void __mobs_update(
		const uint32_t in_tick, const m_immutable_t& in_im,
		m_mutable_t& out_mu, m_events_t& out_events)
	{
		std::vector<m_mob_t> append;

		for (m_mob_t& mob : out_mu.mobs)
		{
			switch (mob.type)
			{
			default:
				assert(0);
				break;

			case M_LOGIC_WALKER:
			case M_LOGIC_WALKER2:
			{
				const float NOW = in_tick * M_SECONDS_PER_TICK;

				if (m_mob_sees_hero(in_im, out_mu, mob))
				{
					if (NOW > mob.walker.fire_time)
					{
						{
							m_mob_t bullet;
							bullet.position = mob.position;
							bullet.position.y -= 1.f;
							if (mob.walker.speed > 0.f)
							{
								bullet.position.x += 16.f;
								bullet.bullet_speed = BULLET_SPEED;
							}
							else
							{
								bullet.position.x -= 16.f;
								bullet.bullet_speed = -BULLET_SPEED;
							}
							bullet.type = M_LOGIC_BULLET;
							bullet.prune = 0;
							append.push_back(bullet);
							m_events_push(bullet.position, M_NUM_COD, M_EVT_BULLET_FIRE, out_events);
						}

						if (mob.walker.burst < 2)
						{
							++mob.walker.burst;
							mob.walker.fire_time = NOW + WALKER_BULLET_TIME;
						}
						else
						{
							mob.walker.burst = 0;
							mob.walker.fire_time = NOW + WALKER_RELOAD_TIME;
						}
					}
				}
				else
				{
					//so there's always a wind-up when first seeing the hero...
					mob.walker.burst = 0;
					mob.walker.fire_time = NOW + WALKER_RELOAD_TIME;

					const c_vec2f_t OP = mob.position;

					mob.position.x += mob.walker.speed * M_SECONDS_PER_TICK;

					//check down
					//2024-07-08: walkers can now traverse SOLID and PLATFORM
#if 0
					__walker_collide(im, mu, OP, { 0.f, MLM_M_TILE_ASPECT * 2.f }, M_TYPE_SOLID, mob);
#else
					switch (m_tile_type_at_position(mob.position + c_vec2f_t{ 0.f, MLM_M_TILE_ASPECT * 2.f }, true, in_im, out_mu))
					{
					default:
						mob.position = OP;
						mob.walker.speed = -mob.walker.speed;
						break;

					case M_TYPE_SOLID:
					case M_TYPE_PLATFORM:
						break;
					}
#endif

					//check sides
					if (mob.walker.speed < 0.f)
						__walker_collide(in_im, out_mu, OP, { -8.f, MLM_M_TILE_ASPECT }, M_TYPE_AIR, mob);
					else
						__walker_collide(in_im, out_mu, OP, { 8.f, MLM_M_TILE_ASPECT }, M_TYPE_AIR, mob);
				}
			}
			break;

			case M_LOGIC_JUMPER:
			case M_LOGIC_JUMPER2:
				if (mob.jumper.start_time < in_tick * M_SECONDS_PER_TICK)
				{
					mob.jumper.speed += JUMPER_GRAVITY * M_SECONDS_PER_TICK;
					mob.position.y += mob.jumper.speed * M_SECONDS_PER_TICK;
					if (mob.position.y > mob.jumper.start)
					{
						mob.position.y = mob.jumper.start;
						mob.jumper.speed = MLM_M_JUMPER_SPEED;
					}

					const uint32_t TYPE = m_tile_type_at_position(mob.position, false, in_im, out_mu);
					switch (TYPE)
					{
					case M_TYPE_LIQUID:
						if (M_TYPE_LIQUID != mob.jumper.previous_special)
							m_events_push(mob.position, M_NUM_COD, M_EVT_JUMPER_IN, out_events);
						break;

					default:
						if (M_TYPE_LIQUID == mob.jumper.previous_special)
							m_events_push(mob.position, M_NUM_COD, M_EVT_JUMPER_OUT, out_events);
						break;
					}
					mob.jumper.previous_special = TYPE;
				}
				break;

			case M_LOGIC_SWIMMER:
			case M_LOGIC_SWIMMER2:
			{
				const c_vec2f_t OP = mob.position;

				if (mob.swimmer.next_vector_time < in_tick * M_SECONDS_PER_TICK)
					__swimmer_new_vector(in_tick, out_mu.prng, mob);

				mob.swimmer.speed += mob.swimmer.vector * (MLM_M_SWIMMER_ACC * M_SECONDS_PER_TICK);
				mob.swimmer.speed -= mob.swimmer.speed * (MLM_M_SWIMMER_BRAKE * M_SECONDS_PER_TICK);
				mob.position += mob.swimmer.speed * M_SECONDS_PER_TICK;

				__swimmer_collide(in_im, in_tick, OP, { -8.f, -8.f }, out_mu, mob) ||
					__swimmer_collide(in_im, in_tick, OP, { 8.f, -8.f }, out_mu, mob) ||
					__swimmer_collide(in_im, in_tick, OP, { -8.f, 8.f }, out_mu, mob) ||
					__swimmer_collide(in_im, in_tick, OP, { 8.f, 8.f }, out_mu, mob);
			}
			break;

			case M_LOGIC_BULLET:
				mob.position.x += mob.bullet_speed * M_SECONDS_PER_TICK;
				if (M_TYPE_SOLID == m_tile_type_at_position(mob.position, true, in_im, out_mu))
					mob.prune = 1;
				break;
			}
		}//for (m_mob_t& mob : mu.mobs)

		//prune
		{
			auto it = out_mu.mobs.begin();
			while (out_mu.mobs.cend() != it)
			{
				if (it->prune)
				{
					m_events_push(it->position, it->type, M_EVT_MOB_PRUNE, out_events);
					it = out_mu.mobs.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		//append
		for (const m_mob_t& mob : append)
			out_mu.mobs.push_back(mob);
	}

	//public
	//public
	//public
	//public

	bool m_in_world(const int32_t in_x, const int32_t in_y)
	{
		return
			in_x >= 0 &&
			in_x < MLM_M_WORLD_WIDTH &&
			in_y >= 0 &&
			in_y < MLM_M_WORLD_HEIGHT;
	}

	uint32_t m_world_to_offset(const c_vec2f_t& in_world)
	{
		return m_grid_to_offset(m_world_to_grid(in_world));
	}

	c_vec2i_t m_world_to_grid(const c_vec2f_t& in_world)
	{
		return{ (int32_t)(in_world.x / MLM_M_TILE_ASPECT), (int32_t)(in_world.y / MLM_M_TILE_ASPECT) };
	}

	uint32_t m_grid_to_offset(const c_vec2i_t& in_grid)
	{
		return in_grid.x + in_grid.y * MLM_M_WORLD_WIDTH;
	}

	c_vec2f_t m_offset_to_world(const uint32_t in_offset)
	{
		c_vec2f_t wp;

		wp.x = (float)((in_offset % MLM_M_WORLD_WIDTH) * MLM_M_TILE_ASPECT + MLM_M_TILE_ASPECT / 2);
		wp.y = (float)((in_offset / MLM_M_WORLD_WIDTH) * MLM_M_TILE_ASPECT + MLM_M_TILE_ASPECT / 2);

		return wp;
	}

	c_vec2f_t m_grid_to_world(const int32_t in_x, const int32_t in_y)
	{
		return{ ((float)in_x + .5f) * MLM_M_TILE_ASPECT, ((float)in_y + .5f) * MLM_M_TILE_ASPECT };
	}

	void m_spawn(
		const uint32_t in_tick, const m_immutable_t& in_im,
		m_mutable_t& out_mu)
	{
		out_mu.mobs.clear();

		for (
			uint32_t offset = 0;
			offset < MLM_M_WORLD_SIZE;
			++offset
			)
		{
			const uint16_t INDEX = in_im.world_tiles[offset];

			if (INDEX == in_im.logic_indices[M_LOGIC_WALKER])
			{
				m_mob_t mob;
				mob.position = m_offset_to_world(offset) + c_vec2f_t{ 0.f, -12.f };
				mob.type = M_LOGIC_WALKER;
				mob.prune = 0;

				mob.walker.speed = MLM_M_WALKER_SPEED;
				mob.walker.fire_time = 0.f;
				mob.walker.burst = 0;
				mob.walker.health = HEAVY_HEALTH;

				out_mu.mobs.push_back(mob);
			}
			else if (INDEX == in_im.logic_indices[M_LOGIC_WALKER2])
			{
				m_mob_t mob;
				mob.position = m_offset_to_world(offset) + c_vec2f_t{ 0.f, -12.f };
				mob.type = M_LOGIC_WALKER2;
				mob.prune = 0;

				mob.walker.speed = MLM_M_WALKER_SPEED;
				mob.walker.fire_time = 0.f;
				mob.walker.burst = 0;
				mob.walker.health = HEAVY_HEALTH;

				out_mu.mobs.push_back(mob);
			}
			else if (INDEX == in_im.logic_indices[M_LOGIC_JUMPER])
			{
				m_mob_t mob;
				mob.position = m_offset_to_world(offset);
				mob.type = M_LOGIC_JUMPER;
				mob.prune = 0;

				mob.jumper.start = m_offset_to_world(offset).y;
				mob.jumper.speed = MLM_M_JUMPER_SPEED;
				mob.jumper.start_time = in_tick * M_SECONDS_PER_TICK + out_mu.prng.float32(0.f, 2.f);
				mob.jumper.previous_special = M_TYPE_LIQUID;

				out_mu.mobs.push_back(mob);
			}
			else if (INDEX == in_im.logic_indices[M_LOGIC_JUMPER2])
			{
				m_mob_t mob;
				mob.position = m_offset_to_world(offset);
				mob.type = M_LOGIC_JUMPER2;
				mob.prune = 0;

				mob.jumper.start = m_offset_to_world(offset).y;
				mob.jumper.speed = MLM_M_JUMPER_SPEED;
				mob.jumper.start_time = in_tick * M_SECONDS_PER_TICK + out_mu.prng.float32(0.f, 2.f);
				mob.jumper.previous_special = M_TYPE_LIQUID;

				out_mu.mobs.push_back(mob);
			}
			else if (INDEX == in_im.logic_indices[M_LOGIC_SWIMMER])
			{
				m_mob_t mob;
				mob.position = m_offset_to_world(offset);
				mob.type = M_LOGIC_SWIMMER;
				mob.prune = 0;

				mob.swimmer.speed = {};
				mob.swimmer.vector = {};
				mob.swimmer.next_vector_time = 0.f;
				__swimmer_new_vector(in_tick, out_mu.prng, mob);

				out_mu.mobs.push_back(mob);
			}
			else if (INDEX == in_im.logic_indices[M_LOGIC_SWIMMER2])
			{
				m_mob_t mob;
				mob.position = m_offset_to_world(offset);
				mob.type = M_LOGIC_SWIMMER2;
				mob.prune = 0;

				mob.swimmer.speed = {};
				mob.swimmer.vector = {};
				mob.swimmer.next_vector_time = 0.f;
				__swimmer_new_vector(in_tick, out_mu.prng, mob);

				out_mu.mobs.push_back(mob);
			}
		}
	}

	void m_restart(
		const uint32_t in_tick, const m_mode_t in_mode, const m_immutable_t& in_im,
		m_mutable_t& out_mu)
	{
		out_mu.prng = c_xorshift128_t::make();

		if (m_mode_t::IDLE != in_mode)
		{
			out_mu.start_time = in_tick * M_SECONDS_PER_TICK;
		}
		else
		{
			out_mu.start_time = 0.f;
		}
		out_mu.exit_time = 0.f;

		//clear all visited state
		for (
			uint32_t i = 0;
			i < MLM_M_WORLD_SIZE;
			++i
			)
			out_mu.world_visited[i] = MLM_M_VISITED_NOT;

		m_spawn(in_tick, in_im, out_mu);

		out_mu.bi = 0;
		out_mu.tri = 0;
		out_mu.checkpoint_offset = in_im.start;
		out_mu.checkpoint_bi = 0;
		out_mu.checkpoint_tri = 0;
		m_character_restart(m_offset_to_world(in_im.start), out_mu);
	}

	void m_set_start(
		const c_vec2i_t& in_grid,
		m_immutable_t& out_im)
	{
		out_im.start = m_grid_to_offset(in_grid);
	}

	void m_persist(const m_immutable_t& in_im)
	{
		p_context_t context;

		context.push_int32(START_KIND, SINGLETON_KEY, LEVEL_START, in_im.start);

		for (
			uint32_t i = 0;
			i < M_NUM_LOGIC;
			++i
			)
			context.push_int32(LOGIC_KIND, SINGLETON_KEY, M_LOGIC_INFO[i].name, in_im.logic_indices[i]);

		for (
			uint32_t i = 0;
			i < MLM_M_MAX_TILE;
			++i
			)
			context.push_int32(TILE_INFO_KIND, 1 + i, TILE_INFO_HEROPASS, in_im.tile_type[i]);

		for (
			uint32_t i = 0;
			i < MLM_M_WORLD_SIZE;
			++i
			)
			context.push_int32(TILE_KIND, i, TILE_INDEX, in_im.world_tiles[i]);

		context.write(ASSET_MODEL);
	}

	void m_set_hero_input(
		const m_immutable_t& in_im, const uint8_t in_flags,
		m_mutable_t& out_mu)
	{
		m_character_set_input(in_im, in_flags, out_mu);
	}

	void m_do_switches(
		const m_immutable_t& in_im,
		m_mutable_t& out_mu, m_events_t& out_events)
	{
		const uint16_t T = m_tile(out_mu.hero_position, in_im);

		//biswitch
		if (m_logic_is(M_LOGIC_BISWITCH, T, in_im))
		{
			++out_mu.bi;
			out_mu.bi %= 2;
			m_events_push(out_mu.hero_position, M_NUM_COD, M_EVT_DO_SWITCHES, out_events);
		}
		//triswitch
		if (m_logic_is(M_LOGIC_TRISWITCH, T, in_im))
		{
			++out_mu.tri;
			out_mu.tri %= 3;
			m_events_push(out_mu.hero_position, M_NUM_COD, M_EVT_DO_SWITCHES, out_events);
		}
	}

	void m_back_to_checkpoint(m_mutable_t& out_mu, m_events_t& out_events)
	{
		//restore switch states
		out_mu.bi = out_mu.checkpoint_bi;
		out_mu.tri = out_mu.checkpoint_tri;

		//restored visited state
		for (
			uint32_t i = 0;
			i < MLM_M_WORLD_SIZE;
			++i
			)
		{
			//all that isn't V_CHECKPOINT gets put back to M_VISITED_NOT
			if (MLM_M_VISITED == out_mu.world_visited[i])
				out_mu.world_visited[i] = MLM_M_VISITED_NOT;
		}

		//hero restart
		m_character_restart(m_offset_to_world(out_mu.checkpoint_offset), out_mu);

		m_events_push(out_mu.hero_position, M_NUM_COD, M_EVT_BACK_TO_CHECKPOINT, out_events);
	}

	void m_load(m_immutable_t& out_im)
	{
		p_context_t context;

		context.read(ASSET_MODEL);

		out_im.start = context.pull_int32(START_KIND, SINGLETON_KEY, LEVEL_START, 0);

		for (
			uint32_t i = 0;
			i < M_NUM_LOGIC;
			++i
			)
			out_im.logic_indices[i] = context.pull_int32(LOGIC_KIND, SINGLETON_KEY, M_LOGIC_INFO[i].name, M_LOGIC_INFO[i].default);

		for (
			uint32_t i = 0;
			i < MLM_M_MAX_TILE;
			++i
			)
			out_im.tile_type[i] = (uint8_t)context.pull_int32(TILE_INFO_KIND, 1 + i, TILE_INFO_HEROPASS, M_TYPE_SOLID);

#if 0
		//conversion from legacy world size
		for (uint32_t i = 0; i < 60000; ++i)
		{
			//legacy x and y
			const int32_t x = i % 250;
			const int32_t y = i / 250;

			//convert to new offset (some fall outside as old world was 240 tall)
			const int32_t offset = x + y * MLM_M_WORLD_WIDTH;
			assert(offset >= 0);
			if (offset < M_WORLD_SIZE)
				im.world_tiles[offset] = (uint16_t)p_context_pull_int32(TILE_KIND, i, TILE_INDEX, 0, context);
		}
#else
		//new world size
		for (
			uint32_t i = 0;
			i < MLM_M_WORLD_SIZE;
			++i
			)
			out_im.world_tiles[i] = (uint16_t)context.pull_int32(TILE_KIND, i, TILE_INDEX, 0);
#endif
	}

	bool m_mob_sees_hero(const m_immutable_t& in_im, const m_mutable_t& in_mu, const m_mob_t& in_mob)
	{
		if (!m_character_alive(in_mu))
			return false;

		const uint32_t HERO_OFFSET = m_world_to_offset(in_mu.hero_position);
		uint32_t range = HEAVY_RANGE;
		uint32_t o = m_world_to_offset(in_mob.position);
		while (
			range &&
			M_TYPE_AIR == m_tile_type_at_offset(o, true, in_im, in_mu)
			)
		{
			if (
				(o + MLM_M_WORLD_WIDTH) == HERO_OFFSET ||
				o == HERO_OFFSET ||
				(o - MLM_M_WORLD_WIDTH) == HERO_OFFSET ||
				(o - MLM_M_WORLD_WIDTH * 2) == HERO_OFFSET
				)
			{
				return true;
			}

			if (in_mob.walker.speed < 0.f)
				--o;
			else
				++o;
			--range;
		}

		return false;
	}

	m_events_t m_update(
		const m_immutable_t& in_im, const uint32_t in_tick,
		m_mutable_t& out_mu)
	{
		m_events_t result{};

		if (m_game_active(out_mu))
		{
			__mobs_update(in_tick, in_im, out_mu, result);

			m_character_update(in_im, in_tick, out_mu, result);
			if (m_character_alive(out_mu))
			{
				const uint16_t T = m_tile(out_mu.hero_position, in_im);
				const uint8_t V = m_visited(out_mu.hero_position, out_mu);

				//checkpoint
				if (m_logic_is(M_LOGIC_CHECKPOINT, T, in_im))
				{
					//m_checkpoint(model, result);
					//void m_checkpoint(m_mutable_t & model, m_events_t & events)
					{
						//store new start position and switch state
						const uint32_t OLD_CHECKPOINT = out_mu.checkpoint_offset;
						out_mu.checkpoint_offset = m_world_to_offset(out_mu.hero_position);
						out_mu.checkpoint_bi = out_mu.bi;
						out_mu.checkpoint_tri = out_mu.tri;

						//promote visited tiles to persistent state
						for (
							uint32_t i = 0;
							i < MLM_M_WORLD_SIZE;
							++i
							)
						{
							//promote M_VISITED to M_VISITED_CHECKPOINT
							if (MLM_M_VISITED == out_mu.world_visited[i])
								out_mu.world_visited[i] = MLM_M_VISITED_CHECKPOINT;
						}

						//only trigger view / sound if the checkpoint differs
						if (out_mu.checkpoint_offset != OLD_CHECKPOINT)
							m_events_push(out_mu.hero_position, M_NUM_COD, M_EVT_CHECKPOINT, result);
					}
				}
				//flower
				else if (m_logic_is(M_LOGIC_FLOWER, T, in_im))
				{
					if (MLM_M_VISITED_NOT == V)
						m_events_push(out_mu.hero_position, M_NUM_COD, M_EVT_PICKED_FLOWER, result);
				}
				//exit / teleport
				else if (
					m_logic_is(M_LOGIC_EXIT, T, in_im) ||
					m_logic_is(M_LOGIC_EXIT_INVISIBLE, T, in_im)
					)
				{
					m_events_push(out_mu.hero_position, M_NUM_COD, M_EVT_EXIT, result);

					/*
					//end of game
					if (model.level == (M_NUM_LEVELS - 1))
					{
						model.exit_time = tick * M_SECONDS_PER_TICK;
					}
					//between levels
					else
					{
						m_model_restart(tick, m_mode_t::play, model.level + 1, im, model);
					}
					*/
					out_mu.exit_time = in_tick * M_SECONDS_PER_TICK;
				}

				//eventified!
				//m_model_visit(model, result);
				//void m_model_visit(m_mutable_t& out_mu, m_events_t& events)
				{
					const c_vec2i_t G = m_world_to_grid(out_mu.hero_position);
					if (m_in_world(G.x, G.y))
					{
						uint8_t& visited = out_mu.world_visited[G.x + G.y * MLM_M_WORLD_WIDTH];

						//only change V_NOT (checkpoints stay)
						if (MLM_M_VISITED_NOT == visited)
							visited = MLM_M_VISITED;
					}

					m_events_push(out_mu.hero_position, M_NUM_COD, M_EVT_VISIT, result);
				}
			}

			/*
			if (m_royalty_on == m_model_level(im, model).royalty)
			{
				m_character_update(im, tick, model.princess, model, result);
				m_character_update(im, tick, model.king, model, result);
			}
			*/
		}

		return result;
	}

	//FIXME: 0 is implicitly "air"...
	uint32_t m_tile_type(const uint16_t in_tile, const bool in_switches, const m_immutable_t& in_im, const m_mutable_t& in_mu)
	{
		assert(in_tile <= MLM_M_MAX_TILE);

		if (in_switches)
		{
			//biswitch
			if (m_logic_is(M_LOGIC_FALSE, in_tile, in_im))
			{
				if (in_mu.bi != 0)
					return in_im.tile_type[0];
			}
			else if (m_logic_is(M_LOGIC_TRUE, in_tile, in_im))
			{
				if (in_mu.bi != 1)
					return in_im.tile_type[0];
			}
			//triswitch
			else if (m_logic_is(M_LOGIC_RED, in_tile, in_im))
			{
				if (in_mu.tri != 0)
					return in_im.tile_type[0];
			}
			else if (m_logic_is(M_LOGIC_GREEN, in_tile, in_im))
			{
				if (in_mu.tri != 1)
					return in_im.tile_type[0];
			}
			else if (m_logic_is(M_LOGIC_BLUE, in_tile, in_im))
			{
				if (in_mu.tri != 2)
					return in_im.tile_type[0];
			}
		}

		return in_im.tile_type[in_tile];
	}

	uint32_t m_tile_type_at_offset(const uint32_t in_offset, const bool in_switches, const m_immutable_t& in_im, const m_mutable_t& in_mu)
	{
		return m_tile_type(m_tile(in_offset, in_im), in_switches, in_im, in_mu);
	}

	uint32_t m_tile_type_at_position(const c_vec2f_t& in_position, const bool in_switches, const m_immutable_t& in_im, const m_mutable_t& in_mu)
	{
		return m_tile_type(m_tile(in_position, in_im), in_switches, in_im, in_mu);
	}

	//private implementation
	bool m_game_active(const m_mutable_t& in_mu)
	{
		return in_mu.start_time > in_mu.exit_time;
	}

	/*
	int32_t m_model_elapsed_milliseconds(const uint32_t tick, const m_mutable_t& m)
	{
		if (m_game_active(m))
			return (int32_t)((tick * M_SECONDS_PER_TICK - m.start_time) * 1000.f);

		return (int32_t)((m.exit_time - m.start_time) * 1000.f);
	}
	*/

	bool m_logic_is(const uint32_t in_value, const uint32_t in_index, const m_immutable_t& in_im)
	{
		assert(in_value < _countof(in_im.logic_indices));
		return in_index == in_im.logic_indices[in_value];
	}

	uint32_t m_logic_index_of(const uint32_t in_value, const m_immutable_t& in_im)
	{
		assert(in_value < _countof(in_im.logic_indices));
		return in_im.logic_indices[in_value];
	}

	uint16_t m_tile(const uint32_t in_offset, const m_immutable_t& in_im)
	{
		if (in_offset < MLM_M_WORLD_SIZE)
			return in_im.world_tiles[in_offset];

		return in_im.world_null_tile;
	}

	uint16_t m_tile(const int32_t in_x, const int32_t in_y, const m_immutable_t& in_im)
	{
		return m_tile(m_grid_to_offset({ in_x, in_y }), in_im);
	}

	uint16_t m_tile(const c_vec2f_t& in_position, const m_immutable_t& in_im)
	{
		const c_vec2i_t g = m_world_to_grid(in_position);
		return m_tile(g.x, g.y, in_im);
	}

	uint8_t m_visited(const int32_t in_x, const int32_t in_y, const m_mutable_t& in_mu)
	{
		if (m_in_world(in_x, in_y))
			return in_mu.world_visited[in_x + in_y * MLM_M_WORLD_WIDTH];

		return in_mu.world_null_visited;
	}

	uint8_t m_visited(const c_vec2f_t& in_positio, const m_mutable_t& in_mu)
	{
		const c_vec2i_t g = m_world_to_grid(in_positio);

		return m_visited(g.x, g.y, in_mu);
	}

	void m_events_push(
		const c_vec2f_t& in_position, const uint32_t in_arg, const uint32_t in_type,
		m_events_t& out_events)
	{
		assert(out_events.count < _countof(out_events.events));
		out_events.events[out_events.count++] = { in_position, in_arg, in_type };
	}
}
