#pragma once

#include "win32_d3d9_resource.h"

struct sd_bitmap_t;

enum struct win32_d3d9_fixed_function_mode_t
{
	SET,
	SET_NO_Z,
	ADD,
	SUB,
	ALPHA_BLEND,
	ALPHA_TEST,
	ALPHA_TEST_NO_Z,
};

struct win32_d3d9_softdraw_adapter_t : public win32_d3d9_resource_t
{
	void win32_d3d9_resource_on_destroy_device() override;

	explicit win32_d3d9_softdraw_adapter_t(const uint32_t in_alpha);

	const uint32_t ALPHA;
	uint32_t texture_aspect;
	::IDirect3DTexture9* texture;
};

void win32_d3d9_softdraw_adapter_present_2d(
	const sd_bitmap_t& in_canvas, const int32_t in_x, const int32_t in_y, const int32_t in_width, const int32_t in_height, const uint32_t in_color, const win32_d3d9_fixed_function_mode_t in_mode, const bool in_filter,
	win32_d3d9_softdraw_adapter_t& in_adapter);
