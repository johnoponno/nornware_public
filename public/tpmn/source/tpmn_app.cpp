#include "stdafx.h"
#include "tpmn_app.h"

#include "../../dx9/state.h"
#include "../../dx9/win32_input.h"
#include "../../dx9/sound_stream.h"

#define APPDATA_PATH "nornware/tpmn"
#define ASSET_HUB "hubb.tpmn"

bool tpmn_app_t::win32_d3d9_app_is_device_acceptable(const ::D3DCAPS9&, const ::D3DFORMAT, const ::D3DFORMAT, const bool) const
{
	//don't really need to require fancy stuff here, using FF user primitive...

	return true;
}

bool tpmn_app_t::win32_d3d9_app_modify_device_settings(const ::D3DCAPS9& caps, win32_d3d9_device_settings_t& device_settings)
{
	// If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
	// then switch to SWVP.
	if ((caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
		caps.VertexShaderVersion < D3DVS_VERSION(1, 1))
	{
		device_settings.behavior_flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	// Debugging vertex shaders requires either REF or software vertex processing 
	// and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
	if (device_settings->DeviceType != D3DDEVTYPE_REF)
	{
		device_settings->BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
		device_settings->BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
		device_settings->BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
#endif
#ifdef DEBUG_PS
	device_settings->DeviceType = D3DDEVTYPE_REF;
#endif
	// For the first device created if its a REF device, optionally display a warning dialog box
	static bool s_bFirstTime = true;
	if (s_bFirstTime)
	{
		s_bFirstTime = false;
		if (D3DDEVTYPE_REF == device_settings.device_type)
			win32_d3d9_display_switching_to_ref_warning();
	}

	device_settings.present_parameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;	//no vsync

	return true;
}

void tpmn_app_t::win32_d3d9_app_frame_move(const double, const float)
{
	//if not foreground, don't run
	if (!win32_d3d9_state.m_active)
		return;

	//sample input from the os and cache it
	win32_input_poll(win32_d3d9_hwnd());

	{
		//tick the simulation
		const tpmn_events_t update_result = tpmn_model_update(_model);

		//if a level change is queued, do that
		if (update_result.world_to_load)
		{
			if (tpmn_model_load_world(update_result.world_to_load, false, _model))
				tpmn_controller_on_load_new_world(_assets, _controller);
		}
		//otherwise potentially "do effects"
		else
		{
			if (EVENT_BIT_HERO_WHIP & update_result.bits)
				tpmn_sound_play(_assets, TPMN_SND_HEROWHIP);

			if (EVENT_BIT_HERO_JUMP & update_result.bits)
				tpmn_sound_play(_assets, TPMN_SND_HEROJUMP01 + uint32_t(tpmn_random_unit() * 5));

			if (EVENT_BIT_HERO_LANDED & update_result.bits)
				tpmn_sound_play(_assets, TPMN_SND_HEROLAND01 + uint32_t(tpmn_random_unit() * 4));

			if (EVENT_BIT_BACK_TO_CHECKPOINT & update_result.bits)
				tpmn_sound_play(_assets, TPMN_SND_SPAWN);

			if (EVENT_BIT_BAT_FLEE & update_result.bits)
				tpmn_sound_play(_assets, TPMN_SND_BATFLEE);

			if (EVENT_BIT_HERO_DIE & update_result.bits)
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

			if (EVENT_BIT_FIXED_SERVER & update_result.bits)
				tpmn_sound_play(_assets, TPMN_SND_FIXSERVER);

			if (EVENT_BIT_CHECKPOINT & update_result.bits)
			{
				const uint32_t offset = tpmn_world_to_offset(_model.hero.x, _model.hero.y);
				if (offset != _controller.last_checkpoint)
				{
					tpmn_sound_play(_assets, TPMN_SND_CHECKPOINT);
					_controller.last_checkpoint = offset;
				}
			}

			if (EVENT_BIT_KEY & update_result.bits)
				tpmn_sound_play(_assets, TPMN_SND_KEY);

			if (EVENT_BIT_ROLLER_DIE & update_result.bits)
			{
				switch (update_result.roller_type)
				{
				case TPMN_LOGIC_INDEX_SPIKYGREEN:
					tpmn_controller_death_create(&_assets.spikygreen, update_result.roller_x, update_result.roller_y, _assets.spikygreen.width, _assets.spikygreen.width, 0, 0, _controller);
					tpmn_sound_play(_assets, TPMN_SND_SPIKYGREEN);
					break;

				case TPMN_LOGIC_INDEX_BLUEBLOB:
					tpmn_controller_death_create(&_assets.blueblob, update_result.roller_x, update_result.roller_y, TPMN_BLOB_FRAME_ASPECT, TPMN_BLOB_FRAME_ASPECT, 0, 0, _controller);
					tpmn_sound_play(_assets, TPMN_SND_BLUEBLOB);
					break;

				case TPMN_LOGIC_INDEX_BROWNBLOB:
					tpmn_controller_death_create(&_assets.brownblob, update_result.roller_x, update_result.roller_y, TPMN_BLOB_FRAME_ASPECT, TPMN_BLOB_FRAME_ASPECT, 0, 0, _controller);
					tpmn_sound_play(_assets, TPMN_SND_BROWNBLOB);
					break;
				}
			}

			if (EVENT_BIT_PLANT_DIE & update_result.bits)
			{
				switch (update_result.plant_type)
				{
				case TPMN_LOGIC_INDEX_PLANT:
					tpmn_controller_death_create(&_assets.plant, update_result.plant_x, update_result.plant_y, TPMN_PLANT_FRAME_ASPECT, TPMN_PLANT_FRAME_ASPECT, 0, 0, _controller);
					tpmn_sound_play(_assets, TPMN_SND_PLANT);
					break;

				case TPMN_LOGIC_INDEX_SCORPION:
					tpmn_controller_death_create(&_assets.scorpion, update_result.plant_x, update_result.plant_y, _assets.scorpion.width / 2, _assets.scorpion.height / 6, 0, _assets.scorpion.height / 6 * 4, _controller);
					tpmn_sound_play(_assets, TPMN_SND_SCORPION);
					break;
				}
			}

			if (EVENT_BIT_SLIDER_DIE & update_result.bits)
			{
				tpmn_controller_death_create(
					&_assets.penguin,
					update_result.slider_x, update_result.slider_y,
					_assets.penguin.width / 2, _assets.penguin.height / 2,
					update_result.slider_speed > 0 ? _assets.penguin.width / 2 : 0, 0, _controller
				);
				tpmn_sound_play(_assets, TPMN_SND_SLIDERDEATH);
			}

			if (EVENT_BIT_SLIDER_IMPULSE & update_result.bits)
				tpmn_sound_play(_assets, TPMN_SND_SLIDER);
		}
	}

	//single pass imgui, render to the canvas to be pushed to the gpu in win32_d3d9_app_frame_render()
	switch (tpmn_controller_input_output(_assets, _model, _controller))
	{
	case tpmn::tpmn_app_event_t::EXIT_APPLICATION:
		win32_d3d9_shutdown(0);
		break;

	case tpmn::tpmn_app_event_t::START_NEW_GAME:
		if (tpmn_model_load_world(ASSET_HUB, true, _model))
			tpmn_controller_on_load_new_world(_assets, _controller);
		break;
	}
}

bool tpmn_app_t::win32_d3d9_app_frame_render(const double, const float)
{
	if (win32_d3d9_state.m_active)
	{
		::HRESULT hr;

		// Render the scene
		if (SUCCEEDED(win32_d3d9_state.m_d3d_device->BeginScene()))
		{
			// Clear the render target and the zbuffer
			VERIFY(win32_d3d9_state.m_d3d_device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.f, 0));

			{
				const int32_t RENDER_MUL = __min(win32_d3d9_state.m_backbuffer_surface_desc.Width / _controller.canvas.width, win32_d3d9_state.m_backbuffer_surface_desc.Height / _controller.canvas.height);
				const int32_t SIZE_X = _controller.canvas.width * RENDER_MUL;
				const int32_t SIZE_Y = _controller.canvas.height * RENDER_MUL;

				win32_d3d9_softdraw_adapter_present_2d(
					_controller.canvas,
					int32_t((win32_d3d9_state.m_backbuffer_surface_desc.Width - SIZE_X) / 2),
					int32_t((win32_d3d9_state.m_backbuffer_surface_desc.Height - SIZE_Y) / 2),
					SIZE_X,
					SIZE_Y,
					UINT32_MAX,
					win32_d3d9_fixed_function_mode_t::SET,
					false,
					_video
				);
			}

			VERIFY(win32_d3d9_state.m_d3d_device->EndScene());
		}
	}

	return win32_d3d9_state.m_active;
}

::LRESULT tpmn_app_t::win32_d3d9_app_msg_proc(const HWND, const UINT, const WPARAM, const LPARAM, bool*)
{
	return 0;
}

//specific
tpmn_app_t::tpmn_app_t()
	:_video(false)
{
}

bool tpmn_app_init(tpmn_app_t& app)
{
	if (!tpmn_model_init(app._model))
		return false;

	if (!tpmn_assets_init(app._assets))
		return false;

	for (uint32_t i = 0; i < TPMN_MAX_TILE; ++i)
		app._controller.current_tiles[i] = i;

	if (!sd_bitmap_init(TPMN_CANVAS_WIDTH, TPMN_CANVAS_HEIGHT, 0, app._controller.canvas))
		return false;

	return true;
}

void tpmn_app_shutdown(tpmn_app_t& app)
{
	delete app._controller.music;
	tpmn_assets_cleanup(app._assets);
}
