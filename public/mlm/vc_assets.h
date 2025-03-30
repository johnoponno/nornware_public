#pragma once

#include "../microlib/c_fixed_string.h"
#include "../microlib/fs.h"
#include "m_immutable.h"

#if 0
#define MLM_VC_SCREEN_WIDTH 384	//24 tiles
#define MLM_VC_SCREEN_HEIGHT 208	//13 tiles
#else
#define MLM_VC_SCREEN_WIDTH 256
#define MLM_VC_SCREEN_HEIGHT 144
#endif

#define MLM_VC_GUI_INSIDE 1
#define MLM_VC_GUI_LEFT 2
#define MLM_VC_GUI_RIGHT 4

#define NINJA_VC_TILE_BITS_KEY 1

#if 0
#define NINJA_VC_TILES_ON_SCREEN_X 24
#define NINJA_VC_TILES_ON_SCREEN_Y 13
#else
#define NINJA_VC_TILES_ON_SCREEN_X 32
#define NINJA_VC_TILES_ON_SCREEN_Y 18
#endif

#define NINJA_VC_TILEFLAGS_REPLACE 1
#define NINJA_VC_TILEFLAGS_NON_PASSAGE 2
#define NINJA_VC_TILEFLAGS_PASSAGE 4

#define NINJA_VC_SOUNDS_NUM_JUMPS 5

#define NINJA_VC_SCREENS_IN_WORLD_X 8
#define NINJA_VC_SCREENS_IN_WORLD_Y 8
static_assert(MLM_M_WORLD_WIDTH / NINJA_VC_TILES_ON_SCREEN_X == NINJA_VC_SCREENS_IN_WORLD_X, "wtf!?");
static_assert(MLM_M_WORLD_HEIGHT / NINJA_VC_TILES_ON_SCREEN_Y == NINJA_VC_SCREENS_IN_WORLD_Y, "wtf!?");

namespace mlm
{
	enum
	{
		VC_FX_TILE,
		VC_FX_SMOKE,
		VC_FX_BLADE,
		VC_FX_GIB,
		VC_FX_ANIMATED,
	};

	enum
	{
		VC_SCREEN_MAIN,
		VC_SCREEN_CREDITS,
	};

	enum
	{
		VC_SND_LAND,
		VC_SND_SLIDE,

		VC_SND_STEP1,
		VC_SND_STEP2,

		VC_SND_PICKFLOWER,
		VC_SND_OUTWATER,
		VC_SND_INWATER,
		VC_SND_SWITCH,
		VC_SND_SAVE,
		VC_SND_INFO,
		VC_SND_INFO2,

		VC_SND_JUMP1,
		VC_SND_JUMP2,
		VC_SND_JUMP3,
		VC_SND_JUMP4,
		VC_SND_JUMP5,

		VC_SND_DIE_IMPALED,
		VC_SND_DIE_DROWNED,
		VC_SND_DIE_BURNED,
		VC_SND_DIE_ELECTRINATED,
		VC_SND_DIE_WALKER,
		VC_SND_DIE_WALKER2,
		VC_SND_DIE_JUMPER,
		VC_SND_DIE_JUMPER2,
		VC_SND_DIE_OUT_OF_BOUNDS,

		VC_SND_BULLET_FIRE,
		VC_SND_BULLET_HIT,
		VC_SND_MELEE,
		VC_SND_MEELE_HIT,

		VC_NUM_SOUNDS,
	};

	enum
	{
		VC_GFX_FLOWER_GLOW,
		VC_GFX_CHECKPOINT_CURRENT,
		VC_GFX_CLEAR,
		VC_GFX_WATER,
		VC_GFX_INFO,
		VC_GFX_INFO_TOUCHED,
		VC_GFX_TELEPORT_GLOW,
		VC_GFX_INFO2,
		VC_GFX_INFO2_TOUCHED,

		VC_NUM_GFX,
	};

	enum
	{
		WANG_OFF,
		WANG_2_CORNER,
		WANG_BLOB,
	};

	enum
	{
		VC_PLAX_SNOWY_MOUNTAINS,
		VC_PLAX_PURPLE_CAVES,
		VC_PLAX_UNDERSEA,
		VC_PLAX_WOODY,
		VC_PLAX_MONOLITH,
		VC_PLAX_VAMPIRE,
		VC_PLAX_CITY,
		VC_PLAX_MUMIN,
		VC_PLAX_MIRKWOOD,
		VC_PLAX_INDUSTRIAL,
		VC_PLAX_ANOTHER_WORLD,
		VC_PLAX_ALIEN,

		VC_PLAX_NUM_STYLES,
	};

	struct vc_sprite_t
	{
		c_blob_t bitmap;
		int32_t half_width;
		int32_t half_height;
		int32_t width;
		int32_t height;
	};

	/*
	struct vc_named_bitmap_t
	{
		c_path_t name;
		c_blob_t bitmap;
	};
	*/

	struct vc_screen_t
	{
		c_path_t ambience;
		uint32_t restart : 1;
		uint32_t room : 3;
		uint32_t plax : 4;
	};

	/*
	struct vc_terminal_t
	{
		uint32_t key;
		const vc_named_bitmap_t* contents;
		uint32_t offset;
	};
	*/

	struct vc_assets_t
	{
		explicit vc_assets_t();
		~vc_assets_t();

		/*
		uint8_t palettize_color(const uint8_t in_r, const uint8_t in_g, const uint8_t in_b) const;
		uint8_t palettize_color(const uint16_t in_color) const;
		*/

		/*
		ds_engine_t engine;
		ds_container_t sounds;
		*/

		c_blob_t tiles;
		uint32_t tile_bits[MLM_M_MAX_TILE];
#if 0
		c_blob_t farplane_assets[54];
#else
		c_blob_t tempcave;
#endif
		uint32_t gfx[VC_NUM_GFX];
		struct
		{
			uint32_t anim_target : 30;
			uint32_t wang : 2;
		} tile_info[MLM_M_MAX_TILE];

		c_blob_t gui_big_font;

		//uint16_t sd_palette[256];//FIXME: shouldn't really be here, want this entire lib to only run on 8 bit, currently used to convert arbitrary colors to palette

		c_blob_t cursor;
		c_blob_t blades;
		c_blob_t gibs;
		c_blob_t impalement;
		c_blob_t drowning;
		c_blob_t burning;
		c_blob_t electrination;
		c_blob_t special_icons;
		c_blob_t fish;
		c_blob_t heavy;
		vc_sprite_t hero;
		vc_sprite_t walker;
		vc_sprite_t walker2;
		vc_sprite_t jumper;
		vc_sprite_t jumper2;
		vc_sprite_t swimmer;
		vc_sprite_t swimmer2;
		vc_screen_t screens[NINJA_VC_SCREENS_IN_WORLD_X * NINJA_VC_SCREENS_IN_WORLD_Y];

#if 0
		vc_named_bitmap_t* terminal_content;
		uint32_t num_terminal_content;

		vc_terminal_t* terminal;
		uint32_t num_terminal;
#endif
		uint8_t orange;
		uint8_t dark_orange;
		uint8_t gray;
		uint8_t dark_gray;
		uint8_t light_blue;
		uint8_t blue;
		uint8_t green;
		uint8_t dark_green;
		uint8_t red;
		uint8_t dark_red;
		uint8_t that_pink;
		uint8_t yellow;
		uint8_t white;
	};

	struct vc_sound_request_t
	{
		uint32_t id;
		const char* file;
		uint32_t channels;
	};

	constexpr struct
	{
		const char* name;
		uint32_t default;
	} VC_GFX_NAME_DEFAULT[VC_NUM_GFX] =
	{
		{ "INDEX_FLOWER_GLOW", 13 },
		{ "INDEX_CHECKPOINT_CURRENT", 14 },
		{ "INDEX_CLEAR", 0 },
		{ "INDEX_WATER", 241 },
		{ "INDEX_INFO", 34 },
		{ "INDEX_INFO_TOUCHED", 35 },
		{ "INDEX_TELEPORT_GLOW", 0 },
		{ "INDEX_INFO2", UINT32_MAX },
		{ "INDEX_INFO2_TOUCHED", UINT32_MAX },
	};

	constexpr char* ASSET_AMBIENCE = "Cave_atmo_waterdrip.ogg";
}
