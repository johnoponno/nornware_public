#include "stdafx.h"
#include "State.h"

namespace dx9
{
	state_t::state_t()
	{
		::ZeroMemory(this, sizeof(*this));
		m_auto_change_adapter = true;
		m_active = true;
	}

	state_t::~state_t()
	{
		assert(nullptr == hwnd());
		assert(nullptr == m_d3d_device);
		assert(nullptr == m_d3d_enumeration);
		assert(nullptr == m_d3d);
	}

	::HWND hwnd()
	{
		return is_windowed() ? state.m_hwnd.device_windowed  : state.m_hwnd.device_fullscreen;
	}

	bool is_windowed()
	{
		if (state.m_device_settings)
			return (state.m_device_settings->present_parameters.Windowed != 0);

		return false;
	}

	bool is_rendering_paused()
	{
		return state.m_pause_rendering_count > 0;
	}

	vec2f_t get_backbuffer_size()
	{
		return{ (const float)state.m_backbuffer_surface_desc.Width, (const float)state.m_backbuffer_surface_desc.Height };
	}

	float get_backbuffer_aspect_ratio()
	{
		const vec2f_t BB = get_backbuffer_size();
		return BB.x / BB.y;
	}

	void toggle_wireframe()
	{
		if (!state.m_d3d_device)
			return;

		::DWORD fill_mode = 0;
		state.m_d3d_device->GetRenderState(D3DRS_FILLMODE, &fill_mode);
		switch (fill_mode)
		{
		default:
			assert(0);
			break;

		case D3DFILL_WIREFRAME:
			state.m_d3d_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
			break;

		case D3DFILL_SOLID:
			state.m_d3d_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			break;
		}
	}

	bool is_wireframe()
	{
		if (!state.m_d3d_device)
			return false;

		::DWORD fill_mode = 0;
		state.m_d3d_device->GetRenderState(D3DRS_FILLMODE, &fill_mode);
		switch (fill_mode)
		{
		default:
			assert(0);
			return false;

		case D3DFILL_WIREFRAME:
			return true;

		case D3DFILL_SOLID:
			return false;
		}
	}

	state_t state;
}