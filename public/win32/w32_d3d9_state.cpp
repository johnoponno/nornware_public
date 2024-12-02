#include "stdafx.h"
#include "w32_d3d9_state.h"

w32_d3d9_state_t::w32_d3d9_state_t()
{
	::ZeroMemory(this, sizeof(*this));
	m_auto_change_adapter = true;
	m_active = true;
}

w32_d3d9_state_t::~w32_d3d9_state_t()
{
	assert(nullptr == w32_d3d9_hwnd());
	assert(nullptr == m_d3d_device);
	assert(nullptr == m_d3d_enumeration);
	assert(nullptr == m_d3d);
}

::HWND w32_d3d9_hwnd()
{
	return w32_d3d9_is_windowed() ? w32_d3d9_state.m_hwnd.device_windowed : w32_d3d9_state.m_hwnd.device_fullscreen;
}

bool w32_d3d9_is_windowed()
{
	if (w32_d3d9_state.m_device_settings)
		return (w32_d3d9_state.m_device_settings->present_parameters.Windowed != 0);

	return false;
}

bool w32_d3d9_is_rendering_paused()
{
	return w32_d3d9_state.m_pause_rendering_count > 0;
}

/*
vec2f_t w32_d3d9_get_backbuffer_size()
{
	return{ (const float)w32_d3d9_state.m_backbuffer_surface_desc.Width, (const float)w32_d3d9_state.m_backbuffer_surface_desc.Height };
}

float w32_d3d9_get_backbuffer_aspect_ratio()
{
	const vec2f_t BB = get_backbuffer_size();
	return BB.x / BB.y;
}

void w32_d3d9_toggle_wireframe()
{
	if (!w32_d3d9_state.m_d3d_device)
		return;

	::DWORD fill_mode = 0;
	w32_d3d9_state.m_d3d_device->GetRenderState(D3DRS_FILLMODE, &fill_mode);
	switch (fill_mode)
	{
	default:
		assert(0);
		break;

	case D3DFILL_WIREFRAME:
		w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		break;

	case D3DFILL_SOLID:
		w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		break;
	}
}

bool w32_d3d9_is_wireframe()
{
	if (!w32_d3d9_state.m_d3d_device)
		return false;

	::DWORD fill_mode = 0;
	w32_d3d9_state.m_d3d_device->GetRenderState(D3DRS_FILLMODE, &fill_mode);
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
*/

w32_d3d9_state_t w32_d3d9_state;
