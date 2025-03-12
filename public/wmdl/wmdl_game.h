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

bool wmdl_game_init(micron_t& out_micron, wmdl_game_t& out_game, std::vector<micron_sound_request_t>& out_sounds);
bool wmdl_game_tick(micron_t& out_micron, wmdl_game_t& out_game);
void wmdl_game_shutdown(wmdl_game_t& out_game);
