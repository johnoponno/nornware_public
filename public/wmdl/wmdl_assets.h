#pragma once

#include "../minyin/minyin.h"
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
	minyin_bitmap_t backgrounds[7];

	minyin_bitmap_t portal;
	minyin_bitmap_t server;
	minyin_bitmap_t arcs;
	minyin_bitmap_t spikygreen;
	minyin_bitmap_t blueblob;
	minyin_bitmap_t brownblob;
	minyin_bitmap_t penguin;
	minyin_bitmap_t plant;
	minyin_bitmap_t scorpion;
	minyin_bitmap_t firedude;
	minyin_bitmap_t bat;
	minyin_bitmap_t gui_server_fixed;
	minyin_bitmap_t gui_server_broken;
	minyin_bitmap_t idle;

	minyin_bitmap_t tiles;
	minyin_bitmap_t hero;
	minyin_bitmap_t whip;

	//sd_bitmap_t dust_far;
	//sd_bitmap_t dust_near;
	minyin_bitmap_t flake;

	minyin_font_t font;

	uint32_t anim_target[WMDL_MAX_TILE];

	uint8_t key_index;
	uint8_t text_edge_index;
};

bool wmdl_assets_init(wmdl_assets_t& out_assets, std::vector<minyin_sound_request_t>& out_sounds);
