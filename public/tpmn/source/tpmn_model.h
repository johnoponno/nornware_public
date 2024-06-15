#pragma once

#define TPMN_TILE_ASPECT 24

#define TPMN_TILES_X 25
#define TPMN_TILES_Y 10

#define TPMN_MAX_TILE 1024

#define TPMN_LOGIC_INDEX_AIR 0
#define TPMN_LOGIC_INDEX_CHECKPOINT 1
#define TPMN_LOGIC_INDEX_SERVER 4
#define TPMN_LOGIC_INDEX_CHECKPOINT_CURRENT 7
#define TPMN_LOGIC_INDEX_SPIKYGREEN 10
#define TPMN_LOGIC_INDEX_ICICLE 11
#define TPMN_LOGIC_INDEX_SANDBLOCK 12
#define TPMN_LOGIC_INDEX_BLUEBLOB 13
#define TPMN_LOGIC_INDEX_BROWNBLOB 14
#define TPMN_LOGIC_INDEX_PLANT 16
#define TPMN_LOGIC_INDEX_FIREDUDE 17
#define TPMN_LOGIC_INDEX_BAT 19
#define TPMN_LOGIC_INDEX_PENGUIN 20
#define TPMN_LOGIC_INDEX_SCORPION 21
#define TPMN_LOGIC_INDEX_KEY0 161
#define TPMN_LOGIC_INDEX_KEY1 162
#define TPMN_LOGIC_INDEX_KEY2 163
#define TPMN_LOGIC_INDEX_KEY0BLOCK 186
#define TPMN_LOGIC_INDEX_KEY1BLOCK 187
#define TPMN_LOGIC_INDEX_KEY2BLOCK 188

#define TPMN_HERO_FLAGS_DOWN 1
#define TPMN_HERO_FLAGS_WHIP 2
#define TPMN_HERO_FLAGS_LEFT 4
#define TPMN_HERO_FLAGS_RIGHT 8
#define TPMN_HERO_FLAGS_JUMP 16

#define TPMN_HERO_KEY0 1
#define TPMN_HERO_KEY1 2
#define TPMN_HERO_KEY2 4

#define TPMN_ICICLE_S_IDLE 0
#define TPMN_ICICLE_S_PENDING 1
#define TPMN_ICICLE_S_FALLING 2

#define TPMN_BAT_S_PASSIVE 0
#define TPMN_BAT_S_ATTACKING 1
#define TPMN_BAT_S_FLEEING 2

#define TPMN_PLANT_BITE_DISTANCE 24.f

#define TPMN_GRAVITY 1300.f

enum
{
	TPMN_WORLD_WIDTH = TPMN_TILES_X * 10,
	TPMN_WORLD_HEIGHT = TPMN_TILES_Y * 4,
	TPMN_WORLD_SIZE = TPMN_WORLD_WIDTH * TPMN_WORLD_HEIGHT,

	TPMN_SCREENS_X = TPMN_WORLD_WIDTH / TPMN_TILES_X,
	TPMN_SCREENS_Y = TPMN_WORLD_HEIGHT / TPMN_TILES_Y,
};

enum
{
	TPMN_EVENT_BIT_HERO_WHIP = (1 << 0),
	TPMN_EVENT_BIT_HERO_JUMP = (1 << 1),
	TPMN_EVENT_BIT_HERO_LANDED = (1 << 2),
	TPMN_EVENT_BIT_BACK_TO_CHECKPOINT = (1 << 3),
	TPMN_EVENT_BIT_BAT_FLEE = (1 << 4),
	TPMN_EVENT_BIT_HERO_DIE = (1 << 5),
	TPMN_EVENT_BIT_FIXED_SERVER = (1 << 6),
	TPMN_EVENT_BIT_CHECKPOINT = (1 << 7),
	TPMN_EVENT_BIT_KEY = (1 << 8),
	TPMN_EVENT_BIT_ROLLER_DIE = (1 << 9),
	TPMN_EVENT_BIT_PLANT_DIE = (1 << 10),
	TPMN_EVENT_BIT_SLIDER_DIE = (1 << 11),
	TPMN_EVENT_BIT_SLIDER_IMPULSE = (1 << 12),
};

struct tpmn_enemy_t
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

struct tpmn_tile_t
{
	uint32_t index;
};

struct tpmn_portal_t
{
	char target_world[32];
	uint32_t offset;
	uint32_t server_count;
};

struct tpmn_info_t
{
	uint32_t offset;
	uint32_t id;
};

struct level_t
{
	uint32_t magic;
	tpmn_tile_t tiles[TPMN_WORLD_SIZE];
	uint32_t screens[TPMN_SCREENS_X * TPMN_SCREENS_Y];
	tpmn_portal_t portals[16];
	tpmn_info_t infos[16];
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

struct tpmn_server_t
{
	char world[32];
	uint32_t offset;
};

struct tpmn_servers_t
{
	tpmn_server_t server[32];
	uint32_t count;
};

struct tpmn_hero_t
{
	tpmn_servers_t fixed_servers;
	tpmn_servers_t checkpoint_fixed_servers;
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

struct tpmn_model_t
{
	level_t level;
	tile_info_t tile_info[TPMN_MAX_TILE];	//FIXME: immutable after program startup, move out!
	tpmn_enemy_t enemy[64];
	tpmn_hero_t hero;
	char last_world[32];
	char world[32];
	uint32_t num_enemy;
	uint32_t tick;
	uint32_t play_bit;
};

struct tpmn_events_t
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

bool tpmn_model_init(tpmn_model_t& model);
bool tpmn_model_load_world(const char* file, const bool new_game, tpmn_model_t& model);
tpmn_events_t tpmn_model_update(tpmn_model_t& model);

const tpmn_tile_t& tpmn_model_get_tile(const int32_t x, const int32_t y, const tpmn_model_t& model);
float tpmn_model_now(const tpmn_model_t& model);
bool model_is_server_fixed(const char* world, const uint32_t offset, const tpmn_hero_t& hero);
const tpmn_info_t* model_info_for_offset(const uint32_t offset, const tpmn_model_t& model);
uint32_t model_screen(const int32_t x, const int32_t y, const tpmn_model_t& model);
const tile_info_t& model_get_tile_info(const tpmn_tile_t& tile, const bool replace, const tpmn_model_t& model);

bool tpmn_hero_is_checkpoint(const int32_t x, const int32_t y, const tpmn_hero_t& hero);
bool tpmn_hero_left_input(const tpmn_hero_t& hero);
bool tpmn_hero_right_input(const tpmn_hero_t& hero);
bool tpmn_hero_whipping(const float now, const tpmn_hero_t& hero);
bool tpmn_hero_whip_extended(const float now, const tpmn_hero_t& hero);
float tpmn_hero_whip_min_x(const tpmn_hero_t& hero);
float tpmn_hero_whip_y(const tpmn_hero_t& hero);

uint32_t tpmn_world_to_offset(const float aWorldX, const float aWorldY);
int32_t tpmn_screen_x(const float x);
int32_t tpmn_screen_y(const float y);
float tpmn_offset_to_world_x(const uint32_t anOffset);
float tpmn_offset_to_world_y(const uint32_t anOffset);
uint32_t tpmn_grid_to_offset(const int32_t grid_x, const int32_t grid_y);
float tpmn_random_unit();
float tpmn_plant_hero_distance(const tpmn_enemy_t& plant, const tpmn_hero_t& hero);

uint16_t tpmn_change_endianness(const uint16_t in);
uint32_t tpmn_change_endianness(const uint32_t in);

constexpr float TPMN_SECONDS_PER_TICK = 1.f / 60.f;
