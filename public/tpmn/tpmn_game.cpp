#include "stdafx.h"
#include "tpmn_game.h"

//#include "../../win32/win32_dsound_stream.h"

bool tpmn_game_init(tpmn_game_t& out_game, std::vector<minyin_sound_request_t>& out_sounds)
{
	if (!tpmn_model_init(out_game.model))
		return false;

	if (!tpmn_assets_init(out_game.assets, out_sounds))
		return false;

	for (uint32_t i = 0; i < TPMN_MAX_TILE; ++i)
		out_game.controller.current_tiles[i] = i;

	if (!sd_bitmap_init(TPMN_CANVAS_WIDTH, TPMN_CANVAS_HEIGHT, 0, out_game.controller.canvas))
		return false;

	return true;
}

bool tpmn_game_tick(
	const minyin_t& in_minyin,
	tpmn_game_t& out_game, std::vector<uint32_t>& out_sound_plays, const char*& out_music_request)
{
	{
		//tick the simulation
		const tpmn_events_t UPDATE_RESULT = tpmn_model_update(out_game.model);

		//if a level change is queued, do that
		if (UPDATE_RESULT.world_to_load)
		{
			if (tpmn_model_load_world(UPDATE_RESULT.world_to_load, false, out_game.model))
				tpmn_controller_on_load_new_world(out_game.controller, out_sound_plays);
		}
		//otherwise potentially "do effects"
		else
		{
			if (TPMN_EVENT_BIT_HERO_WHIP & UPDATE_RESULT.bits)
				out_sound_plays.push_back(TPMN_SND_HEROWHIP);

			if (TPMN_EVENT_BIT_HERO_JUMP & UPDATE_RESULT.bits)
				out_sound_plays.push_back(TPMN_SND_HEROJUMP01 + uint32_t(tpmn_random_unit() * 5));

			if (TPMN_EVENT_BIT_HERO_LANDED & UPDATE_RESULT.bits)
				out_sound_plays.push_back(TPMN_SND_HEROLAND01 + uint32_t(tpmn_random_unit() * 4));

			if (TPMN_EVENT_BIT_BACK_TO_CHECKPOINT & UPDATE_RESULT.bits)
				out_sound_plays.push_back(TPMN_SND_SPAWN);

			if (TPMN_EVENT_BIT_BAT_FLEE & UPDATE_RESULT.bits)
				out_sound_plays.push_back(TPMN_SND_BATFLEE);

			if (TPMN_EVENT_BIT_HERO_DIE & UPDATE_RESULT.bits)
			{
				tpmn_controller_death_create(
					&out_game.assets.hero,
					out_game.model.hero.x, out_game.model.hero.y,
					TPMN_HERO_WIDTH, TPMN_HERO_HEIGHT,
					out_game.model.hero.right_bit ? TPMN_HERO_WIDTH * 2 : 0, 5 * TPMN_HERO_HEIGHT,
					out_game.controller
				);
				out_sound_plays.push_back(TPMN_SND_HERODIE01 + uint32_t(tpmn_random_unit() * 3));
			}

			if (TPMN_EVENT_BIT_FIXED_SERVER & UPDATE_RESULT.bits)
				out_sound_plays.push_back(TPMN_SND_FIXSERVER);

			if (TPMN_EVENT_BIT_CHECKPOINT & UPDATE_RESULT.bits)
			{
				const uint32_t offset = tpmn_world_to_offset(out_game.model.hero.x, out_game.model.hero.y);
				if (offset != out_game.controller.last_checkpoint)
				{
					out_sound_plays.push_back(TPMN_SND_CHECKPOINT);
					out_game.controller.last_checkpoint = offset;
				}
			}

			if (TPMN_EVENT_BIT_KEY & UPDATE_RESULT.bits)
				out_sound_plays.push_back(TPMN_SND_KEY);

			if (TPMN_EVENT_BIT_ROLLER_DIE & UPDATE_RESULT.bits)
			{
				switch (UPDATE_RESULT.roller_type)
				{
				case TPMN_LOGIC_INDEX_SPIKYGREEN:
					tpmn_controller_death_create(&out_game.assets.spikygreen, UPDATE_RESULT.roller_x, UPDATE_RESULT.roller_y, out_game.assets.spikygreen.width, out_game.assets.spikygreen.width, 0, 0, out_game.controller);
					out_sound_plays.push_back(TPMN_SND_SPIKYGREEN);
					break;

				case TPMN_LOGIC_INDEX_BLUEBLOB:
					tpmn_controller_death_create(&out_game.assets.blueblob, UPDATE_RESULT.roller_x, UPDATE_RESULT.roller_y, TPMN_BLOB_FRAME_ASPECT, TPMN_BLOB_FRAME_ASPECT, 0, 0, out_game.controller);
					out_sound_plays.push_back(TPMN_SND_BLUEBLOB);
					break;

				case TPMN_LOGIC_INDEX_BROWNBLOB:
					tpmn_controller_death_create(&out_game.assets.brownblob, UPDATE_RESULT.roller_x, UPDATE_RESULT.roller_y, TPMN_BLOB_FRAME_ASPECT, TPMN_BLOB_FRAME_ASPECT, 0, 0, out_game.controller);
					out_sound_plays.push_back(TPMN_SND_BROWNBLOB);
					break;
				}
			}

			if (TPMN_EVENT_BIT_PLANT_DIE & UPDATE_RESULT.bits)
			{
				switch (UPDATE_RESULT.plant_type)
				{
				case TPMN_LOGIC_INDEX_PLANT:
					tpmn_controller_death_create(&out_game.assets.plant, UPDATE_RESULT.plant_x, UPDATE_RESULT.plant_y, TPMN_PLANT_FRAME_ASPECT, TPMN_PLANT_FRAME_ASPECT, 0, 0, out_game.controller);
					out_sound_plays.push_back(TPMN_SND_PLANT);
					break;

				case TPMN_LOGIC_INDEX_SCORPION:
					tpmn_controller_death_create(&out_game.assets.scorpion, UPDATE_RESULT.plant_x, UPDATE_RESULT.plant_y, out_game.assets.scorpion.width / 2, out_game.assets.scorpion.height / 6, 0, out_game.assets.scorpion.height / 6 * 4, out_game.controller);
					out_sound_plays.push_back(TPMN_SND_SCORPION);
					break;
				}
			}

			if (TPMN_EVENT_BIT_SLIDER_DIE & UPDATE_RESULT.bits)
			{
				tpmn_controller_death_create(
					&out_game.assets.penguin,
					UPDATE_RESULT.slider_x, UPDATE_RESULT.slider_y,
					out_game.assets.penguin.width / 2, out_game.assets.penguin.height / 2,
					UPDATE_RESULT.slider_speed > 0 ? out_game.assets.penguin.width / 2 : 0, 0, out_game.controller
				);
				out_sound_plays.push_back(TPMN_SND_SLIDERDEATH);
			}

			if (TPMN_EVENT_BIT_SLIDER_IMPULSE & UPDATE_RESULT.bits)
				out_sound_plays.push_back(TPMN_SND_SLIDER);
		}
	}

	//single pass imgui, render to the canvas to be pushed to the gpu in win32_d3d9_app_frame_render()
	switch (tpmn_controller_tick(in_minyin, out_game.assets, out_game.model, out_game.controller, out_music_request))
	{
	case tpmn_app_event_t::EXIT_APPLICATION:
		return false;

	case tpmn_app_event_t::START_NEW_GAME:
		if (tpmn_model_load_world("hubb.tpmn", true, out_game.model))
			tpmn_controller_on_load_new_world(out_game.controller, out_sound_plays);
		break;
	}

	return true;
}
