#pragma once

#include "tpmn_model.h"
#include "tpmn_assets.h"
#include "tpmn_controller.h"

struct tpmn_game_t
{
	tpmn_model_t model;
	tpmn_assets_t assets;
	tpmn_controller_t controller;
};

bool tpmn_game_init(micron_t& out_micron, tpmn_game_t& out_game);
bool tpmn_game_tick(micron_t& out_micron, tpmn_game_t& out_game);
