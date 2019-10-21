#pragma once

#include "../../softdraw/Bitmap.h"
#include "model.h"

namespace sound
{
	struct stream_t;
}

namespace tpmn
{
	struct assets_t;

	enum
	{
		CANVAS_WIDTH = 600,
		CANVAS_HEIGHT = 240,

		TEXT_SPACING = 13,

		HERO_WIDTH = 16,
		HERO_HEIGHT = 24,

		BLOB_FRAME_ASPECT = 24,

		PLANT_FRAME_ASPECT = 32,
	};

	enum struct app_event_t
	{
		nothing,
		exit_application,
		start_new_game,
	};

	struct snowflake_t
	{
		float x;
		float y;
		float sx;
		float sy;
		uint32_t t;
	};

	struct cloud_t
	{
		float x;
		float y;
		float s;
		uint32_t t;
	};

	struct death_t
	{
		const softdraw::bitmap_t* bm;
		float x;
		float y;
		float s;
		int32_t w;
		int32_t h;
		int32_t src_x;
		int32_t src_y;
	};

	struct controller_t
	{
		uint32_t play_menu;
		uint32_t last_checkpoint;
		uint32_t track;
		sound::stream_t* music;
		uint32_t current_tiles[MAX_TILE];
		snowflake_t flakes[32];
		cloud_t clouds[8];
		death_t deaths[8];
		uint32_t num_deaths;
		float tile_anim_tick;
		float credits_start_time;
		softdraw::bitmap_t canvas;
	};

	app_event_t controller_input_output(const assets_t& assets, model_t& model, controller_t& controller);
	void controller_on_load_new_world(const assets_t& assets, controller_t& controller);
	void controller_death_create(const softdraw::bitmap_t* aBitmap, const float aX, const float aY, const int32_t aWidth, const int32_t aHeight, const int32_t aSrcX, const int32_t aSrcY, controller_t& controller);
}
