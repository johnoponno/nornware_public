#pragma once

namespace tpmn
{
	enum
	{
		TILE_ASPECT = 24,

		TILES_X = 25,
		TILES_Y = 10,

		WORLD_WIDTH = TILES_X * 10,
		WORLD_HEIGHT = TILES_Y * 4,
		WORLD_SIZE = WORLD_WIDTH * WORLD_HEIGHT,

		SCREENS_X = WORLD_WIDTH / TILES_X,
		SCREENS_Y = WORLD_HEIGHT / TILES_Y,

		MAX_TILE = 1024,

		LOGIC_INDEX_AIR = 0,
		LOGIC_INDEX_CHECKPOINT = 1,
		LOGIC_INDEX_SERVER = 4,
		LOGIC_INDEX_CHECKPOINT_CURRENT = 7,
		LOGIC_INDEX_SPIKYGREEN = 10,
		LOGIC_INDEX_ICICLE = 11,
		LOGIC_INDEX_SANDBLOCK = 12,
		LOGIC_INDEX_BLUEBLOB = 13,
		LOGIC_INDEX_BROWNBLOB = 14,
		LOGIC_INDEX_PLANT = 16,
		LOGIC_INDEX_FIREDUDE = 17,
		LOGIC_INDEX_BAT = 19,
		LOGIC_INDEX_PENGUIN = 20,
		LOGIC_INDEX_SCORPION = 21,
		LOGIC_INDEX_KEY0 = 161,
		LOGIC_INDEX_KEY1 = 162,
		LOGIC_INDEX_KEY2 = 163,
		LOGIC_INDEX_KEY0BLOCK = 186,
		LOGIC_INDEX_KEY1BLOCK = 187,
		LOGIC_INDEX_KEY2BLOCK = 188,

		HERO_FLAGS_DOWN = 1,
		HERO_FLAGS_WHIP = 2,
		HERO_FLAGS_LEFT = 4,
		HERO_FLAGS_RIGHT = 8,
		HERO_FLAGS_JUMP = 16,

		HERO_KEY0 = 1,
		HERO_KEY1 = 2,
		HERO_KEY2 = 4,

		ICICLE_S_IDLE = 0,
		ICICLE_S_PENDING = 1,
		ICICLE_S_FALLING = 2,

		BAT_S_PASSIVE = 0,
		BAT_S_ATTACKING = 1,
		BAT_S_FLEEING = 2,
	};

	struct enemy_t
	{
		float x;
		float y;
		float vector_x;
		float vector_y;
		float spawn_time;
		float change_time;
		float speed;
		uint32_t spawn_offset;
		uint32_t type;
		uint32_t state;
		uint32_t scared;
	};

	struct tile_t
	{
		uint32_t index;
	};

	struct portal_t
	{
		char target_world[32];
		uint32_t offset;
		uint32_t server_count;
	};

	struct info_t
	{
		uint32_t offset;
		uint32_t id;
	};

	struct level_t
	{
		uint32_t magic;
		tile_t tiles[WORLD_SIZE];
		uint32_t screens[SCREENS_X * SCREENS_Y];
		portal_t portals[16];
		info_t infos[16];
		uint32_t start;
		uint32_t music_track;
		uint32_t num_portals;
		uint32_t num_infos;
	};

	struct tile_info_t
	{
		uint32_t hero_pass;
		uint32_t special;
	};

	struct server_t
	{
		char world[32];
		uint32_t offset;
	};

	struct servers_t
	{
		server_t server[32];
		uint32_t count;
	};

	struct hero_t
	{
		servers_t fixed_server;
		servers_t checkpoint_fixed_server;
		uint32_t input;
		uint32_t keys;
		uint32_t checkpoint_keys;
		uint32_t checkpoint;
		float x;
		float y;
		float sx;
		float sy;
		float spawn_time;
		float whip_time;
		unsigned right_bit : 1;
		unsigned air_bit : 1;
	};

	struct model_t
	{
		level_t level;
		tile_info_t tile_info[MAX_TILE];	//FIXME: immutable after program startup, move out!
		enemy_t enemy[64];
		hero_t hero;
		char last_world[32];
		char world[32];
		uint32_t num_enemy;
		uint32_t tick;
		uint32_t play_bit;
	};

	enum
	{
		bit_hero_whip = (1 << 0),
		bit_hero_jump = (1 << 1),
		bit_hero_landed = (1 << 2),
		bit_back_to_checkpoint = (1 << 3),
		bit_bat_flee = (1 << 4),
		bit_hero_die = (1 << 5),
		bit_fixed_server = (1 << 6),
		bit_checkpoint = (1 << 7),
		bit_key = (1 << 8),
		bit_roller_die = (1 << 9),
		bit_plant_die = (1 << 10),
		bit_slider_die = (1 << 11),
		bit_slider_impulse = (1 << 12),
	};

	struct model_update_t
	{
		const char* world_to_load;

		uint32_t bits;

		uint32_t roller_type;
		float roller_x;
		float roller_y;

		uint32_t plant_type;
		float plant_x;
		float plant_y;

		float slider_x;
		float slider_y;
		float slider_speed;
	};

	bool model_init(model_t& model);
	bool model_load_world(const char* file, const bool new_game, model_t& model);
	model_update_t model_update(model_t& model);

	const tile_t& model_get_tile(const int32_t x, const int32_t y, const model_t& model);
	float model_now(const model_t& model);
	bool model_is_server_fixed(const char* world, const uint32_t offset, const hero_t& hero);
	const info_t* model_info_for_offset(const uint32_t offset, const model_t& model);
	uint32_t model_screen(const int32_t x, const int32_t y, const model_t& model);
	const tile_info_t& model_get_tile_info(const tile_t& tile, const bool replace, const model_t& model);

	bool hero_is_checkpoint(const int32_t x, const int32_t y, const hero_t& hero);
	bool hero_left_input(const hero_t& hero);
	bool hero_right_input(const hero_t& hero);
	bool hero_whipping(const float now, const hero_t& hero);
	bool hero_whip_extended(const float now, const hero_t& hero);
	float hero_whip_min_x(const hero_t& hero);
	float hero_whip_y(const hero_t& hero);

	uint32_t world_to_offset(const float aWorldX, const float aWorldY);
	int32_t screen_x(const float x);
	int32_t screen_y(const float y);
	float offset_to_world_x(const uint32_t anOffset);
	float offset_to_world_y(const uint32_t anOffset);
	uint32_t grid_to_offset(const int32_t grid_x, const int32_t grid_y);
	float random_unit();
	float plant_hero_distance(const enemy_t& plant, const hero_t& hero);

	uint16_t change_endianness(const uint16_t in);
	uint32_t change_endianness(const uint32_t in);

	extern const float TIME_PER_TICK;
	extern const float PLANT_BITE_DISTANCE;
	extern const float GRAVITY;
}
