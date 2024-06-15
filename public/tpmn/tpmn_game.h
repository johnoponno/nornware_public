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

bool tpmn_game_init(tpmn_game_t& out_game, std::vector<minyin_sound_request_t>& out_sounds);
bool tpmn_game_tick(
	const minyin_t& in_minyin,
	tpmn_game_t& out_game, std::vector<uint32_t>& out_sound_plays, const char*& out_music_request);
void tpmn_game_shutdown(tpmn_game_t& out_game);
