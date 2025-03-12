#pragma once

#include "../minyin/minyin.h"
#include "../minyin/micron.h"
#include "wmdl_model.h"

struct minyin_sound_request_t;

#define WMDL_TITLE_COLOR 255
#define WMDL_TEXT_COLOR 223
#define WMDL_PROMPT_COLOR 239

enum
{
	WMDL_SND_SPAWN,
	WMDL_SND_FIXSERVER,
	WMDL_SND_HEROWHIP,
	WMDL_SND_HEROJUMP01,
	WMDL_SND_HEROJUMP02,
	WMDL_SND_HEROJUMP03,
	WMDL_SND_HEROJUMP04,
	WMDL_SND_HEROJUMP05,
	WMDL_SND_HEROLAND01,
	WMDL_SND_HEROLAND02,
	WMDL_SND_HEROLAND03,
	WMDL_SND_HEROLAND04,
	WMDL_SND_HERODIE01,
	WMDL_SND_HERODIE02,
	WMDL_SND_HERODIE03,
	WMDL_SND_CHECKPOINT,
	WMDL_SND_SPIKYGREEN,
	WMDL_SND_BLUEBLOB,
	WMDL_SND_BROWNBLOB,
	WMDL_SND_PLANT,
	WMDL_SND_SCORPION,
	WMDL_SND_SLIDERDEATH,
	WMDL_SND_BATFLEE,
	WMDL_SND_KEY,
	WMDL_SND_SLIDER,

	WMDL_NUM_SOUNDS,
};

struct wmdl_assets_t
{
	micron_bitmap_t backgrounds[7];

	micron_bitmap_t portal;
	micron_bitmap_t server;
	micron_bitmap_t arcs;
	micron_bitmap_t spikygreen;
	micron_bitmap_t blueblob;
	micron_bitmap_t brownblob;
	micron_bitmap_t penguin;
	micron_bitmap_t plant;
	micron_bitmap_t scorpion;
	micron_bitmap_t firedude;
	micron_bitmap_t bat;
	micron_bitmap_t gui_server_fixed;
	micron_bitmap_t gui_server_broken;
	micron_bitmap_t idle;

	micron_bitmap_t tiles;
	micron_bitmap_t hero;
	micron_bitmap_t whip;

	//sd_bitmap_t dust_far;
	//sd_bitmap_t dust_near;
	micron_bitmap_t flake;

	micron_font_t font;

	uint32_t anim_target[WMDL_MAX_TILE];

	uint8_t key_index;
	uint8_t text_edge_index;
};

bool wmdl_assets_init(wmdl_assets_t& out_assets, std::vector<minyin_sound_request_t>& out_sounds);
