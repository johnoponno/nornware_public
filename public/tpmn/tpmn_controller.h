#pragma once

#include "../micron/sd_bitmap.h"
#include "tpmn_model.h"

struct micron_t;

struct tpmn_assets_t;

#define TPMN_CANVAS_WIDTH 600
#define TPMN_CANVAS_HEIGHT 240
#define TPMN_TEXT_SPACING 13
#define TPMN_HERO_WIDTH 16
#define TPMN_HERO_HEIGHT 24
#define TPMN_BLOB_FRAME_ASPECT 24
#define TPMN_PLANT_FRAME_ASPECT 32

enum struct tpmn_app_event_t
{
	NOTHING,
	EXIT_APPLICATION,
	START_NEW_GAME,
};

struct tpmn_snowflake_t
{
	float x;
	float y;
	float sx;
	float sy;
	uint32_t t;
};

struct tpmn_death_t
{
	const sd_bitmap_t* bm;
	float x;
	float y;
	float s;
	int32_t w;
	int32_t h;
	int32_t src_x;
	int32_t src_y;
};

struct tpmn_controller_t
{
	uint32_t play_menu;
	uint32_t last_checkpoint;
	uint32_t current_tiles[TPMN_MAX_TILE];
	tpmn_snowflake_t flakes[32];
	tpmn_death_t deaths[8];
	uint32_t num_deaths;
	float tile_anim_tick;
	float credits_start_time;
	sd_bitmap_t canvas;
};

tpmn_app_event_t tpmn_controller_tick(
	const tpmn_assets_t& in_assets,
	tpmn_model_t& out_model, tpmn_controller_t& out_controller, micron_t& out_micron);

void tpmn_controller_on_load_new_world(tpmn_controller_t& out_controller, std::vector<uint32_t>& out_sound_plays);

void tpmn_controller_death_create(
	const sd_bitmap_t* aBitmap, const float aX, const float aY, const int32_t aWidth, const int32_t aHeight, const int32_t aSrcX, const int32_t aSrcY,
	tpmn_controller_t& controller);
