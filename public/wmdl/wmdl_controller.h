#pragma once

#include "../minyin/minyin.h"
#include "wmdl_model.h"

struct wmdl_assets_t;

#define WMDL_TEXT_SPACING 13
#define WMDL_HERO_WIDTH 16
#define WMDL_HERO_HEIGHT 24
#define WMDL_BLOB_FRAME_ASPECT 24
#define WMDL_PLANT_FRAME_ASPECT 32

enum struct wmdl_app_event_t
{
	NOTHING,
	EXIT_APPLICATION,
	START_NEW_GAME,
};

struct wmdl_snowflake_t
{
	float x;
	float y;
	float sx;
	float sy;
	uint32_t t;
};

struct wmdl_death_t
{
	const minyin_bitmap_t* bm;
	float x;
	float y;
	float s;
	int32_t w;
	int32_t h;
	int32_t src_x;
	int32_t src_y;
};

struct wmdl_controller_t
{
	uint32_t play_menu;
	uint32_t last_checkpoint;
	uint32_t current_tiles[WMDL_MAX_TILE];
	wmdl_snowflake_t flakes[32];
	wmdl_death_t deaths[8];
	uint32_t num_deaths;
	float tile_anim_tick;
	float credits_start_time;
	minyin_bitmap_t canvas;
};

wmdl_app_event_t wmdl_controller_tick(
	const minyin_input_t& in_minyin, const wmdl_assets_t& in_assets,
	wmdl_model_t& out_model, wmdl_controller_t& out_controller, const char*& out_music_request);

void wmdl_controller_on_load_new_world(wmdl_controller_t& out_controller, std::vector<uint32_t>& out_sound_plays);

void wmdl_controller_death_create(
	const minyin_bitmap_t* aBitmap, const float aX, const float aY, const int32_t aWidth, const int32_t aHeight, const int32_t aSrcX, const int32_t aSrcY,
	wmdl_controller_t& controller);
