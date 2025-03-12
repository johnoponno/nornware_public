#pragma once

#include "../minyin/sd_bitmap.h"
#include "../minyin/sd_fontv.h"
#include "tpmn_model.h"

struct micron_sound_request_t;

#define TPMN_TITLE_COLOR 0xffff
#define TPMN_TEXT_COLOR 0xc7bf

enum
{
	TPMN_SND_SPAWN,
	TPMN_SND_FIXSERVER,
	TPMN_SND_HEROWHIP,
	TPMN_SND_HEROJUMP01,
	TPMN_SND_HEROJUMP02,
	TPMN_SND_HEROJUMP03,
	TPMN_SND_HEROJUMP04,
	TPMN_SND_HEROJUMP05,
	TPMN_SND_HEROLAND01,
	TPMN_SND_HEROLAND02,
	TPMN_SND_HEROLAND03,
	TPMN_SND_HEROLAND04,
	TPMN_SND_HERODIE01,
	TPMN_SND_HERODIE02,
	TPMN_SND_HERODIE03,
	TPMN_SND_CHECKPOINT,
	TPMN_SND_SPIKYGREEN,
	TPMN_SND_BLUEBLOB,
	TPMN_SND_BROWNBLOB,
	TPMN_SND_PLANT,
	TPMN_SND_SCORPION,
	TPMN_SND_SLIDERDEATH,
	TPMN_SND_BATFLEE,
	TPMN_SND_KEY,
	TPMN_SND_SLIDER,

	TPMN_NUM_SOUNDS,
};

struct tpmn_assets_t
{
	sd_bitmap_t backgrounds[7];

	sd_bitmap_t portal;
	sd_bitmap_t server;
	sd_bitmap_t arcs;
	sd_bitmap_t spikygreen;
	sd_bitmap_t blueblob;
	sd_bitmap_t brownblob;
	sd_bitmap_t penguin;
	sd_bitmap_t plant;
	sd_bitmap_t scorpion;
	sd_bitmap_t firedude;
	sd_bitmap_t bat;
	sd_bitmap_t gui_server_fixed;
	sd_bitmap_t gui_server_broken;
	sd_bitmap_t idle;

	sd_bitmap_t tiles;
	sd_bitmap_t hero;
	sd_bitmap_t whip;

	sd_bitmap_t dust_far;
	sd_bitmap_t dust_near;
	sd_bitmap_t flake;

	sd_fontv_t font;

	uint32_t anim_target[TPMN_MAX_TILE];
};

bool tpmn_assets_init(tpmn_assets_t& out_assets, std::vector<micron_sound_request_t>& out_sounds);
