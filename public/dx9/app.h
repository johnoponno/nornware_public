#pragma once

namespace dx9
{
	struct app_i
	{
		virtual bool modify_device_settings(const ::D3DCAPS9& caps, device_settings_t& device_settings) = 0;
		virtual void frame_move(const double now, const float elapsed) = 0;
		virtual bool frame_render(const double now, const float elapsed) = 0;
		virtual ::LRESULT msg_proc(const HWND window, const UINT message, const WPARAM wparam, const LPARAM lparam, bool* no_further_processing) = 0;
		virtual void do_off_tick_processing(const double now)
		{
			now;
		}
		virtual bool is_fixed_tick_rate() const = 0;
		virtual float seconds_per_fixed_tick() const = 0;
		virtual bool is_device_acceptable(const ::D3DCAPS9& caps, const ::D3DFORMAT adapter_format, const ::D3DFORMAT back_buffer_format, const bool windowed) const = 0;
		virtual uint32_t fixed_tick_background_sleep_milliseconds() const
		{
			return (uint32_t)(seconds_per_fixed_tick() * 500.f);
		}
		virtual bool pause_time_when_minimized() const
		{
			return true;
		}

		uint32_t _frame_drops;
		uint32_t _frame_moves;

	protected:

		explicit app_i()
		{
			_frame_drops = 0;
			_frame_moves = 0;
		}
	};
}