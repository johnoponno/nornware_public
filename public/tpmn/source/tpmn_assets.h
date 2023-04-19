#pragma once

#include "../../softdraw/Bitmap.h"
#include "../../softdraw/fontv.h"
#include "../../dx9/sound_engine.h"
#include "../../dx9/sound_container.h"
#include "tpmn_model.h"

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
	explicit tpmn_assets_t();

	softdraw::bitmap_t backgrounds[7];

	softdraw::bitmap_t portal;
	softdraw::bitmap_t server;
	softdraw::bitmap_t arcs;
	softdraw::bitmap_t spikygreen;
	softdraw::bitmap_t blueblob;
	softdraw::bitmap_t brownblob;
	softdraw::bitmap_t penguin;
	softdraw::bitmap_t plant;
	softdraw::bitmap_t scorpion;
	softdraw::bitmap_t firedude;
	softdraw::bitmap_t bat;
	softdraw::bitmap_t gui_server_fixed;
	softdraw::bitmap_t gui_server_broken;
	softdraw::bitmap_t idle;

	softdraw::bitmap_t tiles;
	softdraw::bitmap_t hero;
	softdraw::bitmap_t whip;

	softdraw::bitmap_t dust_far;
	softdraw::bitmap_t dust_near;
	softdraw::bitmap_t flake;

	softdraw::fontv_t font;

	uint32_t anim_target[TPMN_MAX_TILE];

	sound::engine_t engine;
	sound::container_t container;
};

bool tpmn_assets_init(tpmn_assets_t& assets);
void tpmn_assets_reload(tpmn_assets_t& assets);
void tpmn_assets_cleanup(tpmn_assets_t& assets);
void tpmn_sound_play(const tpmn_assets_t& assets, const uint32_t id);

extern const uint16_t TPMN_TITLE_COLOR;
extern const uint16_t TPMN_TEXT_COLOR;
