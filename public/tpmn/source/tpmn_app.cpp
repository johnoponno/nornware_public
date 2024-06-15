#include "stdafx.h"
#include "tpmn_app.h"

#include "../../win32/win32_dsound_stream.h"

bool tpmn_app_t::win32_d3d9_simpleapp_tick(const win32_cursor_position_t& in_cursor_position)
{
	{
		//tick the simulation
		const tpmn_events_t UPDATE_RESULT = tpmn_model_update(_model);

		//if a level change is queued, do that
		if (UPDATE_RESULT.world_to_load)
		{
			if (tpmn_model_load_world(UPDATE_RESULT.world_to_load, false, _model))
				tpmn_controller_on_load_new_world(_assets, _controller);
		}
		//otherwise potentially "do effects"
		else
		{
			if (TPMN_EVENT_BIT_HERO_WHIP & UPDATE_RESULT.bits)
				tpmn_sound_play(_assets, TPMN_SND_HEROWHIP);

			if (TPMN_EVENT_BIT_HERO_JUMP & UPDATE_RESULT.bits)
				tpmn_sound_play(_assets, TPMN_SND_HEROJUMP01 + uint32_t(tpmn_random_unit() * 5));

			if (TPMN_EVENT_BIT_HERO_LANDED & UPDATE_RESULT.bits)
				tpmn_sound_play(_assets, TPMN_SND_HEROLAND01 + uint32_t(tpmn_random_unit() * 4));

			if (TPMN_EVENT_BIT_BACK_TO_CHECKPOINT & UPDATE_RESULT.bits)
				tpmn_sound_play(_assets, TPMN_SND_SPAWN);

			if (TPMN_EVENT_BIT_BAT_FLEE & UPDATE_RESULT.bits)
				tpmn_sound_play(_assets, TPMN_SND_BATFLEE);

			if (TPMN_EVENT_BIT_HERO_DIE & UPDATE_RESULT.bits)
			{
				tpmn_controller_death_create(
					&_assets.hero,
					_model.hero.x, _model.hero.y,
					TPMN_HERO_WIDTH, TPMN_HERO_HEIGHT,
					_model.hero.right_bit ? TPMN_HERO_WIDTH * 2 : 0, 5 * TPMN_HERO_HEIGHT,
					_controller
				);
				tpmn_sound_play(_assets, TPMN_SND_HERODIE01 + uint32_t(tpmn_random_unit() * 3));
			}

			if (TPMN_EVENT_BIT_FIXED_SERVER & UPDATE_RESULT.bits)
				tpmn_sound_play(_assets, TPMN_SND_FIXSERVER);

			if (TPMN_EVENT_BIT_CHECKPOINT & UPDATE_RESULT.bits)
			{
				const uint32_t offset = tpmn_world_to_offset(_model.hero.x, _model.hero.y);
				if (offset != _controller.last_checkpoint)
				{
					tpmn_sound_play(_assets, TPMN_SND_CHECKPOINT);
					_controller.last_checkpoint = offset;
				}
			}

			if (TPMN_EVENT_BIT_KEY & UPDATE_RESULT.bits)
				tpmn_sound_play(_assets, TPMN_SND_KEY);

			if (TPMN_EVENT_BIT_ROLLER_DIE & UPDATE_RESULT.bits)
			{
				switch (UPDATE_RESULT.roller_type)
				{
				case TPMN_LOGIC_INDEX_SPIKYGREEN:
					tpmn_controller_death_create(&_assets.spikygreen, UPDATE_RESULT.roller_x, UPDATE_RESULT.roller_y, _assets.spikygreen.width, _assets.spikygreen.width, 0, 0, _controller);
					tpmn_sound_play(_assets, TPMN_SND_SPIKYGREEN);
					break;

				case TPMN_LOGIC_INDEX_BLUEBLOB:
					tpmn_controller_death_create(&_assets.blueblob, UPDATE_RESULT.roller_x, UPDATE_RESULT.roller_y, TPMN_BLOB_FRAME_ASPECT, TPMN_BLOB_FRAME_ASPECT, 0, 0, _controller);
					tpmn_sound_play(_assets, TPMN_SND_BLUEBLOB);
					break;

				case TPMN_LOGIC_INDEX_BROWNBLOB:
					tpmn_controller_death_create(&_assets.brownblob, UPDATE_RESULT.roller_x, UPDATE_RESULT.roller_y, TPMN_BLOB_FRAME_ASPECT, TPMN_BLOB_FRAME_ASPECT, 0, 0, _controller);
					tpmn_sound_play(_assets, TPMN_SND_BROWNBLOB);
					break;
				}
			}

			if (TPMN_EVENT_BIT_PLANT_DIE & UPDATE_RESULT.bits)
			{
				switch (UPDATE_RESULT.plant_type)
				{
				case TPMN_LOGIC_INDEX_PLANT:
					tpmn_controller_death_create(&_assets.plant, UPDATE_RESULT.plant_x, UPDATE_RESULT.plant_y, TPMN_PLANT_FRAME_ASPECT, TPMN_PLANT_FRAME_ASPECT, 0, 0, _controller);
					tpmn_sound_play(_assets, TPMN_SND_PLANT);
					break;

				case TPMN_LOGIC_INDEX_SCORPION:
					tpmn_controller_death_create(&_assets.scorpion, UPDATE_RESULT.plant_x, UPDATE_RESULT.plant_y, _assets.scorpion.width / 2, _assets.scorpion.height / 6, 0, _assets.scorpion.height / 6 * 4, _controller);
					tpmn_sound_play(_assets, TPMN_SND_SCORPION);
					break;
				}
			}

			if (TPMN_EVENT_BIT_SLIDER_DIE & UPDATE_RESULT.bits)
			{
				tpmn_controller_death_create(
					&_assets.penguin,
					UPDATE_RESULT.slider_x, UPDATE_RESULT.slider_y,
					_assets.penguin.width / 2, _assets.penguin.height / 2,
					UPDATE_RESULT.slider_speed > 0 ? _assets.penguin.width / 2 : 0, 0, _controller
				);
				tpmn_sound_play(_assets, TPMN_SND_SLIDERDEATH);
			}

			if (TPMN_EVENT_BIT_SLIDER_IMPULSE & UPDATE_RESULT.bits)
				tpmn_sound_play(_assets, TPMN_SND_SLIDER);
		}
	}

	//single pass imgui, render to the canvas to be pushed to the gpu in win32_d3d9_app_frame_render()
	switch (tpmn_controller_input_output(in_cursor_position, _assets, _model, _controller))
	{
	case tpmn_app_event_t::EXIT_APPLICATION:
		return false;

	case tpmn_app_event_t::START_NEW_GAME:
		if (tpmn_model_load_world("hubb.tpmn", true, _model))
			tpmn_controller_on_load_new_world(_assets, _controller);
		break;
	}

	return true;
}

tpmn_app_t::tpmn_app_t()
	:win32_d3d9_simpleapp_i(TPMN_SECONDS_PER_TICK, _controller.canvas)
{
}

bool tpmn_app_t::init()
{
	if (!tpmn_model_init(_model))
		return false;

	if (!tpmn_assets_init(_assets))
		return false;

	for (uint32_t i = 0; i < TPMN_MAX_TILE; ++i)
		_controller.current_tiles[i] = i;

	if (!sd_bitmap_init(TPMN_CANVAS_WIDTH, TPMN_CANVAS_HEIGHT, 0, _controller.canvas))
		return false;

	return true;
}

void tpmn_app_t::shutdown()
{
	delete _controller.music;
	tpmn_assets_cleanup(_assets);
}
