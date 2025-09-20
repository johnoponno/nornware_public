#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define FINALIZE 1
#define MAKE_LABS 1
#define MAKE_CONTROL_ROOM 1
#define PRE_BARRIERS 1
#define POST_BARRIERS 1

#define TUNNELER_DEFAULT_LIFE 400u
#define REACTOR_ROOM_ASPECT 5
#define LAB_SPACE_ASPECT 5
#define LAB_PANIC_CHANCE 4

//I think I arrived at 33x33 (as opposed to for example 32x32)
//because I wanted there to be an actual "center" to the maps where the reactor would go
#define PLAN_CELLS_ASPECT 33
#define PLAN_CELLS_SIZE 1089
static_assert(PLAN_CELLS_SIZE == PLAN_CELLS_ASPECT * PLAN_CELLS_ASPECT, "wtf?");

#define PLAN_OFFSET_XZ(x, z) ((x) + (z) * PLAN_CELLS_ASPECT)
#define PLAN_OFFSET_DELTA(base, deltaX, deltaZ) ((base) + (deltaX) + (deltaZ) * PLAN_CELLS_ASPECT)
#define PLAN_OFFSET_TILE_X(offset) ((offset) % PLAN_CELLS_ASPECT)
#define PLAN_OFFSET_TILE_Z(offset) ((offset) / PLAN_CELLS_ASPECT)

#define PLAN_REACTOR_POSITION_AXIS 16
static_assert(PLAN_REACTOR_POSITION_AXIS == PLAN_CELLS_ASPECT / 2, "wtf!?");

//values of gen_plan_t::cells;
//everything with bit 1 set is "floor", everything else is "wall"

//TODO: if anything "PLAN_CELL_XXX" added, write static assertions to test!
//TODO: if anything "PLAN_CELL_XXX" added, write static assertions to test!
//TODO: if anything "PLAN_CELL_XXX" added, write static assertions to test!
//TODO: if anything "PLAN_CELL_XXX" added, write static assertions to test!
#define PLAN_CELL_FLOORBIT 1

//"walls" don't have PLAN_CELL_FLOORBIT set
#define PLAN_CELL_WALL 0
#define PLAN_CELL_TECH 2
static_assert(0 == PLAN_CELL_WALL, "value of PLAN_CELL_WALL must be 0!!");//lots of clearing code uses the {} style, so PLAN_CELL_WALL must be 0!
static_assert(0 == (PLAN_CELL_FLOORBIT & PLAN_CELL_WALL), "wtf!?");
static_assert(0 == (PLAN_CELL_FLOORBIT & PLAN_CELL_TECH), "wtf!?");

//"floors" have PLAN_CELL_FLOORBIT set
#define PLAN_CELL_FLOOR 1
#define PLAN_CELL_DATA 3
#define PLAN_CELL_COOLANT 5
#define PLAN_CELL_REPAIR 7
#define PLAN_CELL_DOOR_A 9
#define PLAN_CELL_DOOR_B 11
#define PLAN_CELL_SENTRY 13
#define PLAN_CELL_ARMORY 15
#define PLAN_CELL_FENCE 17
#define PLAN_CELL_BREAKER 19
#define PLAN_CELL_LAB 21
#define PLAN_CELL_PANIC 23
static_assert(PLAN_CELL_FLOORBIT& PLAN_CELL_FLOOR, "wtf!?");
static_assert(PLAN_CELL_FLOORBIT& PLAN_CELL_DATA, "wtf!?");
static_assert(PLAN_CELL_FLOORBIT& PLAN_CELL_COOLANT, "wtf!?");
static_assert(PLAN_CELL_FLOORBIT& PLAN_CELL_REPAIR, "wtf!?");
static_assert(PLAN_CELL_FLOORBIT& PLAN_CELL_DOOR_A, "wtf!?");
static_assert(PLAN_CELL_FLOORBIT& PLAN_CELL_DOOR_B, "wtf!?");
static_assert(PLAN_CELL_FLOORBIT& PLAN_CELL_SENTRY, "wtf!?");
static_assert(PLAN_CELL_FLOORBIT& PLAN_CELL_ARMORY, "wtf!?");
static_assert(PLAN_CELL_FLOORBIT& PLAN_CELL_FENCE, "wtf!?");
static_assert(PLAN_CELL_FLOORBIT& PLAN_CELL_BREAKER, "wtf!?");
static_assert(PLAN_CELL_FLOORBIT& PLAN_CELL_LAB, "wtf!?");
static_assert(PLAN_CELL_FLOORBIT& PLAN_CELL_PANIC, "wtf!?");

//these are based on 8 neighbors:
//-x-z == bit 0
//x-z == bit 1
//+x-z == bit 2
//-xz == bit 3
//+xz == bit 4
//-x-z == bit 5
//x-z == bit 6
//+x-z == bit 7
#define DIRECTION_FLAGS_ZNEG 2
#define DIRECTION_FLAGS_XNEG 8
#define DIRECTION_FLAGS_XPOS 16
#define DIRECTION_FLAGS_ZPOS 64
static_assert((1 << 1) == DIRECTION_FLAGS_ZNEG, "wtf!?");
static_assert((1 << 3) == DIRECTION_FLAGS_XNEG, "wtf!?");
static_assert((1 << 4) == DIRECTION_FLAGS_XPOS, "wtf!?");
static_assert((1 << 6) == DIRECTION_FLAGS_ZPOS, "wtf!?");

#define PLAN_FLOODER_SOLID 0
#define PLAN_FLOODER_UNREACHABLE 1
#define PLAN_FLOODER_REACHABLE 2

#define PLAN_MAX_CORES 160
#define PLAN_MAX_DOORS 160
#define PLAN_MAX_FENCES 128
#define PLAN_MAX_PANICS 16
#define PLAN_MAX_SENTRIES 64
#define PLAN_MAX_BREAKERS 64

#define ROC_TOO_MANY_CORES 1
#define ROC_TOO_MANY_DOORS 2
#define ROC_TOO_MANY_FENCES 4
#define ROC_TOO_MANY_BREAKERS 8
#define ROC_AT_LEAST_ONE_BREAKER 16
#define ROC_TOO_MANY_PANICS 32
#define ROC_TOO_MANY_SENTRIES 64

#define COTW_CAMPAIGN_SEED(year, week) (year << 6 | week)

enum
{
	ROOMS_EMPTY,
	ROOMS_FILLED,
	ROOMS_PILLARS,
	ROOMS_UNKEN,
};

enum
{
	CAMPAIGN_RANDOM_SEED,
	CAMPAIGN_EXPLICIT_SEED,
	CAMPAIGN_RANDOMIZE_ALL,
	CAMPAIGN_MOTD,
	CAMPAIGN_COTW,
	CAMPAIGN_KIT,
	CAMPAIGN_USER,
	CAMPAIGN_GRIND,
};

enum
{
	INFESTATION_NONE,
	INFESTATION_LESS,
	INFESTATION_MORE,
	INFESTATION_TOTAL,
	INFESTATION_UNKEN,
};

enum
{
	CHANCE_NONE,
	CHANCE_MORE,
	CHANCE_FEWER,
	CHANCE_UNKEN,
};

enum
{
	BIT_CLEAR,
	BIT_SET,
	BIT_UNKEN,
};

enum
{
	DIFFICULTY_EASIER,
	DIFFICULTY_EASIER_PLUS,
	DIFFICULTY_HARD,
	DIFFICULTY_HARD_PLUS,
	DIFFICULTY_INSANE,
	DIFFICULTY_RANDOM_ALL,
	DIFFICULTY_RANDOM_EACH,
	DIFFICULTY_UNKEN,
};

enum
{
	POWER_FAILURE_NONE,

	POWER_FAILURE_CHANCE_8,
	POWER_FAILURE_CHANCE_16,
	POWER_FAILURE_CHANCE_32,

	POWER_FAILURE_TIME_30_60,
	POWER_FAILURE_TIME_60_120,
	POWER_FAILURE_TIME_120_240,

	POWER_FAILURE_UNKEN,
};

//"flags_256_entity"
enum
{
	F256E_NOTHING,
	F256E_DOOR_RIGHT_ANGLE,
	F256E_BREACH_OK,
	F256E_SENTRY_OK,
};

//while some campaign modes have implicit options, we still need the explicit state because the simulation uses a c_gen_t internally
//it would be a waste of cpu to have to generate the options on the fly for each read while simulating
//also gen_plan_t exists to simplify / isolate that which a "user plan" dictates (i.e. not cotw stuff)
struct gen_plan_t
{
	uint8_t cells[PLAN_CELLS_SIZE];
	uint32_t airlock;
	uint32_t security_console;
	uint32_t seed;
};

//this is persisted as a single uint64_t
struct gen_options64_t
{
	void set_default_planner();
	void set_default_ruler();

	//0 = infestation_none
	//1 = infestation_less
	//2 = infestation_more
	//3 = infestation_total
	//4 = UNKEN
	uint32_t im_planner_infestation : 3;

	//0 = chance_none
	//1 = chance_more
	//2 = chance_fewer
	//3 = UNKEN
	uint32_t im_planner_chance_doors : 2;

	//0 = chance_none
	//1 = chance_more
	//2 = chance_fewer
	//3 = UNKEN
	uint32_t im_planner_chance_fences : 2;

	//0 = normal
	//1 = inverted
	//2 = UNKEN
	uint32_t im_planner_bit_invert_barriers : 2;

	//0 = rooms_empty
	//1 = rooms_filled
	//2 = rooms_pillars
	//3 = UNKEN
	uint32_t im_planner_rooms : 2;

	//0 = chance_none
	//1 = chance_more
	//2 = chance_fewer
	//3 = UNKEN
	uint32_t im_planner_chance_sentries : 2;

	//0 = more
	//1 = fewer
	//2 = UNKEN
	uint32_t im_planner_bit_datacores : 2;

	//0 = more
	//1 = fewer
	//2 = UNKEN
	uint32_t im_planner_bit_armories : 2;

	//0 = more
	//1 = fewer
	//2 = UNKEN
	uint32_t im_planner_bit_repairs : 2;

	//0 = starts off
	//1 = starts on
	//2 = UNKEN
	uint32_t im_ruler_bit_initial_power : 2;

	//0 = barriers don't open during reactor overload
	//1 = barriers open during reactor overload
	//2 = UNKEN
	uint32_t im_ruler_bit_reactor_safety_protocol : 2;

	//0 = beasts never lurk
	//1 = beasts lurk in infestation
	//2 = UNKEN
	uint32_t im_ruler_bit_beasts_lurk : 2;

	//0 = normal
	//1 = random
	//2 = UNKEN
	uint32_t mu_ruler_bit_breaches : 2;

	//0 = off
	//1 = on
	//2 = UNKEN
	uint32_t mu_ruler_bit_astro_creeps : 2;

	//0 = difficulty_easier
	//1 = difficulty_easier_plus
	//2 = difficulty_hard
	//3 = difficulty_hard_plus
	//4 = difficulty_insane
	//5 = difficulty_random_all
	//6 = difficulty_random_each
	//7 = UNKEN
	uint32_t mu_ruler_difficulty : 3;

	//0 = power_failure_none
	//1 = power_failure_chance_8
	//2 = power_failure_chance_16
	//3 = power_failure_chance_32
	//4 = power_failure_time_30_60
	//5 = power_failure_time_60_120
	//6 = power_failure_time_120_240
	//7 = UNKEN
	uint32_t mu_ruler_power_failure : 3;
};
static_assert(sizeof(gen_options64_t) == sizeof(uint64_t), "gen_options64_t doesn't fit in 64 bits");
//this is persisted as a single uint64_t

struct gen_options32_t
{
	uint32_t im_planner_infestation : 2;	//0 = infestation_none, 1 = infestation_less, 2 = infestation_more, 3 = infestation_total
	uint32_t im_planner_chance_doors : 2;	//0 = chance_none, 1 = chance_more, 2 = chance_fewer
	uint32_t im_planner_chance_fences : 2;	//0 = chance_none, 1 = chance_more, 2 = chance_fewer
	uint32_t im_planner_bit_invert_barriers : 1;	//0 = normal, 1 = inverted
	uint32_t im_planner_rooms : 2;	//0 = rooms_empty, 1 = rooms_filled, 2 = rooms_pillars
	uint32_t im_planner_chance_sentries : 2;	//0 = chance_none, 1 = chance_more, 2 = chance_fewer
	uint32_t im_planner_bit_datacores : 1;	//0 = more, 1 = fewer
	uint32_t im_planner_bit_armories : 1;	//0 = more, 1 = fewer
	uint32_t im_planner_bit_repairs : 1;	//0 = more, 1 = fewer

	uint32_t im_ruler_bit_initial_power : 1;	//0 = starts off, 1 = starts on

	uint32_t mu_ruler_bit_breaches : 1;	//0 = normal, 1 = random
	uint32_t mu_ruler_bit_astro_creeps : 1;	//0 = off, 1 = on
	uint32_t mu_ruler_difficulty : 3;	//0 = difficulty_easier, 1 = difficulty_easier_plus, 2 = difficulty_hard, 3 = difficulty_hard_plus, 4 = difficulty_insane, 5 = difficulty_random_all, 6 = difficulty_random_each
	uint32_t mu_ruler_power_failure : 3;	//0 = power_failure_none, 1 = power_failure_chance_8, 2 = power_failure_chance_16, 3 = power_failure_chance_32, 4  = power_failure_time_30_60, 5 = power_failure_time_60_120, 6 = power_failure_time_120_240

	//new 2021-02-28
	uint32_t im_ruler_bit_reactor_safety_protocol : 1;	//0 = barriers don't open during reactor overload, 1 = barriers open during reactor overload

	//new 2021-02-28
	//2022-07-18: changed to be immutable
	uint32_t im_ruler_bit_beasts_lurk : 1;	//0 = beasts never lurk, 1 = beasts lurk in infestation
};
static_assert(sizeof(gen_options32_t) == sizeof(uint32_t), "gen_options32_t doesn't fit in 32 bits");

struct gen_motd_t
{
	union
	{
		struct
		{
			uint32_t day : 5;
			uint32_t month : 4;
			uint32_t year : 23;
		} date;
		uint32_t seed;
	};
};
static_assert(sizeof(uint32_t) == sizeof(gen_motd_t), "wtf!?");

//is a struct (as opposed to flattened) to unify some functions
struct gen_campaign_t	//cotw / grind
{
	uint32_t mission;
	uint32_t start_unixtime;
};

//128 bit xor-shift
struct prng_t
{
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t w;
};

struct tunneler_t
{
	int32_t x;
	int32_t z;
	int32_t direction;
	int32_t life;
};

struct plan_flooder_t
{
	uint8_t data[PLAN_CELLS_SIZE];

	//this queue is very special; we know the max possible size, we only add at the end and remove from the beginning
	uint32_t queue_memory[PLAN_CELLS_SIZE];
	uint32_t queue_first;
	uint32_t queue_next;
};

struct valid_lab_exit_t
{
	uint8_t write;
	bool eh;
};

struct xzi_t
{
	int32_t x;
	int32_t z;
};

struct runtime_object_constraints_t
{
	uint32_t bits;
	uint32_t cores;
	uint32_t doors;
	uint32_t fences;
	uint32_t breakers;
	uint32_t panics;
	uint32_t sentries;
};

struct year_week_t
{
	int32_t year;
	int32_t week;
};

struct year_month_day_t
{
	uint32_t year;
	uint32_t month;
	uint32_t day;
};

static uint32_t __jenkins_hash(const void* in_bytes, const size_t in_size, const uint32_t in_input_hash)
{
	uint32_t result = in_input_hash;

	const uint8_t* b = (uint8_t*)in_bytes;
	for (
		size_t i = 0;
		i < in_size;
		++i
		)
	{
		result += b[i];
		result += (result << 10);
		result ^= (result >> 6);
	}

	result += (result << 3);
	result ^= (result >> 11);
	result += (result << 15);

	return result;
}

static bool __is_leap_year(const int32_t year)
{
	return (0 == year % 4) && ((0 != year % 100) || (0 == year % 400));
}

static year_week_t __year_week(const uint32_t in_unixtime)
{
	year_week_t result;

	tm start_time;
	{
		const time_t TT_UNIXTIME = in_unixtime;
		::gmtime_s(&start_time, &TT_UNIXTIME);
	}

	result.year = start_time.tm_year + 1900;

	// Monday this week: may be negative down to 1-6 = -5;
	const int32_t MONDAY_THIS_WEEK = start_time.tm_yday - (start_time.tm_wday + 6) % 7;

	// First Monday of the year
	const int32_t FIRST_MONDAY_THIS_YEAR = 1 + (MONDAY_THIS_WEEK + 6) % 7;

	// Monday of week 1: should lie between -2 and 4 inclusive
	const int32_t MONDAY_FIRST_WEEK = (FIRST_MONDAY_THIS_YEAR > 4) ? FIRST_MONDAY_THIS_YEAR - 7 : FIRST_MONDAY_THIS_YEAR;

	// Nominal week ... but see below
	result.week = 1 + (MONDAY_THIS_WEEK - MONDAY_FIRST_WEEK) / 7;

	// In ISO-8601 there is no week 0 ... it will be week 52 or 53 of the previous year
	if (0 == result.week)
	{
		--result.year;
		result.week = 52;
		if (3 == FIRST_MONDAY_THIS_YEAR || 4 == FIRST_MONDAY_THIS_YEAR || (__is_leap_year(result.year) && 2 == FIRST_MONDAY_THIS_YEAR))
			result.week = 53;
	}

	// Similar issues at the end of the calendar year
	if (53 == result.week)
	{
		const int32_t days_in_year = __is_leap_year(result.year) ? 366 : 365;
		if (days_in_year - MONDAY_THIS_WEEK < 3)
		{
			++result.year;
			result.week = 1;
		}
	}

	return result;
}

static year_month_day_t __year_month_day(const uint32_t in_unixtime)
{
	year_month_day_t result;

	const time_t TT_UNIXTIME = in_unixtime;
	tm tm;
	::gmtime_s(&tm, &TT_UNIXTIME);
	assert(tm.tm_year > -1);
	result.year = tm.tm_year;
	assert(tm.tm_mon >= 0 && tm.tm_mon <= 11);
	result.month = tm.tm_mon;
	assert(tm.tm_mday >= 1 && tm.tm_mon <= 31);
	result.day = tm.tm_mday - 1;

	return result;
}

static prng_t __prng_make(const uint32_t in_seed)
{
	prng_t result;
	result.x = in_seed;
	result.y = 362436069;
	result.z = 521288629;
	result.w = 88675123;
	return result;
}

static uint32_t __prng_generate(prng_t& out_prng)
{
	const uint32_t T = out_prng.x ^ (out_prng.x << 11);
	out_prng.x = out_prng.y;
	out_prng.y = out_prng.z;
	out_prng.z = out_prng.w;
	return out_prng.w = out_prng.w ^ (out_prng.w >> 19) ^ T ^ (T >> 8);
}

static int32_t __prng_int32(
	const int32_t in_max_plus_one,
	prng_t& out_prng)
{
	assert(in_max_plus_one > 0);
	return __prng_generate(out_prng) % in_max_plus_one;
}

static int32_t __int_max(const int32_t& a, const int32_t& b)
{
	if (a > b)
		return a;
	return b;
}

static uint32_t __uint_min(const uint32_t& a, const uint32_t& b)
{
	if (a < b)
		return a;
	return b;
}

static uint8_t __plan_read_cell(const gen_plan_t& in_plan, const uint32_t in_offset)
{
	assert(in_offset < PLAN_CELLS_SIZE);
	return in_plan.cells[in_offset];
}

static void __plan_write_cell(
	const uint32_t in_offset, const uint8_t in_value,
	gen_plan_t& out_plan)
{
	assert(in_offset < PLAN_CELLS_SIZE);
	out_plan.cells[in_offset] = in_value;
}

static uint8_t __plan_floor_around_floor_flags_256(const gen_plan_t& in_plan, const uint32_t in_offset)
{
	if (in_offset >= PLAN_CELLS_SIZE)
		return 0;

	//the offset we are relative must be "floor"
	if (0 == (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, in_offset)))
		return 0;

	//we are checking for everything that has the floor bit
	uint8_t flags = 0;

	if (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, -1, -1)))
		flags |= (1 << 0);

	if (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, 0, -1)))
		flags |= DIRECTION_FLAGS_ZNEG;

	if (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, 1, -1)))
		flags |= (1 << 2);

	if (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, -1, 0)))
		flags |= DIRECTION_FLAGS_XNEG;

	if (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, 1, 0)))
		flags |= DIRECTION_FLAGS_XPOS;

	if (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, -1, 1)))
		flags |= (1 << 5);

	if (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, 0, 1)))
		flags |= DIRECTION_FLAGS_ZPOS;

	if (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, 1, 1)))
		flags |= (1 << 7);

	return flags;
}

static uint8_t __plan_wall_around_floor_flags_256(const gen_plan_t& in_plan, const uint32_t in_offset)
{
	if (in_offset >= PLAN_CELLS_SIZE)
		return 0;

	//the offset we are relative must be "floor"
	if (0 == (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, in_offset)))
		return 0;

	//we are checking explicitly for the c_wall value, not for "not floor"
	uint8_t flags = 0;

	if (0 == (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, -1, -1))))
		flags |= (1 << 0);

	if (0 == (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, 0, -1))))
		flags |= DIRECTION_FLAGS_ZNEG;

	if (0 == (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, 1, -1))))
		flags |= (1 << 2);

	if (0 == (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, -1, 0))))
		flags |= DIRECTION_FLAGS_XNEG;

	if (0 == (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, 1, 0))))
		flags |= DIRECTION_FLAGS_XPOS;

	if (0 == (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, -1, 1))))
		flags |= (1 << 5);

	if (0 == (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, 0, 1))))
		flags |= DIRECTION_FLAGS_ZPOS;

	if (0 == (PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, 1, 1))))
		flags |= (1 << 7);

	return flags;
}

//these were arrived at over time and quite manually if I recall... :(
static uint8_t __plan_flags_256_entity(const gen_plan_t& in_plan, const uint32_t in_offset)
{
	constexpr uint8_t M_FLAGS256[256] =
	{
		0,0,2,2,0,0,2,2,2,2,3,0,2,2,0,0,2,2,3,0,2,2,0,0,1,1,3,0,1,1,0,0,0,0,2,2,0,0,0,2,2,2,3,3,0,2,0,0,2,0,3,0,2,0,0,0,1,1,0,0,1,1,0,0,2,2,0,0,2,0,0,0,3,0,3,0,3,0,3,0,3,3,3,0,3,0,0,0,3,0,0,0,3,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,2,2,0,0,2,0,2,0,3,0,0,0,0,0,2,0,3,0,2,2,3,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,2,2,2,0,0,0,2,0,0,2,0,0,0,2,2,0,0,1,1,0,0,1,1,0,0,2,2,0,0,2,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,2,2,0,0,2,2,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	};
	return M_FLAGS256[__plan_floor_around_floor_flags_256(in_plan, in_offset)];
}

//these were arrived at over time and quite manually if I recall... :(
static bool __plan_valid_pre_barrier_position_eh(const gen_plan_t& in_plan, const uint32_t in_offset)
{
	switch (__plan_floor_around_floor_flags_256(in_plan, in_offset))
	{
	default:
		return false;

	case 25:
	case 28:
	case 29:
	case 56:
	case 57:
	case 60:
	case 61:
	case 67:
	case 70:
	case 71:
	case 98:
	case 99:
	case 102:
	case 103:
	case 152:
	case 153:
	case 156:
	case 157:
	case 184:
	case 185:
	case 188:
	case 189:
	case 194:
	case 195:
	case 198:
	case 199:
	case 226:
	case 227:
	case 231:
	case 230:
		return true;
	}
}

static uint32_t __plan_find_airlock_door(const gen_plan_t& in_plan)
{
	{
		const uint32_t OFFSET = PLAN_OFFSET_DELTA(in_plan.airlock, 0, 1);
		if (PLAN_CELL_DOOR_B == __plan_read_cell(in_plan, OFFSET))
			return OFFSET;
	}

	{
		const uint32_t OFFSET = PLAN_OFFSET_DELTA(in_plan.airlock, 0, -1);
		if (PLAN_CELL_DOOR_B == __plan_read_cell(in_plan, OFFSET))
			return OFFSET;
	}

	{
		const uint32_t OFFSET = PLAN_OFFSET_DELTA(in_plan.airlock, 1, 0);
		if (PLAN_CELL_DOOR_B == __plan_read_cell(in_plan, OFFSET))
			return OFFSET;
	}

	{
		const uint32_t OFFSET = PLAN_OFFSET_DELTA(in_plan.airlock, -1, 0);
		if (PLAN_CELL_DOOR_B == __plan_read_cell(in_plan, OFFSET))
			return OFFSET;
	}

	return UINT32_MAX;
}

static bool __plan_valid_normal_breach_position_eh(const gen_plan_t& in_plan, const uint32_t in_offset)
{
	if (in_plan.airlock == in_offset)
		return false;

	if (PLAN_CELL_PANIC == __plan_read_cell(in_plan, in_offset))
		return false;

	return F256E_BREACH_OK == __plan_flags_256_entity(in_plan, in_offset);
}

static uint32_t __plan_count_valid_normal_breach_positions(const gen_plan_t& in_plan)
{
	uint32_t count = 0;

	for (
		uint32_t offset = 0;
		offset < PLAN_CELLS_SIZE;
		++offset
		)
	{
		if (__plan_valid_normal_breach_position_eh(in_plan, offset))
			++count;
	}

	return count;
}

static bool __cardinal_collision_eh(const uint32_t in_offset, const uint8_t in_a, const uint8_t in_b, const gen_plan_t& in_plan)
{
	if (in_a != __plan_read_cell(in_plan, in_offset))
		return false;

	if (in_b == __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, -1, 0)))
		return true;

	if (in_b == __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, 1, 0)))
		return true;

	if (in_b == __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, 0, -1)))
		return true;

	if (in_b == __plan_read_cell(in_plan, PLAN_OFFSET_DELTA(in_offset, 0, 1)))
		return true;

	return false;
}

static bool __planner_cardinal_collision_eh(const uint32_t in_offset, const gen_plan_t& in_plan)
{
	return
		__cardinal_collision_eh(in_offset, PLAN_CELL_PANIC, PLAN_CELL_DOOR_A, in_plan) ||
		__cardinal_collision_eh(in_offset, PLAN_CELL_PANIC, PLAN_CELL_DOOR_B, in_plan) ||
		__cardinal_collision_eh(in_offset, PLAN_CELL_PANIC, PLAN_CELL_FENCE, in_plan) ||
		__cardinal_collision_eh(in_offset, PLAN_CELL_PANIC, PLAN_CELL_PANIC, in_plan) ||
		__cardinal_collision_eh(in_offset, PLAN_CELL_DOOR_B, PLAN_CELL_DOOR_B, in_plan);	//lab doors colliding
}

static uint32_t __strict_tile_to_offset(const xzi_t& in_tile)
{
	assert(in_tile.x >= 0 && in_tile.x < PLAN_CELLS_ASPECT);
	assert(in_tile.z >= 0 && in_tile.z < PLAN_CELLS_ASPECT);

	const int32_t SIGNED_OFFSET = in_tile.x + in_tile.z * PLAN_CELLS_ASPECT;
	assert(SIGNED_OFFSET >= 0 && SIGNED_OFFSET < PLAN_CELLS_SIZE);

	return (uint32_t)SIGNED_OFFSET;
}

static tunneler_t __tunneler_make(
	const uint32_t in_life,
	prng_t& out_prng)
{
	tunneler_t result;

	result.x = PLAN_CELLS_ASPECT / 2;
	result.z = PLAN_CELLS_ASPECT / 2;
	result.direction = __prng_int32(4, out_prng);
	result.life = in_life;

	return result;
}

static void __tunneler_update(prng_t& out_prng, gen_plan_t& out_plan, tunneler_t& out_tunneler)
{
	if (out_tunneler.x < 1)
	{
		out_tunneler.x = 1;
		out_tunneler.direction = __prng_int32(4, out_prng);
	}
	else if (out_tunneler.x > PLAN_CELLS_ASPECT - 2)
	{
		out_tunneler.x = PLAN_CELLS_ASPECT - 2;
		out_tunneler.direction = __prng_int32(4, out_prng);
	}

	if (out_tunneler.z < 1)
	{
		out_tunneler.z = 1;
		out_tunneler.direction = __prng_int32(4, out_prng);
	}
	else if (out_tunneler.z > PLAN_CELLS_ASPECT - 2)
	{
		out_tunneler.z = PLAN_CELLS_ASPECT - 2;
		out_tunneler.direction = __prng_int32(4, out_prng);
	}

	__plan_write_cell(PLAN_OFFSET_XZ(out_tunneler.x, out_tunneler.z), PLAN_CELL_FLOOR, out_plan);

	switch (out_tunneler.direction)
	{
	case 0:
		--out_tunneler.x;
		break;

	case 1:
		++out_tunneler.x;
		break;

	case 2:
		--out_tunneler.z;
		break;

	case 3:
		++out_tunneler.z;
		break;
	}

	if (0 == __prng_int32(4, out_prng))
		out_tunneler.direction = __prng_int32(4, out_prng);

	--out_tunneler.life;
}

#if FINALIZE

static void __flooder_add(
	const uint32_t in_offset,
	plan_flooder_t& out_flooder)
{
	if (
		in_offset < PLAN_CELLS_SIZE &&
		PLAN_FLOODER_UNREACHABLE == out_flooder.data[in_offset]
		)
	{
		for (
			uint32_t i = out_flooder.queue_first;
			i < out_flooder.queue_next;
			++i
			)
		{
			if (out_flooder.queue_memory[i] == in_offset)
				return;
		}

		assert(out_flooder.queue_next < _countof(out_flooder.queue_memory));
		out_flooder.queue_memory[out_flooder.queue_next] = in_offset;
		++out_flooder.queue_next;
	}
}

#if PRE_BARRIERS
static void __pre_barriers(
	const int32_t in_chance, const uint8_t in_value,
	gen_plan_t& out_plan, prng_t& out_prng)
{
	assert(in_chance);

	for (
		int32_t x = 0;
		x < PLAN_CELLS_ASPECT;
		++x
		)
	{
		for (
			int32_t z = 0;
			z < PLAN_CELLS_ASPECT;
			++z
			)
		{
			const uint32_t OFFSET = __strict_tile_to_offset({ x, z });
			if (__plan_valid_pre_barrier_position_eh(out_plan, OFFSET))
			{
				//this is to support Rescue mode, which also writes stuff (heavy doors, labs, fences, etc) where doors would be in the base pass,
				switch (__plan_read_cell(out_plan, OFFSET))
				{
					//these can be overwritten
				default:
					if (0 == __prng_int32(in_chance, out_prng))
						__plan_write_cell(OFFSET, in_value, out_plan);
					break;

					//these should be kept
				case PLAN_CELL_DOOR_B:
				case PLAN_CELL_LAB:
					break;
				}
			}
		}
	}
}

static void __pre_barriers(
	const gen_options32_t& in_options,
	gen_plan_t& out_plan, prng_t& out_prng)
{
	if (
		in_options.im_planner_chance_doors &&
		!in_options.im_planner_bit_invert_barriers
		)
	{
		__pre_barriers(in_options.im_planner_chance_doors, PLAN_CELL_DOOR_A, out_plan, out_prng);
	}
	else if (
		in_options.im_planner_chance_fences &&
		in_options.im_planner_bit_invert_barriers
		)
	{
		__pre_barriers(in_options.im_planner_chance_fences, PLAN_CELL_FENCE, out_plan, out_prng);
	}
}
#endif

static void __sentries(
	const int32_t chance, gen_plan_t& plan,
	prng_t& prng)
{
	for (
		int32_t x = 0;
		x < PLAN_CELLS_ASPECT;
		++x
		)
	{
		for (
			int32_t z = 0;
			z < PLAN_CELLS_ASPECT;
			++z
			)
		{
			const uint32_t OFFSET = __strict_tile_to_offset({ x, z });
			if (
				PLAN_CELL_FLOOR == __plan_read_cell(plan, OFFSET) &&
				F256E_SENTRY_OK == __plan_flags_256_entity(plan, OFFSET) &&
				0 == __prng_int32(chance, prng)
				)
			{
				__plan_write_cell(OFFSET, PLAN_CELL_SENTRY, plan);
			}
		}
	}
}

static void __wall_goodies_no_nooks(
	const uint8_t in_value, const int32_t in_chance,
	gen_plan_t& out_plan, prng_t& out_prng)
{
	for (
		int32_t x = 0;
		x < PLAN_CELLS_ASPECT;
		++x
		)
	{
		for (
			int32_t z = 0;
			z < PLAN_CELLS_ASPECT;
			++z
			)
		{
			const uint32_t OFFSET = __strict_tile_to_offset({ x, z });
			if (PLAN_CELL_FLOOR == __plan_read_cell(out_plan, OFFSET))
			{
				if (0 == __prng_int32(in_chance, out_prng))
				{
					//on any wall
					if (__plan_wall_around_floor_flags_256(out_plan, OFFSET) & (DIRECTION_FLAGS_XNEG | DIRECTION_FLAGS_XPOS | DIRECTION_FLAGS_ZNEG | DIRECTION_FLAGS_ZPOS))
					{
						//no nooks EVER
						if (F256E_BREACH_OK != __plan_flags_256_entity(out_plan, OFFSET))
							__plan_write_cell(OFFSET, in_value, out_plan);
					}
				}
			}
		}
	}
}

#if POST_BARRIERS
static void __post_barriers(
	const uint8_t in_value, const int32_t in_sparse_barriers,
	gen_plan_t& out_plan)
{
	const uint8_t X_CORRIDOR = DIRECTION_FLAGS_XNEG | DIRECTION_FLAGS_XPOS;
	const uint8_t Z_CORRIDOR = DIRECTION_FLAGS_ZNEG | DIRECTION_FLAGS_ZPOS;

	for (
		uint32_t offset = 0;
		offset < PLAN_CELLS_SIZE;
		++offset
		)
	{
		if (((0 == in_sparse_barriers && (offset % 2)) || (0 != in_sparse_barriers && 0 == (offset % 5))) &&
			PLAN_CELL_FLOOR == __plan_read_cell(out_plan, offset) &&
			F256E_BREACH_OK != __plan_flags_256_entity(out_plan, offset))	//no smatts
		{
			const uint8_t flags = __plan_wall_around_floor_flags_256(out_plan, offset);
			if (
				X_CORRIDOR == (flags & X_CORRIDOR) ||
				Z_CORRIDOR == (flags & Z_CORRIDOR)
				)
			{
				__plan_write_cell(offset, in_value, out_plan);
			}
		}
	}
}

static void __post_barriers(
	const gen_options32_t& in_options,
	gen_plan_t& out_plan)
{
	if (
		in_options.im_planner_chance_doors &&
		in_options.im_planner_bit_invert_barriers
		)
	{
		__post_barriers(PLAN_CELL_DOOR_A, 1 < in_options.im_planner_chance_doors, out_plan);
	}
	else if (
		in_options.im_planner_chance_fences &&
		!in_options.im_planner_bit_invert_barriers
		)
	{
		__post_barriers(PLAN_CELL_FENCE, 1 < in_options.im_planner_chance_fences, out_plan);
	}
}
#endif

static int32_t __manhattan_distance(const xzi_t& in_a, const xzi_t& in_b)
{
	return ::abs(in_a.x - in_b.x) + ::abs(in_a.z - in_b.z);
}

static bool __make_nook_airlock(gen_plan_t& out_plan)
{
	int32_t largest_manhattan_distance = 0;
	xzi_t airlock_tile{};
	xzi_t door_tile{};

	//find the nook furthest from the reactor
	{
		const xzi_t REACTOR{ PLAN_REACTOR_POSITION_AXIS, PLAN_REACTOR_POSITION_AXIS };
		for (
			int32_t x = 0;
			x < PLAN_CELLS_ASPECT;
			++x
			)
		{
			for (
				int32_t z = 0;
				z < PLAN_CELLS_ASPECT;
				++z
				)
			{
				const uint32_t OFFSET = __strict_tile_to_offset({ x, z });
				switch (__plan_floor_around_floor_flags_256(out_plan, OFFSET))
				{
				case DIRECTION_FLAGS_ZNEG:
				{
					const int32_t MANHATTAN_DISTANCE = __manhattan_distance({ x, z }, REACTOR);
					if (MANHATTAN_DISTANCE > largest_manhattan_distance)
					{
						largest_manhattan_distance = MANHATTAN_DISTANCE;
						airlock_tile = { x, z };
						door_tile = { airlock_tile.x, airlock_tile.z - 1 };
					}
				}
				break;

				case DIRECTION_FLAGS_XNEG:
				{
					const int32_t MANHATTAN_DISTANCE = __manhattan_distance({ x, z }, REACTOR);
					if (MANHATTAN_DISTANCE > largest_manhattan_distance)
					{
						largest_manhattan_distance = MANHATTAN_DISTANCE;
						airlock_tile = { x, z };
						door_tile = { airlock_tile.x - 1, airlock_tile.z };
					}
				}
				break;

				case DIRECTION_FLAGS_XPOS:
				{
					const int32_t MANHATTAN_DISTANCE = __manhattan_distance({ x, z }, REACTOR);
					if (MANHATTAN_DISTANCE > largest_manhattan_distance)
					{
						largest_manhattan_distance = MANHATTAN_DISTANCE;
						airlock_tile = { x, z };
						door_tile = { airlock_tile.x + 1, airlock_tile.z };
					}
				}
				break;

				case DIRECTION_FLAGS_ZPOS:
				{
					const int32_t MANHATTAN_DISTANCE = __manhattan_distance({ x, z }, REACTOR);
					if (MANHATTAN_DISTANCE > largest_manhattan_distance)
					{
						largest_manhattan_distance = MANHATTAN_DISTANCE;
						airlock_tile = { x, z };
						door_tile = { airlock_tile.x, airlock_tile.z + 1 };
					}
				}
				break;
				}
			}
		}
	}

	//failed to find a good nook
	if (0 == largest_manhattan_distance)
		return false;

	//breakers can be generated in the airlock, so wipe the cell if that is the case
	//FIXME: always wipe it?
	out_plan.airlock = __strict_tile_to_offset(airlock_tile);
	if (PLAN_CELL_FLOOR != __plan_read_cell(out_plan, out_plan.airlock))
		__plan_write_cell(out_plan.airlock, PLAN_CELL_FLOOR, out_plan);

	//place the airlock door
	__plan_write_cell(__strict_tile_to_offset(door_tile), PLAN_CELL_DOOR_B, out_plan);

	//since we are storing it outside now (m_immutable_t) we need to make sure it is valid (inside out_plan)
	return __plan_find_airlock_door(out_plan) < PLAN_CELLS_SIZE;
}

static bool __breaker_ok(const uint32_t in_offset, const gen_plan_t& in_plan)
{
	return
		0 == (in_offset % 10) &&
		0 != (__plan_wall_around_floor_flags_256(in_plan, in_offset) & (DIRECTION_FLAGS_XNEG | DIRECTION_FLAGS_XPOS | DIRECTION_FLAGS_ZNEG | DIRECTION_FLAGS_ZPOS));
}

static void __breakers(gen_plan_t& out_plan)
{
	for (
		uint32_t offset = 0;
		offset < PLAN_CELLS_SIZE;
		++offset
		)
	{
		//anywhere we can attach it to a wall; this is like wallGoodies...
		//we allow smatts!
		switch (__plan_read_cell(out_plan, offset))
		{
		case PLAN_CELL_FLOOR:
		case PLAN_CELL_LAB:
			if (__breaker_ok(offset, out_plan))
				__plan_write_cell(offset, PLAN_CELL_BREAKER, out_plan);
			break;

			//case c_cell_lab:
			//	if (__breaker_ok(offset, plan))
			//		write_cell(offset, c_cell_lab_breaker, plan);
			//	break;
		}
	}
}

static void __fill_tech(
	const gen_options32_t& in_options,
	gen_plan_t& out_plan)
{
	if (ROOMS_EMPTY == in_options.im_planner_rooms)
		return;

	{
		//we need to read from a copy since we are writing to the map, this would mess up the reads
		const gen_plan_t PLAN_COPY = out_plan;

		switch (in_options.im_planner_rooms)
		{
			//fill
		default:
			for (
				uint32_t offset = 0;
				offset < PLAN_CELLS_SIZE;
				++offset
				)
			{
				//0 is an invalid offset for the security console / disabled
				//not around lab floor
				//this is the definition of tech: floor surrounded by floor in all 8 neighbouring cells
				if (
					(PLAN_COPY.security_console && PLAN_COPY.security_console != offset) &&

					PLAN_CELL_FLOOR == __plan_read_cell(PLAN_COPY, offset) &&

					255 == __plan_floor_around_floor_flags_256(PLAN_COPY, offset)
					)
				{
					__plan_write_cell(offset, PLAN_CELL_TECH, out_plan);
				}
			}
			break;

			//pillars
		case ROOMS_PILLARS:
			for (
				uint32_t offset = 0;
				offset < PLAN_CELLS_SIZE;
				++offset
				)
			{
				//0 is an invalid offset for the security console / disabled
				//not around lab floor
				//this is the definition of tech: floor surrounded by floor in all 8 neighbouring cells
				if (
					(PLAN_COPY.security_console && PLAN_COPY.security_console != offset) &&

					(PLAN_OFFSET_TILE_X(offset) % 2) &&
					(PLAN_OFFSET_TILE_Z(offset) % 2) &&

					PLAN_CELL_FLOOR == __plan_read_cell(PLAN_COPY, offset) &&

					255 == __plan_floor_around_floor_flags_256(PLAN_COPY, offset)
					)
				{
					__plan_write_cell(offset, PLAN_CELL_TECH, out_plan);
				}
			}
			break;
		}
	}

	//explicit remove tech inside the reactor room
	{
		const int32_t ORIGIN = PLAN_REACTOR_POSITION_AXIS - REACTOR_ROOM_ASPECT / 2;

		for (
			int32_t x = 0;
			x < REACTOR_ROOM_ASPECT;
			++x
			)
		{
			for (
				int32_t z = 0;
				z < REACTOR_ROOM_ASPECT;
				++z
				)
			{
				const uint32_t OFFSET = PLAN_OFFSET_XZ(ORIGIN + x, ORIGIN + z);
				if (PLAN_CELL_TECH == __plan_read_cell(out_plan, OFFSET))
					__plan_write_cell(OFFSET, PLAN_CELL_FLOOR, out_plan);
			}
		}
	}
}

static runtime_object_constraints_t __planner_check_runtime_object_constraints(const gen_plan_t& in_plan)
{
	runtime_object_constraints_t result{};

	for (
		uint32_t offset = 0;
		offset < PLAN_CELLS_SIZE;
		++offset
		)
	{
		switch (in_plan.cells[offset])
		{
		case PLAN_CELL_DATA:
		case PLAN_CELL_COOLANT:
		case PLAN_CELL_REPAIR:
		case PLAN_CELL_ARMORY:
			++result.cores;
			break;

		case PLAN_CELL_SENTRY:
			++result.cores;
			++result.sentries;
			break;

		case PLAN_CELL_DOOR_A:
		case PLAN_CELL_DOOR_B:
			++result.doors;
			break;

		case PLAN_CELL_FENCE:
			++result.fences;
			break;

		case PLAN_CELL_BREAKER:
			++result.breakers;
			break;

		case PLAN_CELL_PANIC:
			++result.panics;
		}
	}

	if (PLAN_MAX_CORES < result.cores)
		result.bits |= ROC_TOO_MANY_CORES;

	if (PLAN_MAX_DOORS < result.doors)
		result.bits |= ROC_TOO_MANY_DOORS;

	if (PLAN_MAX_FENCES < result.fences)
		result.bits |= ROC_TOO_MANY_FENCES;

	if (0 == result.breakers)
		result.bits |= ROC_AT_LEAST_ONE_BREAKER;
	else if (PLAN_MAX_BREAKERS < result.breakers)
		result.bits |= ROC_TOO_MANY_BREAKERS;

	if (PLAN_MAX_PANICS < result.panics)
		result.bits |= ROC_TOO_MANY_PANICS;

	if (PLAN_MAX_SENTRIES < result.sentries)
		result.bits |= ROC_TOO_MANY_SENTRIES;

	return result;
}

static bool __prepare_for_game_rules(
	const gen_options32_t& in_options,
	gen_plan_t& out_plan, prng_t& out_prng)
{
#if PRE_BARRIERS
	__pre_barriers(in_options, out_plan, out_prng);
#endif

	__wall_goodies_no_nooks(PLAN_CELL_DATA, in_options.im_planner_bit_datacores ? 20 : 10, out_plan, out_prng);

	__wall_goodies_no_nooks(PLAN_CELL_ARMORY, in_options.im_planner_bit_armories ? 20 : 10, out_plan, out_prng);

	if (in_options.im_planner_chance_sentries)
		__sentries(in_options.im_planner_chance_sentries, out_plan, out_prng);

	__wall_goodies_no_nooks(PLAN_CELL_REPAIR, in_options.im_planner_bit_repairs ? 20 : 10, out_plan, out_prng);

#if POST_BARRIERS
	__post_barriers(in_options, out_plan);
#endif

	__breakers(out_plan);	//might as well always have breakers due to potentially mutable rules

	//if any of the below fail we just loop to generate a new map...

	//note that a nook airlock position is also a valid breach,
	//so to ensure at least 1 active breach we check for 2 (placing the airlock later)
	assert(0 == out_plan.airlock);
	if (2 > __plan_count_valid_normal_breach_positions(out_plan))
		return false;

	//this will take the place of a valid breach...
	//this might overwrite the last breaker...
	if (!__make_nook_airlock(out_plan))
		return false;

	if (0 != __planner_check_runtime_object_constraints(out_plan).bits)
		return false;

	__fill_tech(in_options, out_plan);

	//all ok!
	return true;
}

static bool __planner_everything_reachable_from_airlock(
	const gen_plan_t& in_plan,
	plan_flooder_t& out_flooder)
{
	out_flooder = {};

	//mark all floorbits (and security console) as unreachable
	for (
		uint32_t offset = 0;
		offset < PLAN_CELLS_SIZE;
		++offset
		)
	{
		if (
			in_plan.security_console != offset &&
			(PLAN_CELL_FLOORBIT & __plan_read_cell(in_plan, offset))
			)
			out_flooder.data[offset] = PLAN_FLOODER_UNREACHABLE;
		else
			out_flooder.data[offset] = PLAN_FLOODER_SOLID;
	}

	//add the airlock as the starting point if it is unreachable (i.e. floorbit)
	assert(in_plan.airlock < PLAN_CELLS_SIZE);
	if (PLAN_FLOODER_UNREACHABLE != out_flooder.data[in_plan.airlock])
	{
		//write everything as unreachable
		for (
			uint32_t offset = 0;
			offset < PLAN_CELLS_SIZE;
			++offset
			)
			out_flooder.data[offset] = PLAN_FLOODER_UNREACHABLE;

		return false;
	}
	__flooder_add(in_plan.airlock, out_flooder);

	//flood
	while (out_flooder.queue_first < out_flooder.queue_next)
	{
		const uint32_t OFFSET = out_flooder.queue_memory[out_flooder.queue_first];
		out_flooder.data[OFFSET] = PLAN_FLOODER_REACHABLE;
		++out_flooder.queue_first;

		__flooder_add(OFFSET - 1, out_flooder);
		__flooder_add(OFFSET + 1, out_flooder);
		__flooder_add(OFFSET - PLAN_CELLS_ASPECT, out_flooder);
		__flooder_add(OFFSET + PLAN_CELLS_ASPECT, out_flooder);
	}

	//nothing may be unreachable now
	for (
		uint32_t offset = 0;
		offset < PLAN_CELLS_SIZE;
		++offset
		)
	{
		if (PLAN_FLOODER_UNREACHABLE == out_flooder.data[offset])
			return false;
	}

	return true;
}

static bool __post_generation_tests(const gen_plan_t& in_plan)
{
	//check for misc cardinal collisions
	for (
		uint32_t offset = 0;
		offset < PLAN_CELLS_SIZE;
		++offset
		)
	{
		if (__planner_cardinal_collision_eh(offset, in_plan))
			return false;
	}

	plan_flooder_t flooder;
	return __planner_everything_reachable_from_airlock(in_plan, flooder);
}
#endif//FINALIZE

#if MAKE_LABS
static bool __space_for_lab_eh(
	const int32_t in_ox, const int32_t in_oz,
	const gen_plan_t& out_plan)
{
	for (
		int32_t z = 0;
		z < LAB_SPACE_ASPECT;
		++z
		)
	{
		for (
			int32_t x = 0;
			x < LAB_SPACE_ASPECT;
			++x
			)
		{
			//must all be solid walls so we can carve out a lab without disturbing anything
			if (PLAN_CELL_WALL != __plan_read_cell(out_plan, PLAN_OFFSET_XZ(in_ox + x, in_oz + z)))
				return false;
		}
	}

	return true;
}

static valid_lab_exit_t __valid_lab_exit(
	const int32_t in_x, const int32_t in_z, const gen_plan_t& in_plan, const gen_options32_t& in_options,
	prng_t& out_prng)
{
	//read the value where we want an exit
	const uint8_t VALUE = __plan_read_cell(in_plan, PLAN_OFFSET_XZ(in_x, in_z));

	//if it doesn't have floorbit, not valid
	if (0 == (PLAN_CELL_FLOORBIT & VALUE))
		return {};

	//if it's a panic, not valid
	if (PLAN_CELL_PANIC == VALUE)
		return {};

	//if it's lab, stay with lab
	if (PLAN_CELL_LAB == VALUE)
		return { PLAN_CELL_LAB, 1 };

	//write the door/fence if they are enabled and if we make the roll
	if (
		0 < in_options.im_planner_chance_doors &&
		0 == __prng_int32(in_options.im_planner_chance_doors, out_prng)
		)
	{
		if (in_options.im_planner_bit_invert_barriers)
			return { PLAN_CELL_FENCE, 1 };

		return { PLAN_CELL_DOOR_B, 1 };
	}

	//otherwise write floor
	return { PLAN_CELL_FLOOR, 1 };
}

static bool __create_lab_exits_and_panic_rooms(
	const int32_t in_ox, const int32_t in_oz, const gen_options32_t& in_options,
	gen_plan_t& out_plan, prng_t& out_prng)
{
	uint32_t directions[] = { 0, 1, 2, 3 };
	uint32_t directions_remaining = 4;
	uint32_t exits = 0;
	valid_lab_exit_t valid_lab_exit;

	while (
		0 < directions_remaining &&
		2 > exits
		)
	{
		const uint32_t SELECT = __prng_int32(directions_remaining, out_prng);
		switch (directions[SELECT])
		{
		default:
			assert(0);
			break;

		case 0:
			//left
			if ((valid_lab_exit = __valid_lab_exit(in_ox - 1, in_oz + 2, out_plan, in_options, out_prng)).eh)
			{
				__plan_write_cell(PLAN_OFFSET_XZ(in_ox, in_oz + 2), valid_lab_exit.write, out_plan);
				++exits;
			}
			else if ((valid_lab_exit = __valid_lab_exit(in_ox - 1, in_oz + 1, out_plan, in_options, out_prng)).eh)
			{
				__plan_write_cell(PLAN_OFFSET_XZ(in_ox, in_oz + 1), valid_lab_exit.write, out_plan);
				++exits;
			}
			else if ((valid_lab_exit = __valid_lab_exit(in_ox - 1, in_oz + 3, out_plan, in_options, out_prng)).eh)
			{
				__plan_write_cell(PLAN_OFFSET_XZ(in_ox, in_oz + 3), valid_lab_exit.write, out_plan);
				++exits;
			}
			else if (0 == __prng_int32(LAB_PANIC_CHANCE, out_prng))
			{
				__plan_write_cell(PLAN_OFFSET_XZ(in_ox, in_oz + 2), PLAN_CELL_PANIC, out_plan);
			}
			break;

		case 1:
			//right
			if ((valid_lab_exit = __valid_lab_exit(in_ox + 5, in_oz + 2, out_plan, in_options, out_prng)).eh)
			{
				__plan_write_cell(PLAN_OFFSET_XZ(in_ox + 4, in_oz + 2), valid_lab_exit.write, out_plan);
				++exits;
			}
			else if ((valid_lab_exit = __valid_lab_exit(in_ox + 5, in_oz + 1, out_plan, in_options, out_prng)).eh)
			{
				__plan_write_cell(PLAN_OFFSET_XZ(in_ox + 4, in_oz + 1), valid_lab_exit.write, out_plan);
				++exits;
			}
			else if ((valid_lab_exit = __valid_lab_exit(in_ox + 5, in_oz + 3, out_plan, in_options, out_prng)).eh)
			{
				__plan_write_cell(PLAN_OFFSET_XZ(in_ox + 4, in_oz + 3), valid_lab_exit.write, out_plan);
				++exits;
			}
			else if (0 == __prng_int32(LAB_PANIC_CHANCE, out_prng))
			{
				__plan_write_cell(PLAN_OFFSET_XZ(in_ox + 4, in_oz + 2), PLAN_CELL_PANIC, out_plan);
			}
			break;

		case 2:
			//down
			if ((valid_lab_exit = __valid_lab_exit(in_ox + 2, in_oz - 1, out_plan, in_options, out_prng)).eh)
			{
				__plan_write_cell(PLAN_OFFSET_XZ(in_ox + 2, in_oz), valid_lab_exit.write, out_plan);
				++exits;
			}
			else if ((valid_lab_exit = __valid_lab_exit(in_ox + 1, in_oz - 1, out_plan, in_options, out_prng)).eh)
			{
				__plan_write_cell(PLAN_OFFSET_XZ(in_ox + 1, in_oz), valid_lab_exit.write, out_plan);
				++exits;
			}
			else if ((valid_lab_exit = __valid_lab_exit(in_ox + 3, in_oz - 1, out_plan, in_options, out_prng)).eh)
			{
				__plan_write_cell(PLAN_OFFSET_XZ(in_ox + 3, in_oz), valid_lab_exit.write, out_plan);
				++exits;
			}
			else if (0 == __prng_int32(LAB_PANIC_CHANCE, out_prng))
			{
				__plan_write_cell(PLAN_OFFSET_XZ(in_ox + 2, in_oz), PLAN_CELL_PANIC, out_plan);
			}
			break;

		case 3:
			//up
			if ((valid_lab_exit = __valid_lab_exit(in_ox + 2, in_oz + 5, out_plan, in_options, out_prng)).eh)
			{
				__plan_write_cell(PLAN_OFFSET_XZ(in_ox + 2, in_oz + 4), valid_lab_exit.write, out_plan);
				++exits;
			}
			else if ((valid_lab_exit = __valid_lab_exit(in_ox + 1, in_oz + 5, out_plan, in_options, out_prng)).eh)
			{
				__plan_write_cell(PLAN_OFFSET_XZ(in_ox + 1, in_oz + 4), valid_lab_exit.write, out_plan);
				++exits;
			}
			else if ((valid_lab_exit = __valid_lab_exit(in_ox + 3, in_oz + 5, out_plan, in_options, out_prng)).eh)
			{
				__plan_write_cell(PLAN_OFFSET_XZ(in_ox + 3, in_oz + 4), valid_lab_exit.write, out_plan);
				++exits;
			}
			else if (0 == __prng_int32(LAB_PANIC_CHANCE, out_prng))
			{
				__plan_write_cell(PLAN_OFFSET_XZ(in_ox + 2, in_oz + 4), PLAN_CELL_PANIC, out_plan);
			}
			break;
		}

		//copy last to selection
		directions[SELECT] = directions[directions_remaining - 1];

		//debug; clear the last
		directions[directions_remaining - 1] = UINT32_MAX;

		//one less to try
		--directions_remaining;
	}

	return 0 < exits;
}

static void __make_labs(
	const uint32_t in_campaign_mode, const gen_options32_t& in_options, const gen_campaign_t& in_cotw, const gen_campaign_t& in_grind,
	gen_plan_t& out_plan, prng_t& out_prng)
{
	uint32_t labs_created_successfully = 0;
	uint32_t labs_could_not_create_exits = 0;
	uint32_t labs_did_not_fit = 0;

	//since we are making panic rooms we potentially have stuff sticking out;
	//it is bad to have floors that go right to the edge of the map
	//const uint32_t lab_edge = 2;//legacy, but that generated long chains of labs which don't have much gameplay

	//used this for a while, was good with tunneler with 400 life
	//const uint32_t lab_edge = 7;

	//thinking that we need to ramp up with the missions
	int32_t lab_edge = 0;
	switch (in_campaign_mode)
	{
	default:
		assert(0);
		break;

	case CAMPAIGN_RANDOM_SEED:
	case CAMPAIGN_EXPLICIT_SEED:
	case CAMPAIGN_RANDOMIZE_ALL:
	case CAMPAIGN_MOTD:
	case CAMPAIGN_KIT:
		lab_edge = 7;
		break;

	case CAMPAIGN_COTW:
		lab_edge = __int_max(7, 11 - (int32_t)in_cotw.mission / 3);
		break;

	case CAMPAIGN_GRIND:
		lab_edge = __int_max(7, 11 - (int32_t)in_grind.mission / 3);
		break;
	}

	for (
		int32_t oz = lab_edge;
		oz < PLAN_CELLS_ASPECT - lab_edge - 3;
		++oz
		)
	{
		for (
			int32_t ox = lab_edge;
			ox < PLAN_CELLS_ASPECT - lab_edge - 3;
			++ox
			)
		{
			if (__space_for_lab_eh(ox, oz, out_plan))
			{
				//cut out the room
				for (
					int32_t z = 1;
					z < 4;
					++z
					)
					for (
						int32_t x = 1;
						x < 4;
						++x
						)
						__plan_write_cell(PLAN_OFFSET_XZ(ox + x, oz + z), PLAN_CELL_LAB, out_plan);

				if (__create_lab_exits_and_panic_rooms(ox, oz, in_options, out_plan, out_prng))
				{
					//one more created
					++labs_created_successfully;
				}
				else
				{
					//one failed
					++labs_could_not_create_exits;

					//back out of this room instance
					for (
						int32_t z = 0;
						z < 5;
						++z
						)
					{
						for (
							int32_t x = 0;
							x < 5;
							++x
							)
						{
							__plan_write_cell(PLAN_OFFSET_XZ(ox + x, oz + z), PLAN_CELL_WALL, out_plan);
						}
					}
				}
			}
			else
			{
				//not enough space
				++labs_did_not_fit;
			}
		}
	}
}
#endif//MAKE_LABS

//this is the legacy algorithm, paths may well cross
static void __carve_corridors(
	const uint32_t in_campaign_mode, const gen_campaign_t& in_cotw, const gen_campaign_t& in_grind,
	gen_plan_t& out_plan, prng_t& out_prng)
{
	::memset(&out_plan.cells, 0, sizeof(out_plan.cells));
	out_plan.airlock = 0;
	out_plan.security_console = 0;

	tunneler_t tunneler{};
	switch (in_campaign_mode)
	{
	default:
		assert(0);
		break;

	case CAMPAIGN_RANDOM_SEED:
	case CAMPAIGN_EXPLICIT_SEED:
	case CAMPAIGN_RANDOMIZE_ALL:
	case CAMPAIGN_MOTD:
	case CAMPAIGN_KIT:
		tunneler = __tunneler_make(TUNNELER_DEFAULT_LIFE, out_prng);
		break;

	case CAMPAIGN_COTW:
		//tunneler = __tunneler_make(c_min_uint32(TUNNELER_DEFAULT_LIFE, 50 + 10 * in_cotw.mission), in_prng);
		tunneler = __tunneler_make(__uint_min(TUNNELER_DEFAULT_LIFE, 50 + 10 * (5 + in_cotw.mission)), out_prng);	//trim off the first five missions (too easy)
		break;

	case CAMPAIGN_GRIND:
		tunneler = __tunneler_make(__uint_min(TUNNELER_DEFAULT_LIFE, 50 + 10 * in_grind.mission), out_prng);
		break;
	}

	while (0 < tunneler.life)
		__tunneler_update(out_prng, out_plan, tunneler);
}

static void __make_reactor_room(gen_plan_t& out_plan)
{
	constexpr uint8_t BLUEPRINT[REACTOR_ROOM_ASPECT][REACTOR_ROOM_ASPECT] =
	{
		{ PLAN_CELL_FLOOR, PLAN_CELL_FLOOR, PLAN_CELL_FLOOR, PLAN_CELL_FLOOR, PLAN_CELL_FLOOR },
		{ PLAN_CELL_FLOOR, PLAN_CELL_FLOOR, PLAN_CELL_COOLANT, PLAN_CELL_FLOOR, PLAN_CELL_FLOOR },
		{ PLAN_CELL_FLOOR, PLAN_CELL_COOLANT, PLAN_CELL_WALL, PLAN_CELL_COOLANT, PLAN_CELL_FLOOR },
		{ PLAN_CELL_FLOOR, PLAN_CELL_FLOOR, PLAN_CELL_COOLANT, PLAN_CELL_FLOOR, PLAN_CELL_FLOOR },
		{ PLAN_CELL_FLOOR, PLAN_CELL_FLOOR, PLAN_CELL_FLOOR, PLAN_CELL_FLOOR, PLAN_CELL_FLOOR },
	};

	for (
		int32_t x = 0;
		x < REACTOR_ROOM_ASPECT;
		++x
		)
		for (
			int32_t z = 0;
			z < REACTOR_ROOM_ASPECT;
			++z
			)
			__plan_write_cell(PLAN_OFFSET_XZ(PLAN_REACTOR_POSITION_AXIS + x - 2, PLAN_REACTOR_POSITION_AXIS + z - 2), BLUEPRINT[z][x], out_plan);
}

#if MAKE_CONTROL_ROOM
static bool __valid_control_room_eh(const uint32_t in_door, const uint32_t in_center, const gen_plan_t& in_plan)
{
	for (
		int32_t z = -2;
		z <= 2;
		++z
		)
	{
		for (
			int32_t x = -2;
			x <= 2;
			++x
			)
		{
			const uint32_t OFFSET = PLAN_OFFSET_DELTA(in_center, x, z);

			if (PLAN_OFFSET_TILE_X(OFFSET) < 1)
				return false;
			if (PLAN_OFFSET_TILE_X(OFFSET) > PLAN_CELLS_ASPECT - 2)
				return false;

			if (PLAN_OFFSET_TILE_Z(OFFSET) < 1)
				return false;
			if (PLAN_OFFSET_TILE_Z(OFFSET) > PLAN_CELLS_ASPECT - 2)
				return false;

			if (
				in_door != OFFSET &&
				PLAN_CELL_WALL != __plan_read_cell(in_plan, OFFSET)
				)
				return false;
		}
	}

	return true;
}

static bool __validate_and_cut_control_room(
	const uint32_t in_door, const uint32_t in_center,
	gen_plan_t& out_plan)
{
	if (!__valid_control_room_eh(in_door, in_center, out_plan))
		return false;

	for (
		int32_t z = -1;
		z <= 1;
		++z
		)
		for (
			int32_t x = -1;
			x <= 1;
			++x
			)
			__plan_write_cell(PLAN_OFFSET_DELTA(in_center, x, z), PLAN_CELL_FLOOR, out_plan);

	out_plan.security_console = in_center;

	return true;
}

static void __make_control_room(gen_plan_t& out_plan)
{
	for (
		int32_t x = 0;
		x < PLAN_CELLS_ASPECT;
		++x
		)
	{
		for (
			int32_t z = 0;
			z < PLAN_CELLS_ASPECT;
			++z
			)
		{
			const uint32_t OFFSET = PLAN_OFFSET_XZ(x, z);
			if (F256E_BREACH_OK == __plan_flags_256_entity(out_plan, OFFSET))
			{
				const uint8_t FLAGS = __plan_floor_around_floor_flags_256(out_plan, OFFSET);

				//this is certainly not all possible cases
				switch (FLAGS)
				{
				case 2:
				case 3:
				case 6:
				case 7:
					if (__validate_and_cut_control_room(OFFSET, PLAN_OFFSET_DELTA(OFFSET, 0, 2), out_plan))
						return;
					break;

				case 8:
				case 9:
				case 40:
				case 41:
				case 169:
					if (__validate_and_cut_control_room(OFFSET, PLAN_OFFSET_DELTA(OFFSET, 2, 0), out_plan))
						return;
					break;

				case 16:
				case 20:
				case 144:
				case 148:
					if (__validate_and_cut_control_room(OFFSET, PLAN_OFFSET_DELTA(OFFSET, -2, 0), out_plan))
						return;
					break;

				case 64:
				case 96:
				case 192:
				case 224:
					if (__validate_and_cut_control_room(OFFSET, PLAN_OFFSET_DELTA(OFFSET, 0, -2), out_plan))
						return;
					break;
				}
			}
		}
	}
}
#endif

static uint32_t __planner_generate_plan(
	const uint32_t in_campaign_mode, const gen_options32_t& in_options, const gen_campaign_t& in_cotw, const gen_campaign_t& in_grind,
	gen_plan_t& out_plan)
{
	assert(CAMPAIGN_USER != in_campaign_mode);

	assert(in_options.im_planner_infestation < INFESTATION_UNKEN);
	assert(in_options.im_planner_chance_doors < CHANCE_UNKEN);
	assert(in_options.im_planner_chance_fences < CHANCE_UNKEN);
	assert(in_options.im_planner_bit_invert_barriers < BIT_UNKEN);
	assert(in_options.im_planner_rooms < ROOMS_UNKEN);
	assert(in_options.im_planner_chance_sentries < CHANCE_UNKEN);
	assert(in_options.im_planner_bit_datacores < BIT_UNKEN);
	assert(in_options.im_planner_bit_armories < BIT_UNKEN);
	assert(in_options.im_planner_bit_repairs < BIT_UNKEN);

	assert(in_options.im_ruler_bit_initial_power < BIT_UNKEN);

	assert(in_options.mu_ruler_bit_breaches < BIT_UNKEN);
	assert(in_options.mu_ruler_bit_astro_creeps < BIT_UNKEN);
	assert(in_options.mu_ruler_difficulty < DIFFICULTY_UNKEN);
	assert(in_options.mu_ruler_power_failure < POWER_FAILURE_UNKEN);

	assert(in_options.im_ruler_bit_reactor_safety_protocol < BIT_UNKEN);

	assert(in_options.im_ruler_bit_beasts_lurk < BIT_UNKEN);

	prng_t prng = __prng_make(out_plan.seed);

	uint32_t iterations = 1;
	while (1)
	{
		__carve_corridors(in_campaign_mode, in_cotw, in_grind, out_plan, prng);
		__make_reactor_room(out_plan);

#if MAKE_CONTROL_ROOM
		__make_control_room(out_plan);
#endif

#if MAKE_LABS
		__make_labs(in_campaign_mode, in_options, in_cotw, in_grind, out_plan, prng);
#else
		options;
#endif

#if FINALIZE
		if (
			__prepare_for_game_rules(in_options, out_plan, prng) &&
			__post_generation_tests(out_plan)
			)
			return iterations;
		++iterations;
#else
		return iterations;
#endif
	}
}

static uint32_t __solidify_option(
	const uint32_t in_value, const uint32_t in_range,
	prng_t& out_prng)
{
	if (in_value >= in_range)
		return __prng_int32(in_range, out_prng);
	return in_value;
}

static uint32_t __unixtime()
{
	return  (uint32_t)::time(nullptr);
}

static gen_options64_t __randomized_immutable_options(const gen_plan_t& in_plan)
{
	gen_options64_t result{};

	//randomize all the IMMUTABLE fields (that don't change at runtime)
	{
		prng_t prng = __prng_make(in_plan.seed);
		result.im_planner_infestation = __prng_int32(INFESTATION_UNKEN, prng);
		result.im_planner_chance_doors = __prng_int32(CHANCE_UNKEN, prng);
		result.im_planner_chance_fences = __prng_int32(CHANCE_UNKEN, prng);
		result.im_planner_bit_invert_barriers = __prng_int32(BIT_UNKEN, prng);
		result.im_planner_rooms = __prng_int32(ROOMS_UNKEN, prng);
		result.im_planner_chance_sentries = __prng_int32(CHANCE_UNKEN, prng);
		result.im_planner_bit_datacores = __prng_int32(BIT_UNKEN, prng);
		result.im_planner_bit_armories = __prng_int32(BIT_UNKEN, prng);
		result.im_planner_bit_repairs = __prng_int32(BIT_UNKEN, prng);
		result.im_ruler_bit_initial_power = __prng_int32(BIT_UNKEN, prng);
		result.im_ruler_bit_reactor_safety_protocol = __prng_int32(BIT_UNKEN, prng);
		result.im_ruler_bit_beasts_lurk = __prng_int32(BIT_UNKEN, prng);
	}

	return result;
}

static uint32_t __campaign_seed(const gen_campaign_t& in_campaign)
{
	const year_week_t YEAR_WEEK = __year_week(in_campaign.start_unixtime);
	return COTW_CAMPAIGN_SEED(YEAR_WEEK.year, YEAR_WEEK.week);
}

static uint32_t __campaign_mission_seed(const gen_campaign_t& in_campaign)
{
	uint32_t result = __campaign_seed(in_campaign);
	prng_t prng = __prng_make(result);
	for (
		uint32_t i = 0; 
		i < in_campaign.mission; 
		++i
		)
		result = __prng_generate(prng);

	return result;
}

static bool __cotw_hardest_settings_eh(const gen_options64_t& in_options)
{
	if (INFESTATION_TOTAL != in_options.im_planner_infestation)
		return false;

	if (CHANCE_NONE != in_options.im_planner_chance_doors)
		return false;

	if (CHANCE_NONE != in_options.im_planner_chance_fences)
		return false;

	if (1 != in_options.im_planner_bit_invert_barriers)
		return false;

	if (ROOMS_EMPTY != in_options.im_planner_rooms)
		return false;

	if (CHANCE_NONE != in_options.im_planner_chance_sentries)
		return false;

	//0 == more
	if (0 != in_options.im_planner_bit_datacores)
		return false;

	//0 == more
	//community pointed out that going from more to fewer makes the game end due to ammo depletion, so starting with fewer and going to more
	if (0 != in_options.im_planner_bit_armories)
		return false;

	//1 == fewer
	if (1 != in_options.im_planner_bit_repairs)
		return false;

	//0 == off
	if (0 != in_options.im_ruler_bit_initial_power)
		return false;

	//1 == random
	if (1 != in_options.mu_ruler_bit_breaches)
		return false;

	//1 == on
	if (1 != in_options.mu_ruler_bit_astro_creeps)
		return false;

	if (DIFFICULTY_INSANE != in_options.mu_ruler_difficulty)
		return false;

	if (POWER_FAILURE_CHANCE_8 != in_options.mu_ruler_power_failure)
		return false;

	if (1 != in_options.im_ruler_bit_reactor_safety_protocol)
		return false;

	if (0 != in_options.im_ruler_bit_beasts_lurk)
		return false;

	return true;
}

static gen_options64_t __cotw_mutate_options(
	const gen_options64_t& in_options,
	prng_t& out_prng)
{
	gen_options64_t result = in_options;

	while (!__cotw_hardest_settings_eh(result))
	{
		//pick an option to nerf
		switch (__prng_generate(out_prng) % 16)
		{
		default:
			assert(0);
			return {};

			//im_planner_infestation
		case 0:
			//0 = infestation_none, 1 = infestation_less, 2 = infestation_more, 3 = infestation_total
			if (result.im_planner_infestation < INFESTATION_TOTAL)
			{
				++result.im_planner_infestation;
				return result;
			}
			break;

			//im_planner_chance_doors
		case 1:
			//0 = chance_none, 1 = chance_more, 2 = chance_fewer
			if (CHANCE_MORE == result.im_planner_chance_doors)
			{
				result.im_planner_chance_doors = CHANCE_FEWER;
				return result;
			}
			if (CHANCE_FEWER == result.im_planner_chance_doors)
			{
				result.im_planner_chance_doors = CHANCE_NONE;
				return result;
			}
			break;

			//im_planner_chance_fences
		case 2:
			//0 = chance_none, 1 = chance_more, 2 = chance_fewer
			if (CHANCE_MORE == result.im_planner_chance_fences)
			{
				result.im_planner_chance_fences = CHANCE_FEWER;
				return result;
			}
			if (CHANCE_FEWER == result.im_planner_chance_fences)
			{
				result.im_planner_chance_fences = CHANCE_NONE;
				return result;
			}
			break;

			//im_planner_bit_barriers
		case 3:
			//0 = normal, 1 = inverted
			if (0 == result.im_planner_bit_invert_barriers)
			{
				result.im_planner_bit_invert_barriers = 1;
				return result;
			}
			break;

			//im_planner_rooms
		case 4:
			//0 = rooms_empty, 1 = rooms_filled, 2 = rooms_pillars
			if (ROOMS_FILLED == result.im_planner_rooms)
			{
				result.im_planner_rooms = ROOMS_PILLARS;
				return result;
			}
			if (ROOMS_PILLARS == result.im_planner_rooms)
			{
				result.im_planner_rooms = ROOMS_EMPTY;
				return result;
			}
			break;

			//im_planner_chance_sentries
		case 5:
			//0 = chance_none, 1 = chance_more, 2 = chance_fewer
			if (CHANCE_MORE == result.im_planner_chance_sentries)
			{
				result.im_planner_chance_sentries = CHANCE_FEWER;
				return result;
			}
			if (CHANCE_FEWER == result.im_planner_chance_sentries)
			{
				result.im_planner_chance_sentries = CHANCE_NONE;
				return result;
			}
			break;

			//im_planner_bit_datacores
		case 6:
			//0 = more, 1 = fewer
			if (1 == result.im_planner_bit_datacores)
			{
				result.im_planner_bit_datacores = 0;
				return result;
			}
			break;

			//im_planner_bit_armories
			//community pointed out that going from more to fewer makes the game end due to ammo depletion, so starting with fewer and going to more
		case 7:
			//0 = more, 1 = fewer
			if (1 == result.im_planner_bit_armories)
			{
				result.im_planner_bit_armories = 0;//more
				return result;
			}
			break;

			//im_planner_bit_repairs
		case 8:
			//0 = more, 1 = fewer
			if (0 == result.im_planner_bit_repairs)
			{
				result.im_planner_bit_repairs = 1;
				return result;
			}
			break;

			//im_ruler_bit_initial_power
		case 9:
			//0 = starts off, 1 = starts on
			if (1 == result.im_ruler_bit_initial_power)
			{
				result.im_ruler_bit_initial_power = 0;
				return result;
			}
			break;

			//mu_ruler_bit_breaches
		case 10:
			//0 = normal, 1 = random
			if (0 == result.mu_ruler_bit_breaches)
			{
				result.mu_ruler_bit_breaches = 1;
				return result;
			}
			break;

			//mu_ruler_bit_astro_creeps
		case 11:
			//0 = off, 1 = on
			if (0 == result.mu_ruler_bit_astro_creeps)
			{
				result.mu_ruler_bit_astro_creeps = 1;
				return result;
			}
			break;

			//mu_ruler_difficulty
				//these are measured times on explicit 00000000 with default settings (less astro-creeps)
					//qbtf::model::breach_update() : easier 386.650024 beast spawn SATURATED
					//qbtf::model::breach_update() : random all 296.316681 beast spawn SATURATED
					//qbtf::model::breach_update() : random each 270.766693 beast spawn SATURATED
					//qbtf::model::breach_update() : easier + 273.666687 beast spawn SATURATED
					//qbtf::model::breach_update() : hard 188.183350 beast spawn SATURATED
					//qbtf::model::breach_update() : hard + 138.583344 beast spawn SATURATED
					//qbtf::model::breach_update() : insane 122.116676 beast spawn SATURATED
		case 12:
			//0 = difficulty_easier, 1 = difficulty_easier_plus, 2 = difficulty_hard, 3 = difficulty_hard_plus, 4 = difficulty_insane, 5 = difficulty_random_all, //6 = difficulty_random_each
			if (DIFFICULTY_EASIER == result.mu_ruler_difficulty)
			{
				result.mu_ruler_difficulty = DIFFICULTY_RANDOM_ALL;
				return result;
			}
			if (DIFFICULTY_RANDOM_ALL == result.mu_ruler_difficulty)
			{
				result.mu_ruler_difficulty = DIFFICULTY_RANDOM_EACH;
				return result;
			}
			if (DIFFICULTY_RANDOM_EACH == result.mu_ruler_difficulty)
			{
				result.mu_ruler_difficulty = DIFFICULTY_EASIER_PLUS;
				return result;
			}
			if (DIFFICULTY_EASIER_PLUS == result.mu_ruler_difficulty)
			{
				result.mu_ruler_difficulty = DIFFICULTY_HARD;
				return result;
			}
			if (DIFFICULTY_HARD == result.mu_ruler_difficulty)
			{
				result.mu_ruler_difficulty = DIFFICULTY_HARD_PLUS;
				return result;
			}
			if (DIFFICULTY_HARD_PLUS == result.mu_ruler_difficulty)
			{
				result.mu_ruler_difficulty = DIFFICULTY_INSANE;
				return result;
			}
			break;

			//mu_ruler_power_failure
		case 13:
			//0 = power_failure_none, 1 = power_failure_chance_8, 2 = power_failure_chance_16, 3 = power_failure_chance_32, 4  = power_failure_time_30_60, 5 = power_failure_time_60_120, 6 = power_failure_time_120_240
			if (POWER_FAILURE_NONE == result.mu_ruler_power_failure)
			{
				result.mu_ruler_power_failure = POWER_FAILURE_TIME_120_240;
				return result;
			}
			if (POWER_FAILURE_TIME_120_240 == result.mu_ruler_power_failure)
			{
				result.mu_ruler_power_failure = POWER_FAILURE_TIME_60_120;
				return result;
			}
			if (POWER_FAILURE_TIME_60_120 == result.mu_ruler_power_failure)
			{
				result.mu_ruler_power_failure = POWER_FAILURE_TIME_30_60;
				return result;
			}
			if (POWER_FAILURE_TIME_30_60 == result.mu_ruler_power_failure)
			{
				result.mu_ruler_power_failure = POWER_FAILURE_CHANCE_32;
				return result;
			}
			if (POWER_FAILURE_CHANCE_32 == result.mu_ruler_power_failure)
			{
				result.mu_ruler_power_failure = POWER_FAILURE_CHANCE_16;
				return result;
			}
			if (POWER_FAILURE_CHANCE_16 == result.mu_ruler_power_failure)
			{
				result.mu_ruler_power_failure = POWER_FAILURE_CHANCE_8;
				return result;
			}
			break;

			//im_ruler_bit_reactor_safety_protocol
		case 14:
			//0 = off, 1 = on
			if (0 == result.im_ruler_bit_reactor_safety_protocol)
			{
				result.im_ruler_bit_reactor_safety_protocol = 1;
				return result;
			}
			break;

			//mu_ruler_bit_beasts_lurk (assume off is "harder")
		case 15:
			//0 = off, 1 = on
			if (1 == result.im_ruler_bit_beasts_lurk)
			{
				result.im_ruler_bit_beasts_lurk = 0;
				return result;
			}
			break;
		}//switch (out_prng.generate() % 16)
	}//while (!__cotw_hardest_settings_eh(result))

	//will return here if the result is maxed out / hardest
	return result;
}

static gen_options64_t __cotw_options(const int32_t in_year, const int32_t in_week, const uint32_t in_mission)
{
	gen_options64_t result{};

	//set the "easiest" settings
	result.im_planner_infestation = INFESTATION_NONE;
	result.im_planner_chance_doors = CHANCE_MORE;
	result.im_planner_chance_fences = CHANCE_MORE;
	result.im_planner_bit_invert_barriers = BIT_CLEAR;
	result.im_planner_rooms = ROOMS_FILLED;
	result.im_planner_chance_sentries = CHANCE_MORE;
	result.im_planner_bit_datacores = BIT_SET;
	result.im_planner_bit_armories = BIT_SET;	//community pointed out that going from more to less makes the game end due to ammo depletion, so starting with fewer and going to more
	result.im_planner_bit_repairs = BIT_CLEAR;

	result.im_ruler_bit_initial_power = BIT_SET;
	result.im_ruler_bit_reactor_safety_protocol = BIT_CLEAR;
	result.im_ruler_bit_beasts_lurk = BIT_SET;

	result.mu_ruler_bit_breaches = BIT_CLEAR;
	result.mu_ruler_bit_astro_creeps = BIT_CLEAR;
	result.mu_ruler_difficulty = DIFFICULTY_EASIER;
	result.mu_ruler_power_failure = POWER_FAILURE_NONE;

	//mutate for each mission
	prng_t prng = __prng_make(COTW_CAMPAIGN_SEED(in_year, in_week));
	for (
		uint32_t m = 0;
		m < in_mission;
		++m
		)
	{
		//twice per mission
		result = __cotw_mutate_options(result, prng);
		result = __cotw_mutate_options(result, prng);
	}

	//in this case we don't want to have anything be UNKEN
	assert(result.im_planner_infestation < INFESTATION_UNKEN);
	assert(result.im_planner_chance_doors < CHANCE_UNKEN);
	assert(result.im_planner_chance_fences < CHANCE_UNKEN);
	assert(result.im_planner_bit_invert_barriers < BIT_UNKEN);
	assert(result.im_planner_rooms < ROOMS_UNKEN);
	assert(result.im_planner_chance_sentries < CHANCE_UNKEN);
	assert(result.im_planner_bit_datacores < BIT_UNKEN);
	assert(result.im_planner_bit_armories < BIT_UNKEN);
	assert(result.im_planner_bit_repairs < BIT_UNKEN);
	assert(result.im_ruler_bit_initial_power < BIT_UNKEN);
	assert(result.mu_ruler_bit_breaches < BIT_UNKEN);
	assert(result.mu_ruler_bit_astro_creeps < BIT_UNKEN);
	assert(result.mu_ruler_difficulty < DIFFICULTY_UNKEN);
	assert(result.mu_ruler_power_failure < POWER_FAILURE_UNKEN);
	assert(result.im_ruler_bit_reactor_safety_protocol < BIT_UNKEN);
	assert(result.im_ruler_bit_beasts_lurk < BIT_UNKEN);

	return result;
}

static gen_options64_t __cotw_options(const gen_campaign_t& in_campaign)
{
	const year_week_t YEAR_WEEK = __year_week(in_campaign.start_unixtime);
	return __cotw_options(YEAR_WEEK.year, YEAR_WEEK.week, in_campaign.mission);
}

static void __setup_for_campaign_mode(
	const uint32_t in_campaign_mode, const gen_motd_t& in_motd,
	gen_campaign_t& out_cotw, gen_campaign_t& out_grind, gen_options64_t& out_options, gen_plan_t& out_plan)
{
	switch (in_campaign_mode)
	{
	default:
		assert(0);
		break;

		//options are at the user's discretion
	case CAMPAIGN_RANDOM_SEED:
		out_plan.seed = __unixtime();
		break;

		//options are at the user's discretion
	case CAMPAIGN_EXPLICIT_SEED:
		out_plan.seed = 0;
		break;

	case CAMPAIGN_RANDOMIZE_ALL:
		out_plan.seed = __unixtime();
		out_options = __randomized_immutable_options(out_plan);
		break;

	case CAMPAIGN_MOTD:
		out_plan.seed = in_motd.seed;
		out_options = __randomized_immutable_options(out_plan);
		break;

	case CAMPAIGN_COTW:
		if (0 == out_cotw.mission)
			out_cotw.start_unixtime = __unixtime();
		out_plan.seed = __campaign_mission_seed(out_cotw);
		out_options = __cotw_options(out_cotw);
		break;

	case CAMPAIGN_KIT:
		out_plan.seed = __unixtime();
		out_options = __randomized_immutable_options(out_plan);
		break;

		//there is a whole backend to this...
	case CAMPAIGN_USER:
		//vc_deployment_user_plans.n_user_plans_schedule_refresh();
		break;

	case CAMPAIGN_GRIND:
		if (0 == out_grind.mission)
			out_grind.start_unixtime = __unixtime();
		out_plan.seed = __campaign_mission_seed(out_grind);
		out_options = __randomized_immutable_options(out_plan);
		break;
	}
}

static gen_motd_t __motd_make(const year_month_day_t& in)
{
	gen_motd_t result;

	result.date.year = in.year;
	assert(in.month < 12);
	result.date.month = in.month;
	assert(in.day < 31);
	result.date.day = in.day;

	return result;
}

static gen_options32_t __make_options32(const gen_options64_t& in_options64, const gen_plan_t& in_plan)
{
	prng_t prng = __prng_make(__jenkins_hash(&in_plan.seed, sizeof(in_plan.seed), 0));

	gen_options32_t result;

	result.im_planner_infestation = __solidify_option(in_options64.im_planner_infestation, INFESTATION_UNKEN, prng);
	result.im_planner_chance_doors = __solidify_option(in_options64.im_planner_chance_doors, CHANCE_UNKEN, prng);
	result.im_planner_chance_fences = __solidify_option(in_options64.im_planner_chance_fences, CHANCE_UNKEN, prng);
	result.im_planner_bit_invert_barriers = __solidify_option(in_options64.im_planner_bit_invert_barriers, BIT_UNKEN, prng);
	result.im_planner_rooms = __solidify_option(in_options64.im_planner_rooms, ROOMS_UNKEN, prng);
	result.im_planner_chance_sentries = __solidify_option(in_options64.im_planner_chance_sentries, CHANCE_UNKEN, prng);
	result.im_planner_bit_datacores = __solidify_option(in_options64.im_planner_bit_datacores, BIT_UNKEN, prng);
	result.im_planner_bit_armories = __solidify_option(in_options64.im_planner_bit_armories, BIT_UNKEN, prng);
	result.im_planner_bit_repairs = __solidify_option(in_options64.im_planner_bit_repairs, BIT_UNKEN, prng);
	result.im_ruler_bit_initial_power = __solidify_option(in_options64.im_ruler_bit_initial_power, BIT_UNKEN, prng);
	result.mu_ruler_bit_breaches = __solidify_option(in_options64.mu_ruler_bit_breaches, BIT_UNKEN, prng);
	result.mu_ruler_bit_astro_creeps = __solidify_option(in_options64.mu_ruler_bit_astro_creeps, BIT_UNKEN, prng);
	result.mu_ruler_difficulty = __solidify_option(in_options64.mu_ruler_difficulty, DIFFICULTY_UNKEN, prng);
	result.mu_ruler_power_failure = __solidify_option(in_options64.mu_ruler_power_failure, POWER_FAILURE_UNKEN, prng);
	result.im_ruler_bit_reactor_safety_protocol = __solidify_option(in_options64.im_ruler_bit_reactor_safety_protocol, BIT_UNKEN, prng);
	result.im_ruler_bit_beasts_lurk = __solidify_option(in_options64.im_ruler_bit_beasts_lurk, BIT_UNKEN, prng);

	assert(result.im_planner_infestation < INFESTATION_UNKEN);
	assert(result.im_planner_chance_doors < CHANCE_UNKEN);
	assert(result.im_planner_chance_fences < CHANCE_UNKEN);
	assert(result.im_planner_bit_invert_barriers < BIT_UNKEN);
	assert(result.im_planner_rooms < ROOMS_UNKEN);
	assert(result.im_planner_chance_sentries < CHANCE_UNKEN);
	assert(result.im_planner_bit_datacores < BIT_UNKEN);
	assert(result.im_planner_bit_armories < BIT_UNKEN);
	assert(result.im_planner_bit_repairs < BIT_UNKEN);
	assert(result.im_ruler_bit_initial_power < BIT_UNKEN);
	assert(result.mu_ruler_bit_breaches < BIT_UNKEN);
	assert(result.mu_ruler_bit_astro_creeps < BIT_UNKEN);
	assert(result.mu_ruler_difficulty < DIFFICULTY_UNKEN);
	assert(result.mu_ruler_power_failure < POWER_FAILURE_UNKEN);
	assert(result.im_ruler_bit_reactor_safety_protocol < BIT_UNKEN);
	assert(result.im_ruler_bit_beasts_lurk < BIT_UNKEN);

	return result;
}

//entry point
//entry point
//entry point
//entry point

int main()
{
	//the main thing to set is the CAMPAIGN MODE, which in turn uses all the inputs in various ways...
	//start by setting this to the various legal values
	const uint32_t INPUT_CAMPAIGN_MODE = CAMPAIGN_MOTD;

	//motd depends on year / month / day (set whatever you want)
	const gen_motd_t INPUT_MOTD = __motd_make(__year_month_day(__unixtime()));

	//the current seed for campaigns depends on the current mission and when the campaign was started
	//set whatever mission you would like to generate
	//set the CAMPAIGN MODE to CAMPAIGN_COTW to use this
	gen_campaign_t input_cotw{};
	input_cotw.mission = 0;
	input_cotw.start_unixtime = (uint32_t)::time(nullptr);

	//the current seed for campaigns depends on the current mission and when the campaign was started
	//set whatever mission you would like to generate
	//set the CAMPAIGN MODE to CAMPAIGN_GRIND to use this
	gen_campaign_t input_grind{};
	input_grind.mission = 0;
	input_grind.start_unixtime = (uint32_t)::time(nullptr);

	//user-facing options grew beyond 32 bits over time... :P
	gen_options64_t input_options64{};

	//this is what we are trying to generate (the value of all the cells as well as the position of the airlock and security console)
	//note that the seed shouldn't conceptually be a part of that (the plan is generated FROM the seed),
	//but there are legacy reasons for including it (things like visualization stuff being selected based on seed)
	gen_plan_t output_plan{};

	//here we setup a lot of stuff mainly based on CAMPAIGN MODE, see the implementation...
	__setup_for_campaign_mode(INPUT_CAMPAIGN_MODE, INPUT_MOTD, input_cotw, input_grind, input_options64, output_plan);

	//from here we are ready to generate...
	//from here we are ready to generate...
	//from here we are ready to generate...
	//from here we are ready to generate...

	//make 32 bit options (actually used by plan generation and the sim)
	const gen_options32_t INPUT_OPTIONS32 = __make_options32(input_options64, output_plan);

	//actually generate the plan from the myriad of options we have accrued... :P
	const uint32_t ITERATIONS = __planner_generate_plan(INPUT_CAMPAIGN_MODE, INPUT_OPTIONS32, input_cotw, input_grind, output_plan);
	::fprintf_s(stdout, "generated in %u iterations...", ITERATIONS);

	//this output is similar to what is shown in the ASCII plan preview in the game...
	for (
		uint32_t z = 0;
		z < PLAN_CELLS_ASPECT;
		++z
		)
	{
		for (
			uint32_t x = 0;
			x < PLAN_CELLS_ASPECT;
			++x
			)
		{
			const uint32_t OFFSET = x + z * PLAN_CELLS_ASPECT;
			const uint8_t CELL = __plan_read_cell(output_plan, OFFSET);
			char c = ' ';
			if (PLAN_CELL_FLOORBIT & CELL)
			{
				if (OFFSET == output_plan.airlock)
				{
					c = '@';
				}
				else
				{
					switch (CELL)
					{
					default:
						assert(0);
						break;

						//don't show
					case PLAN_CELL_PANIC:
						break;

					case PLAN_CELL_FLOOR:
						if (output_plan.security_console == OFFSET)
							c = 'M';
						else if (F256E_BREACH_OK == __plan_flags_256_entity(output_plan, OFFSET))
							c = 'B';
						else
							c = '#';
						break;

					case PLAN_CELL_LAB:
						if (F256E_BREACH_OK == __plan_flags_256_entity(output_plan, OFFSET))
							c = 'B';
						else
							c = 'L';
						break;

					case PLAN_CELL_DATA:
						c = 'D';
						break;

					case PLAN_CELL_COOLANT:
						c = 'C';
						break;

					case PLAN_CELL_SENTRY:
						c = 'S';
						break;

					case PLAN_CELL_REPAIR:
						c = 'R';
						break;

					case PLAN_CELL_ARMORY:
						c = 'A';
						break;

					case PLAN_CELL_DOOR_A:
					case PLAN_CELL_DOOR_B:
						c = '+';
						break;
						break;

					case PLAN_CELL_FENCE:
						c = 'X';
						break;

						//Breakers can be in same cell as Breaches, so we may want to prioritize the Breaches
						//this will make it appear like there is a Breaker missing (in the legend) because it is under the Breach
					case PLAN_CELL_BREAKER:
						if (F256E_BREACH_OK == __plan_flags_256_entity(output_plan, OFFSET))
							c = 'B';
						else
							c = '%';
						break;
					}//cell value switch
				}//not airlock
			}//floor bit

			::fputc(c, stdout);
		}
		::fputc('\n', stdout);
	}
}
