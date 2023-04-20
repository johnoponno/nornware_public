#pragma once

#include "win32_d3d9_resource.h"

struct sd_bitmap_t;

namespace dx9
{
	struct texture_t;

	enum struct ff_mode_t
	{
		set,
		set_no_z,
		add,
		sub,
		alpha_blend,
		alpha_test,
		alpha_test_no_z,
	};

	struct softdraw_adapter_t : public win32_d3d9_resource_t
	{
		void win32_d3d9_resource_on_destroy_device() override;

		explicit softdraw_adapter_t(const uint32_t anAlpha);

		const uint32_t alpha;
		uint32_t texture_aspect;
		::IDirect3DTexture9* texture;
	};

	void softdraw_adapter_present_2d(const sd_bitmap_t& aCanvas, const int32_t aX, const int32_t aY, const int32_t aWidth, const int32_t aHeight, const uint32_t aColor, const ff_mode_t aMode, const bool aFilter, softdraw_adapter_t& a);
}
