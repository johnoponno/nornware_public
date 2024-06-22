#include "stdafx.h"
#include "win32_d3d9_app.h"

#include "../minyin/sd_bitmap.h"
#include "win32_d3d9_state.h"
#include "win32_d3d9_softdraw_adapter.h"
#include "win32_dsound_stream.h"

struct canvas_layout_t
{
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
};

static canvas_layout_t __canvas_layout(const int32_t in_width, const int32_t in_height)
{
	const int32_t RENDER_MUL = __min(win32_d3d9_state.m_backbuffer_surface_desc.Width / in_width, win32_d3d9_state.m_backbuffer_surface_desc.Height / in_height);
	canvas_layout_t result;

	result.width = in_width * RENDER_MUL;
	result.height = in_height * RENDER_MUL;

	result.x = ((int32_t)win32_d3d9_state.m_backbuffer_surface_desc.Width - result.width) / 2;
	result.y = ((int32_t)win32_d3d9_state.m_backbuffer_surface_desc.Height - result.height) / 2;

	return result;
}

static uint32_t __clear_color(const win32_d3d9_app_i* in_app)
{
	uint32_t result;

	static uint32_t last_frame_drops = 0;
	if (in_app->_win32_d3d9_app_frame_drops != last_frame_drops)
		result = 0xffff0000;
	else
		result = 0xff000000;
	last_frame_drops = in_app->_win32_d3d9_app_frame_drops;

	return result;
}

//public
//public
//public
//public

bool win32_d3d9_init(
	const char* in_title, const bool in_windowed, const int32_t in_width, const int32_t in_height,
	win32_d3d9_app_i& out_app)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Show the cursor and clip it when in full screen
	win32_d3d9_set_cursor_settings(false, true);

	// Initialize DXUT and create the desired Win32 window and Direct3D 
	// device for the application. Calling each of these functions is optional, but they
	// allow you to set several options which control the behavior of the framework.
	win32_d3d9_init();
	win32_d3d9_create_window(in_title);
	{
		const ::HRESULT err = win32_d3d9_create_device(D3DADAPTER_DEFAULT, in_windowed, in_width, in_height, &out_app);
		if (FAILED(err))
			return false;
	}

	return true;
}

//softdraw
//softdraw
//softdraw
//softdraw

bool win32_d3d9_softdraw_app_t::win32_d3d9_app_modify_device_settings(
	const ::D3DCAPS9& in_caps,
	win32_d3d9_device_settings_t& out_device_settings)
{
	// If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
	// then switch to SWVP.
	if ((in_caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
		in_caps.VertexShaderVersion < D3DVS_VERSION(1, 1))
	{
		out_device_settings.behavior_flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	// Debugging vertex shaders requires either REF or software vertex processing 
	// and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
	if (device_settings->DeviceType != D3DDEVTYPE_REF)
	{
		out_device_settings->BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
		out_device_settings->BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
		out_device_settings->BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
}
#endif
#ifdef DEBUG_PS
	out_device_settings->DeviceType = D3DDEVTYPE_REF;
#endif
	// For the first device created if its a REF device, optionally display a warning dialog box
	static bool s_bFirstTime = true;
	if (s_bFirstTime)
	{
		s_bFirstTime = false;
		if (D3DDEVTYPE_REF == out_device_settings.device_type)
			win32_d3d9_display_switching_to_ref_warning();
	}

	out_device_settings.present_parameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;	//no vsync

	return true;
}

::LRESULT win32_d3d9_softdraw_app_t::win32_d3d9_app_msg_proc(const HWND, const UINT, const WPARAM, const LPARAM, bool*)
{
	return 0;
}

void win32_d3d9_softdraw_app_t::win32_d3d9_app_frame_move(const double, const float)
{
	//if not foreground, don't run
	if (!win32_d3d9_state.m_active)
		return;

	//sample input from the os and cache it
#if 0
	win32_input_poll(win32_d3d9_hwnd());
#else
	{
		for (
			uint32_t key = 0;
			key < _countof(_input.keys);
			++key
			)
		{
			_input.keys[key].down_last = _input.keys[key].down_current;
			_input.keys[key].down_current = 0 != ::GetAsyncKeyState(key);
		}

		{
			::POINT cursor_position;
			::GetCursorPos(&cursor_position);
			::ScreenToClient(win32_d3d9_hwnd(), &cursor_position);

			//__state.cursor_movement_x = cursor_position.x - __state.cursor_pos_x;
			//__state.cursor_movement_y = cursor_position.y - __state.cursor_pos_y;
			_input.screen_cursor_x = cursor_position.x;
			_input.screen_cursor_y = cursor_position.y;
		}

		/*
		if (__state.mouse_wheel.event_consumed)
		{
			__state.mouse_wheel.delta = 0;
			__state.mouse_wheel.event_consumed = false;
		}
		*/
	}
#endif

	{
		const canvas_layout_t LAYOUT = __canvas_layout(REF_CANVAS.width, REF_CANVAS.height);

		float cpx = (float)_input.screen_cursor_x;
		cpx -= LAYOUT.x;
		cpx /= (float)LAYOUT.width;
		cpx *= (float)REF_CANVAS.width;

		float cpy = (float)_input.screen_cursor_y;
		cpy -= LAYOUT.y;
		cpy /= (float)LAYOUT.height;
		cpy *= (float)REF_CANVAS.height;

		_input.canvas_cursor_x = (int32_t)cpx;
		_input.canvas_cursor_y = (int32_t)cpy;

		if (cpx < 0.f)
			--_input.canvas_cursor_x;

		if (cpy < 0.f)
			--_input.canvas_cursor_y;
	}

	if (!win32_d3d9_softdraw_app_tick(_input, _sound_container))
		win32_d3d9_shutdown(0);
}

bool win32_d3d9_softdraw_app_t::win32_d3d9_app_frame_render(const double, const float)
{
	if (win32_d3d9_state.m_active)
	{
		::HRESULT hr;

		// Render the scene
		if (SUCCEEDED(win32_d3d9_state.m_d3d_device->BeginScene()))
		{
			// Clear the render target and the zbuffer
			VERIFY(win32_d3d9_state.m_d3d_device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, __clear_color(this), 1.f, 0));

			{
				const canvas_layout_t LAYOUT = __canvas_layout(REF_CANVAS.width, REF_CANVAS.height);

				win32_d3d9_softdraw_present_2d(
					REF_CANVAS,
					LAYOUT.x,
					LAYOUT.y,
					LAYOUT.width,
					LAYOUT.height,
					UINT32_MAX,
					win32_d3d9_fixed_function_mode_t::SET,
					false,
					_video_adapter
				);
			}

			VERIFY(win32_d3d9_state.m_d3d_device->EndScene());
		}
	}

	return win32_d3d9_state.m_active;
}

bool win32_d3d9_softdraw_app_t::win32_d3d9_app_is_fixed_tick_rate() const
{
	return true;
}

float win32_d3d9_softdraw_app_t::win32_d3d9_app_seconds_per_fixed_tick() const
{
	return SECONDS_PER_FIXED_TICK;
}

bool win32_d3d9_softdraw_app_t::win32_d3d9_app_is_device_acceptable(const ::D3DCAPS9& caps, const ::D3DFORMAT adapter_format, const ::D3DFORMAT back_buffer_format, const bool windowed) const
{
	caps;
	adapter_format;
	back_buffer_format;
	windowed;
	return true;
}

win32_d3d9_softdraw_app_t::win32_d3d9_softdraw_app_t(const float in_seconds_per_fixed_tick, const sd_bitmap_t& in_canvas, const uint32_t in_num_sounds)
	:SECONDS_PER_FIXED_TICK(in_seconds_per_fixed_tick)
	, REF_CANVAS(in_canvas)
	, _video_adapter(false)
	, _sound_container(in_num_sounds)
{
	_music_file = nullptr;
	_music_stream = nullptr;
}

bool win32_d3d9_softdraw_app_t::win32_d3d9_softdraw_app_init_audio(const std::vector<minyin_sound_request_t>& in_sound_requests)
{
	if (!_sound_engine.init(win32_d3d9_hwnd()))
		return false;

	if (!_sound_container.init())
		return false;

	for (const minyin_sound_request_t& SR : in_sound_requests)
	{
		if (!_sound_container.add_sound(_sound_engine, SR.asset, SR.id, 1))
			return false;
	}

	return true;
}

void win32_d3d9_softdraw_app_t::win32_d3d9_softdraw_app_cleanup_audio()
{
	_music_file = nullptr;
	delete _music_stream;
	_music_stream = nullptr;
	_sound_container.cleanup();
	_sound_engine.cleanup();
}

void win32_d3d9_softdraw_app_t::win32_d3d9_softdraw_app_handle_music_request(const char* in_music_request)
{
	if (_music_file != in_music_request || !_music_stream)
	{
		if (_music_stream)
		{
			delete _music_stream;
			_music_stream = nullptr;
		}

		_music_stream = win32_dsound_stream_create(_sound_engine, in_music_request);
		if (_music_stream)
			_music_stream->play(true, 0.f, 1.f);

		_music_file = in_music_request;
	}

	if (_music_stream)
		_music_stream->update(_win32_d3d9_app_frame_moves * SECONDS_PER_FIXED_TICK, 1.f);
}

//chunky
//chunky
//chunky
//chunky

bool win32_d3d9_chunky_app_t::win32_d3d9_app_modify_device_settings(
	const ::D3DCAPS9& in_caps,
	win32_d3d9_device_settings_t& out_device_settings)
{
	// If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
	// then switch to SWVP.
	if ((in_caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
		in_caps.VertexShaderVersion < D3DVS_VERSION(1, 1))
	{
		out_device_settings.behavior_flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	// Debugging vertex shaders requires either REF or software vertex processing 
	// and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
	if (device_settings->DeviceType != D3DDEVTYPE_REF)
	{
		out_device_settings->BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
		out_device_settings->BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
		out_device_settings->BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
#endif
#ifdef DEBUG_PS
	out_device_settings->DeviceType = D3DDEVTYPE_REF;
#endif
	// For the first device created if its a REF device, optionally display a warning dialog box
	static bool s_bFirstTime = true;
	if (s_bFirstTime)
	{
		s_bFirstTime = false;
		if (D3DDEVTYPE_REF == out_device_settings.device_type)
			win32_d3d9_display_switching_to_ref_warning();
	}

	out_device_settings.present_parameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;	//no vsync

	return true;
}

::LRESULT win32_d3d9_chunky_app_t::win32_d3d9_app_msg_proc(const HWND, const UINT, const WPARAM, const LPARAM, bool*)
{
	return 0;
}

void win32_d3d9_chunky_app_t::win32_d3d9_app_frame_move(const double, const float)
{
	//if not foreground, don't run
	if (!win32_d3d9_state.m_active)
		return;

	//sample input from the os and cache it
#if 0
	win32_input_poll(win32_d3d9_hwnd());
#else
	{
		for (
			uint32_t key = 0;
			key < _countof(_input.keys);
			++key
			)
		{
			_input.keys[key].down_last = _input.keys[key].down_current;
			_input.keys[key].down_current = 0 != ::GetAsyncKeyState(key);
		}

		{
			::POINT cursor_position;
			::GetCursorPos(&cursor_position);
			::ScreenToClient(win32_d3d9_hwnd(), &cursor_position);

			//__state.cursor_movement_x = cursor_position.x - __state.cursor_pos_x;
			//__state.cursor_movement_y = cursor_position.y - __state.cursor_pos_y;
			_input.screen_cursor_x = cursor_position.x;
			_input.screen_cursor_y = cursor_position.y;
		}

		/*
		if (__state.mouse_wheel.event_consumed)
		{
			__state.mouse_wheel.delta = 0;
			__state.mouse_wheel.event_consumed = false;
		}
		*/
	}
#endif

	{
		const canvas_layout_t LAYOUT = __canvas_layout(REF_CANVAS.width, REF_CANVAS.height);

		float cpx = (float)_input.screen_cursor_x;
		cpx -= LAYOUT.x;
		cpx /= (float)LAYOUT.width;
		cpx *= (float)REF_CANVAS.width;

		float cpy = (float)_input.screen_cursor_y;
		cpy -= LAYOUT.y;
		cpy /= (float)LAYOUT.height;
		cpy *= (float)REF_CANVAS.height;

		_input.canvas_cursor_x = (int32_t)cpx;
		_input.canvas_cursor_y = (int32_t)cpy;

		if (cpx < 0.f)
			--_input.canvas_cursor_x;

		if (cpy < 0.f)
			--_input.canvas_cursor_y;
	}

	if (!win32_d3d9_chunky_app_tick(_input, _sound_container))
		win32_d3d9_shutdown(0);
}

bool win32_d3d9_chunky_app_t::win32_d3d9_app_frame_render(const double, const float)
{
	if (win32_d3d9_state.m_active)
	{
		::HRESULT hr;

		// Render the scene
		if (SUCCEEDED(win32_d3d9_state.m_d3d_device->BeginScene()))
		{
			// Clear the render target and the zbuffer
			VERIFY(win32_d3d9_state.m_d3d_device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, __clear_color(this), 1.f, 0));

			{
				const canvas_layout_t LAYOUT = __canvas_layout(REF_CANVAS.width, REF_CANVAS.height);

				win32_d3d9_chunky_present_2d(
					REF_CANVAS,
					LAYOUT.x,
					LAYOUT.y,
					LAYOUT.width,
					LAYOUT.height,
					UINT32_MAX,
					win32_d3d9_fixed_function_mode_t::SET,
					false,
					_video_adapter
				);
			}

			VERIFY(win32_d3d9_state.m_d3d_device->EndScene());
		}
	}

	return win32_d3d9_state.m_active;
}

bool win32_d3d9_chunky_app_t::win32_d3d9_app_is_fixed_tick_rate() const
{
	return true;
}

float win32_d3d9_chunky_app_t::win32_d3d9_app_seconds_per_fixed_tick() const
{
	return SECONDS_PER_FIXED_TICK;
}

bool win32_d3d9_chunky_app_t::win32_d3d9_app_is_device_acceptable(const ::D3DCAPS9& caps, const ::D3DFORMAT adapter_format, const ::D3DFORMAT back_buffer_format, const bool windowed) const
{
	caps;
	adapter_format;
	back_buffer_format;
	windowed;
	return true;
}

win32_d3d9_chunky_app_t::win32_d3d9_chunky_app_t(const float in_seconds_per_fixed_tick, const minyin_bitmap_t& in_canvas, const uint32_t in_num_sounds)
	:SECONDS_PER_FIXED_TICK(in_seconds_per_fixed_tick)
	, REF_CANVAS(in_canvas)
	, _video_adapter(false)
	, _sound_container(in_num_sounds)
{
	_music_file = nullptr;
	_music_stream = nullptr;
}

bool win32_d3d9_chunky_app_t::win32_d3d9_chunky_app_init_audio(const std::vector<minyin_sound_request_t>& in_sound_requests)
{
	if (!_sound_engine.init(win32_d3d9_hwnd()))
		return false;

	if (!_sound_container.init())
		return false;

	for (const minyin_sound_request_t& SR : in_sound_requests)
	{
		if (!_sound_container.add_sound(_sound_engine, SR.asset, SR.id, 1))
			return false;
	}

	return true;
}

void win32_d3d9_chunky_app_t::win32_d3d9_chunky_app_cleanup_audio()
{
	_music_file = nullptr;
	delete _music_stream;
	_music_stream = nullptr;
	_sound_container.cleanup();
	_sound_engine.cleanup();
}

void win32_d3d9_chunky_app_t::win32_d3d9_chunky_app_handle_music_request(const char* in_music_request)
{
	if (_music_file != in_music_request || !_music_stream)
	{
		if (_music_stream)
		{
			delete _music_stream;
			_music_stream = nullptr;
		}

		_music_stream = win32_dsound_stream_create(_sound_engine, in_music_request);
		if (_music_stream)
			_music_stream->play(true, 0.f, 1.f);

		_music_file = in_music_request;
	}

	if (_music_stream)
		_music_stream->update(_win32_d3d9_app_frame_moves * SECONDS_PER_FIXED_TICK, 1.f);
}
