#pragma once

#include "w32_d3d9_resource.h"

struct minyin_bitmap_t;

struct sd_bitmap_t;

enum struct w32_d3d9_fixed_function_mode_t
{
	SET,
	SET_NO_Z,
	ADD,
	SUB,
	ALPHA_BLEND,
	ALPHA_TEST,
	ALPHA_TEST_NO_Z,
};

struct w32_d3d9_softdraw_adapter_t : public w32_d3d9_resource_t
{
	void w32_d3d9_resource_on_destroy_device() override;

	explicit w32_d3d9_softdraw_adapter_t(const uint32_t in_alpha);

	const uint32_t ALPHA;
	uint32_t texture_aspect;
	::IDirect3DTexture9* texture;
};

void w32_d3d9_softdraw_present_2d(
	const sd_bitmap_t& in_canvas, const int32_t in_x, const int32_t in_y, const int32_t in_width, const int32_t in_height, const uint32_t in_color, const w32_d3d9_fixed_function_mode_t in_mode, const bool in_filter,
	w32_d3d9_softdraw_adapter_t& in_adapter);

void w32_d3d9_chunky_present_2d(
	const minyin_bitmap_t& in_canvas, const int32_t in_x, const int32_t in_y, const int32_t in_width, const int32_t in_height, const uint32_t in_color, const w32_d3d9_fixed_function_mode_t in_mode, const bool in_filter,
	w32_d3d9_softdraw_adapter_t& in_adapter);

::IDirect3DStateBlock9* w32_d3d9_state_block_begin(const w32_d3d9_fixed_function_mode_t in_mode, const ::DWORD in_cull, const ::DWORD in_fvf, const bool in_filter, ::IDirect3DTexture9* in_texture);
