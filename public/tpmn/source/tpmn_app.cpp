#include "stdafx.h"
#include "tpmn_app.h"

#include "../../dx9/state.h"
#include "../../dx9/input.h"
#include "../../dx9/sound_stream.h"

#define APPDATA_PATH "nornware/tpmn"
#define ASSET_HUB "hubb.tpmn"

bool tpmn_app_t::is_device_acceptable(const ::D3DCAPS9&, const ::D3DFORMAT, const ::D3DFORMAT, const bool) const
{
	//don't really need to require fancy stuff here, using FF user primitive...

	return true;
}

bool tpmn_app_t::modify_device_settings(const ::D3DCAPS9& caps, dx9::device_settings_t& device_settings)
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
			dx9::display_switching_to_ref_warning();
	}

	device_settings.present_parameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;	//no vsync

	return true;
}

void tpmn_app_t::frame_move(const double, const float)
{
	//if not foreground, don't run
	if (!dx9::state.m_active)
		return;

	//sample input from the os and cache it
	dx9::input_poll(dx9::hwnd());

	{
		//tick the simulation
		const tpmn::model_update_t update_result = model_update(_model);

		//if a level change is queued, do that
		if (update_result.world_to_load)
		{
			if (model_load_world(update_result.world_to_load, false, _model))
				controller_on_load_new_world(_assets, _controller);
		}
		//otherwise potentially "do effects"
		else
		{
			if (tpmn::bit_hero_whip & update_result.bits)
				tpmn_sound_play(_assets, TPMN_SND_HEROWHIP);

			if (tpmn::bit_hero_jump & update_result.bits)
				tpmn_sound_play(_assets, TPMN_SND_HEROJUMP01 + uint32_t(tpmn::random_unit() * 5));

			if (tpmn::bit_hero_landed & update_result.bits)
				tpmn_sound_play(_assets, TPMN_SND_HEROLAND01 + uint32_t(tpmn::random_unit() * 4));

			if (tpmn::bit_back_to_checkpoint & update_result.bits)
				tpmn_sound_play(_assets, TPMN_SND_SPAWN);

			if (tpmn::bit_bat_flee & update_result.bits)
				tpmn_sound_play(_assets, TPMN_SND_BATFLEE);

			if (tpmn::bit_hero_die & update_result.bits)
			{
				controller_death_create(
					&_assets.hero,
					_model.hero.x, _model.hero.y,
					tpmn::HERO_WIDTH, tpmn::HERO_HEIGHT,
					_model.hero.right_bit ? tpmn::HERO_WIDTH * 2 : 0, 5 * tpmn::HERO_HEIGHT,
					_controller
				);
				tpmn_sound_play(_assets, TPMN_SND_HERODIE01 + uint32_t(tpmn::random_unit() * 3));
			}

			if (tpmn::bit_fixed_server & update_result.bits)
				tpmn_sound_play(_assets, TPMN_SND_FIXSERVER);

			if (tpmn::bit_checkpoint & update_result.bits)
			{
				const uint32_t offset = tpmn::world_to_offset(_model.hero.x, _model.hero.y);
				if (offset != _controller.last_checkpoint)
				{
					tpmn_sound_play(_assets, TPMN_SND_CHECKPOINT);
					_controller.last_checkpoint = offset;
				}
			}

			if (tpmn::bit_key & update_result.bits)
				tpmn_sound_play(_assets, TPMN_SND_KEY);

			if (tpmn::bit_roller_die & update_result.bits)
			{
				switch (update_result.roller_type)
				{
				case tpmn::LOGIC_INDEX_SPIKYGREEN:
					controller_death_create(&_assets.spikygreen, update_result.roller_x, update_result.roller_y, _assets.spikygreen.width, _assets.spikygreen.width, 0, 0, _controller);
					tpmn_sound_play(_assets, TPMN_SND_SPIKYGREEN);
					break;

				case tpmn::LOGIC_INDEX_BLUEBLOB:
					controller_death_create(&_assets.blueblob, update_result.roller_x, update_result.roller_y, tpmn::BLOB_FRAME_ASPECT, tpmn::BLOB_FRAME_ASPECT, 0, 0, _controller);
					tpmn_sound_play(_assets, TPMN_SND_BLUEBLOB);
					break;

				case tpmn::LOGIC_INDEX_BROWNBLOB:
					controller_death_create(&_assets.brownblob, update_result.roller_x, update_result.roller_y, tpmn::BLOB_FRAME_ASPECT, tpmn::BLOB_FRAME_ASPECT, 0, 0, _controller);
					tpmn_sound_play(_assets, TPMN_SND_BROWNBLOB);
					break;
				}
			}

			if (tpmn::bit_plant_die & update_result.bits)
			{
				switch (update_result.plant_type)
				{
				case tpmn::LOGIC_INDEX_PLANT:
					controller_death_create(&_assets.plant, update_result.plant_x, update_result.plant_y, tpmn::PLANT_FRAME_ASPECT, tpmn::PLANT_FRAME_ASPECT, 0, 0, _controller);
					tpmn_sound_play(_assets, TPMN_SND_PLANT);
					break;

				case tpmn::LOGIC_INDEX_SCORPION:
					controller_death_create(&_assets.scorpion, update_result.plant_x, update_result.plant_y, _assets.scorpion.width / 2, _assets.scorpion.height / 6, 0, _assets.scorpion.height / 6 * 4, _controller);
					tpmn_sound_play(_assets, TPMN_SND_SCORPION);
					break;
				}
			}

			if (tpmn::bit_slider_die & update_result.bits)
			{
				controller_death_create(
					&_assets.penguin,
					update_result.slider_x, update_result.slider_y,
					_assets.penguin.width / 2, _assets.penguin.height / 2,
					update_result.slider_speed > 0 ? _assets.penguin.width / 2 : 0, 0, _controller
				);
				tpmn_sound_play(_assets, TPMN_SND_SLIDERDEATH);
			}

			if (tpmn::bit_slider_impulse & update_result.bits)
				tpmn_sound_play(_assets, TPMN_SND_SLIDER);
		}
	}

	//single pass imgui, render to the canvas to be pushed to the gpu in frame_render()
	switch (controller_input_output(_assets, _model, _controller))
	{
	case tpmn::app_event_t::exit_application:
		dx9::shutdown(0);
		break;

	case tpmn::app_event_t::start_new_game:
		if (model_load_world(ASSET_HUB, true, _model))
			controller_on_load_new_world(_assets, _controller);
		break;
	}
}

bool tpmn_app_t::frame_render(const double, const float)
{
	if (dx9::state.m_active)
	{
		::HRESULT hr;

		// Render the scene
		if (SUCCEEDED(dx9::state.m_d3d_device->BeginScene()))
		{
			// Clear the render target and the zbuffer
			VERIFY(dx9::state.m_d3d_device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.f, 0));

			{
				const int32_t render_mul = __min(dx9::state.m_backbuffer_surface_desc.Width / _controller.canvas.width, dx9::state.m_backbuffer_surface_desc.Height / _controller.canvas.height);
				const int32_t size_x = _controller.canvas.width * render_mul;
				const int32_t size_y = _controller.canvas.height * render_mul;

				softdraw_adapter_present_2d(
					_controller.canvas,
					int32_t((dx9::state.m_backbuffer_surface_desc.Width - size_x) / 2),
					int32_t((dx9::state.m_backbuffer_surface_desc.Height - size_y) / 2),
					size_x,
					size_y,
					UINT32_MAX,
					dx9::ff_mode_t::set,
					false,
					_video);
			}

			VERIFY(dx9::state.m_d3d_device->EndScene());
		}
	}

	return dx9::state.m_active;
}

::LRESULT tpmn_app_t::msg_proc(const HWND, const UINT, const WPARAM, const LPARAM, bool*)
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
	if (!model_init(app._model))
		return false;

	if (!tpmn_assets_init(app._assets))
		return false;

	for (uint32_t i = 0; i < tpmn::MAX_TILE; ++i)
		app._controller.current_tiles[i] = i;

	if (!bitmap_init(tpmn::CANVAS_WIDTH, tpmn::CANVAS_HEIGHT, 0, app._controller.canvas))
		return false;

	return true;
}

void tpmn_app_shutdown(tpmn_app_t& app)
{
	delete app._controller.music;
	tpmn_assets_cleanup(app._assets);
}
