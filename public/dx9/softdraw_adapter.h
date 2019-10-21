#pragma once

#include "d3d_resource.h"

namespace softdraw
{
	struct bitmap_t;
}

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

	struct softdraw_adapter_t : public d3d_resource_t
	{
		void on_destroy_device() override;

		explicit softdraw_adapter_t(const uint32_t anAlpha);

		const uint32_t alpha;
		uint32_t texture_aspect;
		::IDirect3DTexture9* texture;
	};

	void softdraw_adapter_present_2d(const softdraw::bitmap_t& aCanvas, const int32_t aX, const int32_t aY, const int32_t aWidth, const int32_t aHeight, const uint32_t aColor, const ff_mode_t aMode, const bool aFilter, softdraw_adapter_t& a);
}
