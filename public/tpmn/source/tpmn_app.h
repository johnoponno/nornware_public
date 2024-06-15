#pragma once

#include "../../win32/win32_d3d9_app.h"
#include "tpmn_model.h"
#include "tpmn_assets.h"
#include "tpmn_controller.h"

struct tpmn_app_t : public win32_d3d9_simpleapp_i
{
	bool win32_d3d9_simpleapp_tick(const win32_cursor_position_t& in_cursor_position) override;

	explicit tpmn_app_t();
	bool init();
	void shutdown();

	tpmn_model_t _model;
	tpmn_assets_t _assets;
	tpmn_controller_t _controller;
};
