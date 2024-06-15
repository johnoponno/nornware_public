#pragma once

#include "../softdraw/minyin.h"
#include "win32_d3d9_softdraw_adapter.h"
#include "win32_dsound_engine.h"
#include "win32_dsound_container.h"

struct sd_bitmap_t;

struct win32_d3d9_device_settings_t;

struct win32_d3d9_app_i
{
	virtual bool win32_d3d9_app_modify_device_settings(const ::D3DCAPS9& caps, win32_d3d9_device_settings_t& device_settings) = 0;
	virtual void win32_d3d9_app_frame_move(const double now, const float elapsed) = 0;
	virtual bool win32_d3d9_app_frame_render(const double now, const float elapsed) = 0;
	virtual ::LRESULT win32_d3d9_app_msg_proc(const HWND window, const UINT message, const WPARAM wparam, const LPARAM lparam, bool* no_further_processing) = 0;
	virtual void win32_d3d9_app_do_off_tick_processing(const double now)
	{
		now;
	}
	virtual bool win32_d3d9_app_is_fixed_tick_rate() const = 0;
	virtual float win32_d3d9_app_seconds_per_fixed_tick() const = 0;
	virtual bool win32_d3d9_app_is_device_acceptable(const ::D3DCAPS9& caps, const ::D3DFORMAT adapter_format, const ::D3DFORMAT back_buffer_format, const bool windowed) const = 0;
	virtual uint32_t win32_d3d9_app_fixed_tick_background_sleep_milliseconds() const
	{
		return (uint32_t)(win32_d3d9_app_seconds_per_fixed_tick() * 500.f);
	}
	virtual bool win32_d3d9_app_pause_time_when_minimized() const
	{
		return true;
	}

	uint32_t _frame_drops;
	uint32_t _frame_moves;

protected:

	explicit win32_d3d9_app_i()
	{
		_frame_drops = 0;
		_frame_moves = 0;
	}
};

//simplified / fixed tick version
struct win32_d3d9_simpleapp_i : public win32_d3d9_app_i
{
	bool win32_d3d9_app_modify_device_settings(const ::D3DCAPS9& caps, win32_d3d9_device_settings_t& device_settings) override;
	::LRESULT win32_d3d9_app_msg_proc(const HWND window, const UINT message, const WPARAM wparam, const LPARAM lparam, bool* no_further_processing) override;
	void win32_d3d9_app_frame_move(const double now, const float elapsed) override;
	bool win32_d3d9_app_frame_render(const double now, const float elapsed) override;

	bool win32_d3d9_app_is_fixed_tick_rate() const override;
	float win32_d3d9_app_seconds_per_fixed_tick() const override;
	bool win32_d3d9_app_is_device_acceptable(const ::D3DCAPS9& caps, const ::D3DFORMAT adapter_format, const ::D3DFORMAT back_buffer_format, const bool windowed) const override;

	explicit win32_d3d9_simpleapp_i(const float in_seconds_per_fixed_tick, const sd_bitmap_t& in_canvas, const uint32_t in_num_sounds);
	bool win32_d3d9_simpleapp_init_audio(const std::vector<minyin_sound_request_t>& in_sound_requests);
	void win32_d3d9_simpleapp_cleanup_audio();
	void win32_d3d9_simpleapp_handle_music_request(const char* in_music_request);
	virtual bool win32_d3d9_simpleapp_tick(const minyin_t& in_minyin) = 0;

protected:

	win32_dsound_container_t _sound_container;

private:

	const float SECONDS_PER_FIXED_TICK;
	const sd_bitmap_t& REF_CANVAS;
	win32_d3d9_softdraw_adapter_t _video_adapter;
	win32_dsound_engine_t _sound_engine;
	const char* _music_file;
	win32_dsound_stream_t* _music_stream;
	minyin_t _minyin;
};

//simplifies winmain
bool win32_d3d9_init(
	const char* in_title, const bool in_windowed, const int32_t in_width, const int32_t in_height,
	win32_d3d9_app_i& out_app);
