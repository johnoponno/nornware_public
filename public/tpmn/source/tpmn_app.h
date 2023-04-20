#pragma once

#include "../../win32/win32_d3d9_app.h"
#include "../../win32/win32_d3d9_softdraw_adapter.h"
#include "tpmn_model.h"
#include "tpmn_assets.h"
#include "tpmn_controller.h"

struct tpmn_app_t : public win32_d3d9_app_i
{
	//win32_d3d9_app_i
	bool win32_d3d9_app_is_fixed_tick_rate() const override { return true; }
	float win32_d3d9_app_seconds_per_fixed_tick() const override { return TPMN_SECONDS_PER_TICK; }
	bool win32_d3d9_app_is_device_acceptable(const ::D3DCAPS9& caps, const ::D3DFORMAT adapter_format, const ::D3DFORMAT back_buffer_format, const bool windowed) const override;
	bool win32_d3d9_app_modify_device_settings(const ::D3DCAPS9& caps, win32_d3d9_device_settings_t& device_settings) override;
	void win32_d3d9_app_frame_move(const double now, const float elapsed) override;
	bool win32_d3d9_app_frame_render(const double now, const float elapsed) override;
	::LRESULT win32_d3d9_app_msg_proc(const HWND window, const UINT message, const WPARAM wparam, const LPARAM lparam, bool* no_further_processing) override;

	//specific
	explicit tpmn_app_t();

	tpmn_model_t _model;
	tpmn_assets_t _assets;
	tpmn::tpmn_controller_t _controller;
	win32_d3d9_softdraw_adapter_t _video;
};

bool tpmn_app_init(tpmn_app_t& app);
void tpmn_app_shutdown(tpmn_app_t& app);
