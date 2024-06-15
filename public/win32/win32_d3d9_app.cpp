#include "stdafx.h"
#include "win32_d3d9_app.h"

#include "../softdraw/sd_bitmap.h"
#include "win32_d3d9_state.h"
#include "win32_input.h"
#include "win32_d3d9_softdraw_adapter.h"

struct canvas_layout_t
{
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
};

static canvas_layout_t __canvas_layout(const sd_bitmap_t& canvas)
{
	const int32_t RENDER_MUL = __min(win32_d3d9_state.m_backbuffer_surface_desc.Width / canvas.width, win32_d3d9_state.m_backbuffer_surface_desc.Height / canvas.height);
	canvas_layout_t result;

	result.width = canvas.width * RENDER_MUL;
	result.height = canvas.height * RENDER_MUL;

	result.x = ((int32_t)win32_d3d9_state.m_backbuffer_surface_desc.Width - result.width) / 2;
	result.y = ((int32_t)win32_d3d9_state.m_backbuffer_surface_desc.Height - result.height) / 2;

	return result;
}

static win32_cursor_position_t __canvas_cursor_pos(const sd_bitmap_t& in_canvas)
{
	const canvas_layout_t LAYOUT = __canvas_layout(in_canvas);
	const win32_cursor_position_t CURSOR_POSITION = win32_mouse_cursor_position();

	float cpx = (float)CURSOR_POSITION.x;
	cpx -= LAYOUT.x;
	cpx /= (float)LAYOUT.width;
	cpx *= (float)in_canvas.width;

	float cpy = (float)CURSOR_POSITION.y;
	cpy -= LAYOUT.y;
	cpy /= (float)LAYOUT.height;
	cpy *= (float)in_canvas.height;

	win32_cursor_position_t i{ (int32_t)cpx, (int32_t)cpy };

	if (cpx < 0.f)
		--i.x;

	if (cpy < 0.f)
		--i.y;

	return i;
}

//public
//public
//public
//public

bool win32_d3d9_simpleapp_i::win32_d3d9_app_modify_device_settings(const ::D3DCAPS9& caps, win32_d3d9_device_settings_t& device_settings)
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

::LRESULT win32_d3d9_simpleapp_i::win32_d3d9_app_msg_proc(const HWND, const UINT, const WPARAM, const LPARAM, bool*)
{
	return 0;
}

void win32_d3d9_simpleapp_i::win32_d3d9_app_frame_move(const double, const float)
{
	//if not foreground, don't run
	if (!win32_d3d9_state.m_active)
		return;

	//sample input from the os and cache it
	win32_input_poll(win32_d3d9_hwnd());

	if (!win32_d3d9_simpleapp_tick(__canvas_cursor_pos(REF_CANVAS)))
		win32_d3d9_shutdown(0);
}

bool win32_d3d9_simpleapp_i::win32_d3d9_app_frame_render(const double, const float)
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
				const canvas_layout_t LAYOUT = __canvas_layout(REF_CANVAS);

				win32_d3d9_softdraw_adapter_present_2d(
					REF_CANVAS,
					LAYOUT.x,
					LAYOUT.y,
					LAYOUT.width,
					LAYOUT.height,
					UINT32_MAX,
					win32_d3d9_fixed_function_mode_t::SET,
					false,
					_adapter
				);
			}

			VERIFY(win32_d3d9_state.m_d3d_device->EndScene());
		}
	}

	return win32_d3d9_state.m_active;
}

bool win32_d3d9_simpleapp_i::win32_d3d9_app_is_fixed_tick_rate() const
{
	return true;
}

float win32_d3d9_simpleapp_i::win32_d3d9_app_seconds_per_fixed_tick() const
{
	return SECONDS_PER_FIXED_TICK;
}

bool win32_d3d9_simpleapp_i::win32_d3d9_app_is_device_acceptable(const ::D3DCAPS9& caps, const ::D3DFORMAT adapter_format, const ::D3DFORMAT back_buffer_format, const bool windowed) const
{
	caps;
	adapter_format;
	back_buffer_format;
	windowed;
	return true;
}

win32_d3d9_simpleapp_i::win32_d3d9_simpleapp_i(const float in_seconds_per_fixed_tick, const sd_bitmap_t& in_canvas)
	:SECONDS_PER_FIXED_TICK(in_seconds_per_fixed_tick)
	, REF_CANVAS(in_canvas)
	, _adapter(false)
{
}

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
