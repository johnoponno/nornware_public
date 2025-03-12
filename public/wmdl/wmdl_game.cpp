#include "stdafx.h"
#include "wmdl_game.h"

bool wmdl_game_init(micron_t& out_micron, wmdl_game_t& out_game, std::vector<micron_sound_request_t>& out_sounds)
{
	if (!wmdl_model_init(out_game.model))
		return false;

	if (!wmdl_assets_init(out_micron, out_game.assets, out_sounds))
		return false;

	//setup default animation data
	for (
		uint32_t i = 0;
		i < WMDL_MAX_TILE;
		++i
		)
		out_game.controller.current_tiles[i] = i;

	return true;
}

bool wmdl_game_tick(micron_t& out_micron, wmdl_game_t& out_game)
{
	{
		//tick the simulation
		const wmdl_events_t UPDATE_RESULT = wmdl_model_update(out_game.model);

		//if a level change is queued, do that
		if (UPDATE_RESULT.world_to_load)
		{
			if (wmdl_model_load_world(UPDATE_RESULT.world_to_load, false, out_game.model))
				wmdl_controller_on_load_new_world(out_game.controller, out_micron);
		}
		//otherwise potentially "do effects"
		else
		{
			if (WMDL_EVENT_BIT_HERO_WHIP & UPDATE_RESULT.bits)
				out_micron.sound_plays.push_back(WMDL_SND_HEROWHIP);

			if (WMDL_EVENT_BIT_HERO_JUMP & UPDATE_RESULT.bits)
				out_micron.sound_plays.push_back(WMDL_SND_HEROJUMP01 + uint32_t(wmdl_random_unit() * 5));

			if (WMDL_EVENT_BIT_HERO_LANDED & UPDATE_RESULT.bits)
				out_micron.sound_plays.push_back(WMDL_SND_HEROLAND01 + uint32_t(wmdl_random_unit() * 4));

			if (WMDL_EVENT_BIT_BACK_TO_CHECKPOINT & UPDATE_RESULT.bits)
				out_micron.sound_plays.push_back(WMDL_SND_SPAWN);

			if (WMDL_EVENT_BIT_BAT_FLEE & UPDATE_RESULT.bits)
				out_micron.sound_plays.push_back(WMDL_SND_BATFLEE);

			if (WMDL_EVENT_BIT_HERO_DIE & UPDATE_RESULT.bits)
			{
				wmdl_controller_death_create(
					&out_game.assets.hero,
					out_game.model.hero.x, out_game.model.hero.y,
					WMDL_HERO_WIDTH, WMDL_HERO_HEIGHT,
					out_game.model.hero.right_bit ? WMDL_HERO_WIDTH * 2 : 0, 5 * WMDL_HERO_HEIGHT,
					out_game.controller
				);
				out_micron.sound_plays.push_back(WMDL_SND_HERODIE01 + uint32_t(wmdl_random_unit() * 3));
			}

			if (WMDL_EVENT_BIT_FIXED_SERVER & UPDATE_RESULT.bits)
				out_micron.sound_plays.push_back(WMDL_SND_FIXSERVER);

			if (WMDL_EVENT_BIT_CHECKPOINT & UPDATE_RESULT.bits)
			{
				const uint32_t offset = wmdl_world_to_offset(out_game.model.hero.x, out_game.model.hero.y);
				if (offset != out_game.controller.last_checkpoint)
				{
					out_micron.sound_plays.push_back(WMDL_SND_CHECKPOINT);
					out_game.controller.last_checkpoint = offset;
				}
			}

			if (WMDL_EVENT_BIT_KEY & UPDATE_RESULT.bits)
				out_micron.sound_plays.push_back(WMDL_SND_KEY);

			if (WMDL_EVENT_BIT_ROLLER_DIE & UPDATE_RESULT.bits)
			{
				switch (UPDATE_RESULT.roller_type)
				{
				case WMDL_LOGIC_INDEX_SPIKYGREEN:
					wmdl_controller_death_create(&out_game.assets.spikygreen, UPDATE_RESULT.roller_x, UPDATE_RESULT.roller_y, out_game.assets.spikygreen.width, out_game.assets.spikygreen.width, 0, 0, out_game.controller);
					out_micron.sound_plays.push_back(WMDL_SND_SPIKYGREEN);
					break;

				case WMDL_LOGIC_INDEX_BLUEBLOB:
					wmdl_controller_death_create(&out_game.assets.blueblob, UPDATE_RESULT.roller_x, UPDATE_RESULT.roller_y, WMDL_BLOB_FRAME_ASPECT, WMDL_BLOB_FRAME_ASPECT, 0, 0, out_game.controller);
					out_micron.sound_plays.push_back(WMDL_SND_BLUEBLOB);
					break;

				case WMDL_LOGIC_INDEX_BROWNBLOB:
					wmdl_controller_death_create(&out_game.assets.brownblob, UPDATE_RESULT.roller_x, UPDATE_RESULT.roller_y, WMDL_BLOB_FRAME_ASPECT, WMDL_BLOB_FRAME_ASPECT, 0, 0, out_game.controller);
					out_micron.sound_plays.push_back(WMDL_SND_BROWNBLOB);
					break;
				}
			}

			if (WMDL_EVENT_BIT_PLANT_DIE & UPDATE_RESULT.bits)
			{
				switch (UPDATE_RESULT.plant_type)
				{
				case WMDL_LOGIC_INDEX_PLANT:
					wmdl_controller_death_create(&out_game.assets.plant, UPDATE_RESULT.plant_x, UPDATE_RESULT.plant_y, WMDL_PLANT_FRAME_ASPECT, WMDL_PLANT_FRAME_ASPECT, 0, 0, out_game.controller);
					out_micron.sound_plays.push_back(WMDL_SND_PLANT);
					break;

				case WMDL_LOGIC_INDEX_SCORPION:
					wmdl_controller_death_create(&out_game.assets.scorpion, UPDATE_RESULT.plant_x, UPDATE_RESULT.plant_y, out_game.assets.scorpion.width / 2, out_game.assets.scorpion.height / 6, 0, out_game.assets.scorpion.height / 6 * 4, out_game.controller);
					out_micron.sound_plays.push_back(WMDL_SND_SCORPION);
					break;
				}
			}

			if (WMDL_EVENT_BIT_SLIDER_DIE & UPDATE_RESULT.bits)
			{
				wmdl_controller_death_create(
					&out_game.assets.penguin,
					UPDATE_RESULT.slider_x, UPDATE_RESULT.slider_y,
					out_game.assets.penguin.width / 2, out_game.assets.penguin.height / 2,
					UPDATE_RESULT.slider_speed > 0 ? out_game.assets.penguin.width / 2 : 0, 0, out_game.controller
				);
				out_micron.sound_plays.push_back(WMDL_SND_SLIDERDEATH);
			}

			if (WMDL_EVENT_BIT_SLIDER_IMPULSE & UPDATE_RESULT.bits)
				out_micron.sound_plays.push_back(WMDL_SND_SLIDER);
		}
	}

	//single pass imgui, render to the canvas to be pushed to the gpu in win32_d3d9_app_frame_render()
	switch (wmdl_controller_tick(out_game.assets, out_game.model, out_game.controller, out_micron))
	{
	case wmdl_app_event_t::EXIT_APPLICATION:
		return false;

	case wmdl_app_event_t::START_NEW_GAME:
		if (wmdl_model_load_world("hubb.tpmn", true, out_game.model))
			wmdl_controller_on_load_new_world(out_game.controller, out_micron);
		break;
	}

	return true;
}

void wmdl_game_shutdown(wmdl_game_t& out_game)
{
	wmdl_assets_cleanup(out_game.assets);
}
