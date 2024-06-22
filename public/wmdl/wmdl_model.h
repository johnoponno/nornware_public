#pragma once

#define WMDL_TILE_ASPECT 24

#define WMDL_TILES_X 25
#define WMDL_TILES_Y 10

#define WMDL_MAX_TILE 1024

#define WMDL_LOGIC_INDEX_AIR 0
#define WMDL_LOGIC_INDEX_CHECKPOINT 1
#define WMDL_LOGIC_INDEX_SERVER 4
#define WMDL_LOGIC_INDEX_CHECKPOINT_CURRENT 7
#define WMDL_LOGIC_INDEX_SPIKYGREEN 10
#define WMDL_LOGIC_INDEX_ICICLE 11
#define WMDL_LOGIC_INDEX_SANDBLOCK 12
#define WMDL_LOGIC_INDEX_BLUEBLOB 13
#define WMDL_LOGIC_INDEX_BROWNBLOB 14
#define WMDL_LOGIC_INDEX_PLANT 16
#define WMDL_LOGIC_INDEX_FIREDUDE 17
#define WMDL_LOGIC_INDEX_BAT 19
#define WMDL_LOGIC_INDEX_PENGUIN 20
#define WMDL_LOGIC_INDEX_SCORPION 21
#define WMDL_LOGIC_INDEX_KEY0 161
#define WMDL_LOGIC_INDEX_KEY1 162
#define WMDL_LOGIC_INDEX_KEY2 163
#define WMDL_LOGIC_INDEX_KEY0BLOCK 186
#define WMDL_LOGIC_INDEX_KEY1BLOCK 187
#define WMDL_LOGIC_INDEX_KEY2BLOCK 188

#define WMDL_HERO_FLAGS_DOWN 1
#define WMDL_HERO_FLAGS_WHIP 2
#define WMDL_HERO_FLAGS_LEFT 4
#define WMDL_HERO_FLAGS_RIGHT 8
#define WMDL_HERO_FLAGS_JUMP 16

#define WMDL_HERO_KEY0 1
#define WMDL_HERO_KEY1 2
#define WMDL_HERO_KEY2 4

#define WMDL_ICICLE_S_IDLE 0
#define WMDL_ICICLE_S_PENDING 1
#define WMDL_ICICLE_S_FALLING 2

#define WMDL_BAT_S_PASSIVE 0
#define WMDL_BAT_S_ATTACKING 1
#define WMDL_BAT_S_FLEEING 2

#define WMDL_PLANT_BITE_DISTANCE 24.f

/*
#if HANNAH
#define WMDL_GRAVITY 850.f
#else
#define WMDL_GRAVITY 1300.f
#endif
*/

constexpr float WMDL_SECONDS_PER_TICK = 1.f / 60.f;

constexpr int32_t WMDL_WORLD_WIDTH = WMDL_TILES_X * 10;
constexpr int32_t WMDL_WORLD_HEIGHT = WMDL_TILES_Y * 4;
constexpr int32_t WMDL_WORLD_SIZE = WMDL_WORLD_WIDTH * WMDL_WORLD_HEIGHT;
constexpr int32_t WMDL_SCREENS_X = WMDL_WORLD_WIDTH / WMDL_TILES_X;
constexpr int32_t WMDL_SCREENS_Y = WMDL_WORLD_HEIGHT / WMDL_TILES_Y;

constexpr uint32_t WMDL_EVENT_BIT_HERO_WHIP = (1 << 0);
constexpr uint32_t WMDL_EVENT_BIT_HERO_JUMP = (1 << 1);
constexpr uint32_t WMDL_EVENT_BIT_HERO_LANDED = (1 << 2);
constexpr uint32_t WMDL_EVENT_BIT_BACK_TO_CHECKPOINT = (1 << 3);
constexpr uint32_t WMDL_EVENT_BIT_BAT_FLEE = (1 << 4);
constexpr uint32_t WMDL_EVENT_BIT_HERO_DIE = (1 << 5);
constexpr uint32_t WMDL_EVENT_BIT_FIXED_SERVER = (1 << 6);
constexpr uint32_t WMDL_EVENT_BIT_CHECKPOINT = (1 << 7);
constexpr uint32_t WMDL_EVENT_BIT_KEY = (1 << 8);
constexpr uint32_t WMDL_EVENT_BIT_ROLLER_DIE = (1 << 9);
constexpr uint32_t WMDL_EVENT_BIT_PLANT_DIE = (1 << 10);
constexpr uint32_t WMDL_EVENT_BIT_SLIDER_DIE = (1 << 11);
constexpr uint32_t WMDL_EVENT_BIT_SLIDER_IMPULSE = (1 << 12);

struct wmdl_enemy_t
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

struct wmdl_tile_t
{
	uint32_t index;
};

struct wmdl_portal_t
{
	char target_world[32];
	uint32_t offset;
	uint32_t server_count;
};

struct wmdl_info_t
{
	uint32_t offset;
	uint32_t id;
};

struct wmdl_level_t
{
	uint32_t magic;
	wmdl_tile_t tiles[WMDL_WORLD_SIZE];
	uint32_t screens[WMDL_SCREENS_X * WMDL_SCREENS_Y];
	wmdl_portal_t portals[16];
	wmdl_info_t infos[16];
	uint32_t start;
	uint32_t music_track;
	uint32_t num_portals;
	uint32_t num_infos;
};

struct wmdl_tile_info_t
{
	uint32_t hero_pass;
	uint32_t special;
};

struct wmdl_server_t
{
	char world[32];
	uint32_t offset;
};

struct wmdl_servers_t
{
	wmdl_server_t server[32];
	uint32_t count;
};

struct wmdl_hero_t
{
	wmdl_servers_t fixed_servers;
	wmdl_servers_t checkpoint_fixed_servers;
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

struct wmdl_model_t
{
	wmdl_level_t level;
	wmdl_tile_info_t tile_info[WMDL_MAX_TILE];	//FIXME: immutable after program startup, move out!
	wmdl_enemy_t enemy[64];
	wmdl_hero_t hero;
	char last_world[32];
	char world[32];
	uint32_t num_enemy;
	uint32_t tick;
	uint32_t play_bit;
};

struct wmdl_events_t
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

bool wmdl_model_init(wmdl_model_t& model);
bool wmdl_model_load_world(const char* file, const bool new_game, wmdl_model_t& model);
wmdl_events_t wmdl_model_update(wmdl_model_t& model);

const wmdl_tile_t& wmdl_model_get_tile(const wmdl_model_t& in_model, const int32_t in_x, const int32_t in_y);
float wmdl_model_now(const wmdl_model_t& model);
bool wmdl_model_is_server_fixed(const char* world, const uint32_t offset, const wmdl_hero_t& hero);
const wmdl_info_t* wmdl_model_info_for_offset(const wmdl_model_t& model, const uint32_t offset);
uint32_t wmdl_model_screen(const wmdl_model_t& model, const int32_t x, const int32_t y);
const wmdl_tile_info_t& wmdl_model_get_tile_info(const wmdl_model_t& model, const wmdl_tile_t& tile, const bool replace);

bool wmdl_hero_is_checkpoint(const wmdl_hero_t& hero, const int32_t x, const int32_t y);
bool wmdl_hero_left_input(const wmdl_hero_t& hero);
bool wmdl_hero_right_input(const wmdl_hero_t& hero);
bool wmdl_hero_whipping(const wmdl_hero_t& hero, const float now);
bool wmdl_hero_whip_extended(const wmdl_hero_t& hero, const float now);
float wmdl_hero_whip_min_x(const wmdl_hero_t& hero);
float wmdl_hero_whip_y(const wmdl_hero_t& hero);

uint32_t wmdl_world_to_offset(const float aWorldX, const float aWorldY);
int32_t wmdl_screen_x(const float x);
int32_t wmdl_screen_y(const float y);
float wmdl_offset_to_world_x(const uint32_t anOffset);
float wmdl_offset_to_world_y(const uint32_t anOffset);
uint32_t wmdl_grid_to_offset(const int32_t grid_x, const int32_t grid_y);
float wmdl_random_unit();
float wmdl_plant_hero_distance(const wmdl_enemy_t& plant, const wmdl_hero_t& hero);

uint16_t wmdl_change_endianness(const uint16_t in);
uint32_t wmdl_change_endianness(const uint32_t in);

float wmdl_gravity();
float wmdl_hero_jump();

extern bool wmdl_tune_new;