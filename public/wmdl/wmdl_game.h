#pragma once

#include "wmdl_model.h"
#include "wmdl_assets.h"
#include "wmdl_controller.h"

struct wmdl_game_t
{
	wmdl_model_t model;
	wmdl_assets_t assets;
	wmdl_controller_t controller;
};

bool wmdl_game_init(wmdl_game_t& out_game, std::vector<minyin_sound_request_t>& out_sounds);
bool wmdl_game_tick(
	const minyin_input_t& in_minyin,
	wmdl_game_t& out_game, std::vector<uint32_t>& out_sound_plays, const char*& out_music_request);
