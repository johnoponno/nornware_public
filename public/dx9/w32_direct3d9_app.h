#pragma once

struct win32_direct3d9_app_i
{
	virtual bool win32_direct3d9_app_modify_device_settings(const ::D3DCAPS9& caps, dx9::device_settings_t& device_settings) = 0;
	virtual void win32_direct3d9_app_frame_move(const double now, const float elapsed) = 0;
	virtual bool win32_direct3d9_app_frame_render(const double now, const float elapsed) = 0;
	virtual ::LRESULT win32_direct3d9_app_msg_proc(const HWND window, const UINT message, const WPARAM wparam, const LPARAM lparam, bool* no_further_processing) = 0;
	virtual void win32_direct3d9_app_do_off_tick_processing(const double now)
	{
		now;
	}
	virtual bool win32_direct3d9_app_is_fixed_tick_rate() const = 0;
	virtual float win32_direct3d9_app_seconds_per_fixed_tick() const = 0;
	virtual bool win32_direct3d9_app_is_device_acceptable(const ::D3DCAPS9& caps, const ::D3DFORMAT adapter_format, const ::D3DFORMAT back_buffer_format, const bool windowed) const = 0;
	virtual uint32_t win32_direct3d9_app_fixed_tick_background_sleep_milliseconds() const
	{
		return (uint32_t)(win32_direct3d9_app_seconds_per_fixed_tick() * 500.f);
	}
	virtual bool pause_time_when_minimized() const
	{
		return true;
	}

	uint32_t _frame_drops;
	uint32_t _frame_moves;

protected:

	explicit win32_direct3d9_app_i()
	{
		_frame_drops = 0;
		_frame_moves = 0;
	}
};
