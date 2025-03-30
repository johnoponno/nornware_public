#pragma once

#define MLM_M_MELEE_RANGE_X 64
#define MLM_M_MELEE_RANGE_Y 16

#define MLM_M_TICKS_PER_SECOND 60

#define MLM_M_MAX_TILE 1560

#if 0
#define MLM_M_CHAR_HALF_WIDTH 8
#define MLM_M_CHAR_HALF_HEIGHT 16
#else
#define MLM_M_CHAR_HALF_WIDTH 4
#define MLM_M_CHAR_HALF_HEIGHT 8
#endif

#define MLM_M_JUMPER_SPEED -350.f

#define MLM_M_WALKER_SPEED -40.f

#define MLM_M_SWIMMER_ACC 25.f
#define MLM_M_SWIMMER_BRAKE 1.f
#define MLM_M_SWIMMER_MIN_NEXT_VECTOR_TIME 2.f
#define MLM_M_SWIMMER_MAX_NEXT_VECTOR_TIME 5.f

#define MLM_M_FLAGS_DOWN 1
#define MLM_M_FLAGS_LEFT 2
#define MLM_M_FLAGS_RIGHT 4
#define MLM_M_FLAGS_JUMP 8
#define MLM_M_FLAGS_SPRINT 16
#define MLM_M_FLAGS_MELEE 32

#define MLM_M_VISITED_NOT 0
#define MLM_M_VISITED 1
#define MLM_M_VISITED_CHECKPOINT 2

#define MLM_M_TILE_ASPECT 8

#if 0
#define MLM_M_WORLD_WIDTH 384
#define MLM_M_WORLD_HEIGHT 208
#define MLM_M_WORLD_SIZE 79872
static_assert(MLM_M_WORLD_WIDTH* MLM_M_WORLD_HEIGHT == MLM_M_WORLD_SIZE, "wtf?");
#else
#define MLM_M_WORLD_WIDTH 256
#define MLM_M_WORLD_HEIGHT 144
#define MLM_M_WORLD_SIZE 36864
static_assert(MLM_M_WORLD_WIDTH* MLM_M_WORLD_HEIGHT == MLM_M_WORLD_SIZE, "wtf?");
#endif

namespace mlm
{
	enum
	{
		M_COD_IMPALED,
		M_COD_DROWNED,
		M_COD_BURNED,
		M_COD_ELECTRINATED,
		M_COD_WALKER,
		M_COD_WALKER2,
		M_COD_JUMPER,
		M_COD_JUMPER2,
		M_COD_OUT_OF_BOUNDS,

		M_NUM_COD,
	};

	enum
	{
		M_TYPE_AIR,
		M_TYPE_SOLID,
		M_TYPE_PLATFORM,
		M_TYPE_LIQUID,
		M_TYPE_PASSAGE,
		M_TYPE_ABDUCT,
		M_TYPE_SPIKES,
		M_TYPE_FIRE,
		M_TYPE_ELECTRICITY,

		M_NUM_TYPE,
	};

	enum
	{
		M_LOGIC_CHECKPOINT,
		M_LOGIC_FLOWER,
		M_LOGIC_EXIT,
		M_LOGIC_EXIT_INVISIBLE,

		//enemies
		M_LOGIC_WALKER,
		M_LOGIC_JUMPER,
		M_LOGIC_SWIMMER,
		M_LOGIC_WALKER2,
		M_LOGIC_JUMPER2,
		M_LOGIC_SWIMMER2,

		//triswitch
		M_LOGIC_TRISWITCH,
		M_LOGIC_RED,
		M_LOGIC_GREEN,
		M_LOGIC_BLUE,

		//biswitch
		M_LOGIC_BISWITCH,
		M_LOGIC_FALSE,
		M_LOGIC_TRUE,

		M_NUM_LOGIC,

		//non-tiles
		M_LOGIC_BULLET,
	};

	enum
	{
		M_EVT_DO_SWITCHES,
		M_EVT_HERO_JUMP,
		M_EVT_HERO_LANDED,
		M_EVT_HERO_DIED,
		M_EVT_VISIT,
		M_EVT_CHECKPOINT,
		M_EVT_BACK_TO_CHECKPOINT,
		M_EVT_PICKED_FLOWER,
		M_EVT_JUMPER_IN,
		M_EVT_JUMPER_OUT,
		M_EVT_EXIT,
		M_EVT_BULLET_FIRE,
		M_EVT_MOB_PRUNE,
		M_EVT_MELEE_HIT,
	};

	enum struct m_mode_t
	{
		IDLE,
		PLAY,
	};

	struct m_immutable_t
	{
		explicit m_immutable_t();

		uint32_t tile_type[MLM_M_MAX_TILE];
		uint32_t logic_indices[M_NUM_LOGIC];
		uint32_t start;

		uint16_t world_tiles[MLM_M_WORLD_SIZE];
		uint16_t world_null_tile;
	};

	constexpr struct
	{
		const char* name;
		uint32_t default;
	} M_LOGIC_INFO[M_NUM_LOGIC] =
	{
		{ "INDEX_CHECKPOINT", 1 },
		{ "INDEX_FLOWER", 4 },
		{ "INDEX_EXIT", 6 },
		{ "INDEX_EXIT_INVISIBLE", 7 },

		{ "INDEX_WALKER", 10 },
		{ "INDEX_JUMPER", 11 },
		{ "INDEX_SWIMMER", 12 },
		{ "INDEX_WALKER2", UINT32_MAX },
		{ "INDEX_JUMPER2", UINT32_MAX },
		{ "INDEX_SWIMMER2", UINT32_MAX },

		{ "INDEX_TRISWITCH", 215 },
		{ "INDEX_RED", 161 },
		{ "INDEX_GREEN", 162 },
		{ "INDEX_BLUE", 163 },

		{ "INDEX_BISWITCH", 0 },
		{ "INDEX_FALSE", 0 },
		{ "INDEX_TRUE", 0 },
	};

	constexpr float M_SECONDS_PER_TICK = 1.f / (float)MLM_M_TICKS_PER_SECOND;

	constexpr float M_CHAR_MAX_AIR_SPEED_X = 166.f;
	constexpr float M_CHAR_MAX_AIR_SPEED_Y = 500.f;;
}
