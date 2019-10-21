#pragma once

#include "../../softdraw/Bitmap.h"
#include "../../softdraw/fontv.h"
#include "../../dx9/sound_engine.h"
#include "../../dx9/sound_container.h"
#include "model.h"

namespace tpmn
{
	enum
	{
		snd_spawn,
		snd_fixserver,
		snd_herowhip,
		snd_herojump01,
		snd_herojump02,
		snd_herojump03,
		snd_herojump04,
		snd_herojump05,
		snd_heroland01,
		snd_heroland02,
		snd_heroland03,
		snd_heroland04,
		snd_herodie01,
		snd_herodie02,
		snd_herodie03,
		snd_checkpoint,
		snd_spikygreen,
		snd_blueblob,
		snd_brownblob,
		snd_plant,
		snd_scorpion,
		snd_sliderdeath,
		snd_batflee,
		snd_key,
		snd_slider,

		num_sounds,
	};

	struct assets_t
	{
		explicit assets_t();

		softdraw::bitmap_t myBackgrounds[7];

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

		uint32_t anim_target[MAX_TILE];

		sound::engine_t engine;
		sound::container_t container;
	};

	bool assets_init(assets_t& assets);
	void assets_reload(assets_t& assets);
	void assets_cleanup(assets_t& assets);
	void sound_play(const assets_t& assets, const uint32_t id);

	extern const uint16_t TITLE_COLOR;
	extern const uint16_t TEXT_COLOR;
}
