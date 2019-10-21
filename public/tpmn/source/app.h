#pragma once

#include "../../dx9/app.h"
#include "../../dx9/softdraw_adapter.h"
#include "model.h"
#include "assets.h"
#include "controller.h"

namespace tpmn
{
	struct app_t : public dx9::app_i
	{
		//dx9::app_i
		bool is_fixed_tick_rate() const override { return true; }
		float seconds_per_fixed_tick() const override { return TIME_PER_TICK; }
		bool is_device_acceptable(const ::D3DCAPS9& caps, const ::D3DFORMAT adapter_format, const ::D3DFORMAT back_buffer_format, const bool windowed) const override;
		bool modify_device_settings(const ::D3DCAPS9& caps, dx9::device_settings_t& device_settings) override;
		void frame_move(const double now, const float elapsed) override;
		bool frame_render(const double now, const float elapsed) override;
		::LRESULT msg_proc(const HWND window, const UINT message, const WPARAM wparam, const LPARAM lparam, bool* no_further_processing) override;

		//specific
		explicit app_t();

		model_t _model;
		assets_t _assets;
		controller_t _controller;
		dx9::softdraw_adapter_t _video;
	};

	bool app_init(app_t& app);
	void app_shutdown(app_t& app);
}
