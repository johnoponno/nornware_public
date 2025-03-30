#include "stdafx.h"
#include "w32_d3d9_softdraw_adapter.h"

#include "w32_d3d9_state.h"

static void __copy(
	const uint16_t* in_palette,
	const uint8_t* in_canvas_pixels, const int32_t in_canvas_width, const int32_t in_canvas_height, 
	const ::D3DLOCKED_RECT& in_rect)
{
	const uint32_t PITCH = in_rect.Pitch / sizeof(uint16_t);
	uint16_t* dst_start = (uint16_t*)in_rect.pBits;

	for (
		int32_t y = 0; 
		y < in_canvas_height; 
		++y
		)
	{
		const uint8_t* SRC = in_canvas_pixels + y * in_canvas_width;
		uint16_t* dst = dst_start + y * PITCH;

		for (
			int32_t x = 0;
			x < in_canvas_width;
			++x
			)
		{
			*dst = in_palette[*SRC];
			++SRC;
			++dst;
		}
	}
}

static void __copy_alpha(
	const uint16_t* in_palette, const uint16_t in_key,
	const uint8_t* in_canvas_pixels, const int32_t in_canvas_width, const int32_t in_canvas_height,
	const ::D3DLOCKED_RECT& in_rect)
{
	const uint32_t PITCH = in_rect.Pitch / sizeof(uint16_t);
	uint16_t* dst_start = (uint16_t*)in_rect.pBits;

	for (
		int32_t y = 0; 
		y < in_canvas_height; 
		++y
		)
	{
		const uint8_t* SRC = in_canvas_pixels + y * in_canvas_width;
		uint16_t* dst = dst_start + y * PITCH;

		for (
			int32_t x = 0;
			x < in_canvas_width;
			++x
			)
		{
			if (in_palette[*SRC] != in_key)
				*dst = 0x8000 | in_palette[*SRC];	//alpha bit 15
			else
				*dst = in_palette[*SRC];
			++SRC;
			++dst;
		}
	}
}

//public
//public
//public
//public

void w32_d3d9_present_8bit(
	const uint16_t* in_palette, const uint16_t in_alpha_key,
	const uint8_t* in_canvas_pixels, const int32_t in_canvas_width, const int32_t in_canvas_height,
	const int32_t in_x, const int32_t in_y, const int32_t in_width, const int32_t in_height, const uint32_t in_color, const w32_d3d9_fixed_function_mode_t in_mode, const bool in_filter,
	w32_d3d9_softdraw_adapter_t& out_adapter)
{
	::HRESULT hr;

	//make sure the texture is created and big enough for the incoming canvas
	if (
		!out_adapter.texture ||
		out_adapter.texture_aspect < (uint32_t)in_canvas_width ||
		out_adapter.texture_aspect < (uint32_t)in_canvas_height
		)
	{
		//release any previous texture (in order to support incoming canvases of varying sizes we just use the biggest one we've seen)
		SAFE_RELEASE(out_adapter.texture);

		//we want in_adapter power of 2 aspect texture
		out_adapter.texture_aspect = 1;
		while (
			out_adapter.texture_aspect < (uint32_t)in_canvas_width ||
			out_adapter.texture_aspect < (uint32_t)in_canvas_height
			)
		{
			out_adapter.texture_aspect *= 2;
		}

		assert(w32_d3d9_state.m_d3d_device);

		//texture format depends on if we want alpha test support
		if (out_adapter.ALPHA)
		{
			VERIFY(w32_d3d9_state.m_d3d_device->CreateTexture(out_adapter.texture_aspect, out_adapter.texture_aspect, 1, 0, D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, &out_adapter.texture, nullptr));
		}
		else
		{
			VERIFY(w32_d3d9_state.m_d3d_device->CreateTexture(out_adapter.texture_aspect, out_adapter.texture_aspect, 1, 0, D3DFMT_R5G6B5, D3DPOOL_MANAGED, &out_adapter.texture, nullptr));
		}
	}

	//copy the canvas to the texture
	{
		::D3DLOCKED_RECT locked_rect;
		assert(out_adapter.texture);
		if (SUCCEEDED(out_adapter.texture->LockRect(0, &locked_rect, nullptr, D3DLOCK_DISCARD)))
		{
			if (out_adapter.ALPHA)
				__copy_alpha(in_palette, in_alpha_key, in_canvas_pixels, in_canvas_width, in_canvas_height, locked_rect);
			else
				__copy(in_palette, in_canvas_pixels, in_canvas_width, in_canvas_height, locked_rect);
			out_adapter.texture->UnlockRect(0);
		}
	}

	{
		//setup fixed function state in preparation for render
		::IDirect3DStateBlock9* state_block = w32_d3d9_state_block_begin(
			in_mode,
			::D3DCULL_CCW,
			D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1,
			in_filter,
			out_adapter.texture
		);

		//render the quad
		{
			constexpr uint16_t INDICES[6] =
			{
				0, 1, 2,
				2, 1, 3,
			};
			const float XF = (float)in_x;
			const float YF = (float)in_y;
			const float WIDTH = (float)in_width - .5f;
			const float HEIGHT = (float)in_height - .5f;
			const float U_MAX = (float)in_canvas_width / (float)out_adapter.texture_aspect;
			const float V_MAX = (float)in_canvas_height / (float)out_adapter.texture_aspect;
			const struct
			{
				float x;
				float y;
				float z;
				float rhw;
				uint32_t color;
				float u;
				float v;
			} VERTICES[4] =
			{
				{ XF + 0, YF + 0, 0.f, 1.f, in_color, 0.f, 0.f },
				{ XF + WIDTH, YF + 0, 0.f, 1.f, in_color, U_MAX, 0.f },
				{ XF + 0, YF + HEIGHT, 0.f, 1.f, in_color, 0.f, V_MAX },
				{ XF + WIDTH, YF + HEIGHT, 0.f, 1.f, in_color, U_MAX, V_MAX },
			};
			VERIFY(w32_d3d9_state.m_d3d_device->DrawIndexedPrimitiveUP(
				D3DPT_TRIANGLELIST, 
				0, 
				4, 
				2, 
				INDICES, 
				D3DFMT_INDEX16, 
				VERTICES, 
				sizeof(VERTICES[0]))
			);
		}

		//restore any state we changed
		VERIFY(state_block->Apply());
		SAFE_RELEASE(state_block);
	}
}
