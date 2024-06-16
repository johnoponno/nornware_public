#include "stdafx.h"
#include "win32_d3d9_softdraw_adapter.h"

#include "../minyin/sd_bitmap.h"
#include "win32_d3d9_state.h"

static void __copy(const sd_bitmap_t& in_canvas, const ::D3DLOCKED_RECT& in_rect)
{
	const uint32_t PITCH = in_rect.Pitch / sizeof(uint16_t);
	uint16_t* dst_start = (uint16_t*)in_rect.pBits;

	for (int32_t y = 0; y < in_canvas.height; ++y)
	{
		int32_t x = in_canvas.width;
		uint16_t* dst = dst_start + y * PITCH;
		const uint16_t* SRC = in_canvas.pixels + y * in_canvas.width;

		while (x)
		{
			*dst = *SRC;
			++SRC;
			++dst;
			--x;
		}
	}
}

static void __copy_alpha(const sd_bitmap_t& in_canvas, const ::D3DLOCKED_RECT& in_rect)
{
	const uint32_t PITCH = in_rect.Pitch / sizeof(uint16_t);
	uint16_t* dst_start = (uint16_t*)in_rect.pBits;

	for (int32_t y = 0; y < in_canvas.height; ++y)
	{
		int32_t x = in_canvas.width;
		uint16_t* dst = dst_start + y * PITCH;
		const uint16_t* SRC = in_canvas.pixels + y * in_canvas.width;

		while (x)
		{
			if (*SRC != sd_that_pink)
				*dst = 0x8000 | *SRC;	//alpha bit 15
			else
				*dst = *SRC;
			++SRC;
			++dst;
			--x;
		}
	}
}

//public
//public
//public
//public
void win32_d3d9_softdraw_adapter_t::win32_d3d9_resource_on_destroy_device()
{
	SAFE_RELEASE(texture);
}

win32_d3d9_softdraw_adapter_t::win32_d3d9_softdraw_adapter_t(const uint32_t in_alpha)
	:ALPHA(in_alpha)
{
	texture_aspect = 0;
	texture = nullptr;

	//this sets up color encoding to match the texture format we will be using
	if (in_alpha)
		sd_set_555();
	else
		sd_set_565();
}

void win32_d3d9_softdraw_present_2d(
	const sd_bitmap_t& in_canvas, const int32_t in_x, const int32_t in_y, const int32_t in_width, const int32_t in_height, const uint32_t in_color, const win32_d3d9_fixed_function_mode_t in_mode, const bool in_filter,
	win32_d3d9_softdraw_adapter_t& in_adapter)
{
	::HRESULT hr;

	//make sure the texture is created and big enough for the incoming canvas
	if (!in_adapter.texture || in_adapter.texture_aspect < (uint32_t)in_canvas.width || in_adapter.texture_aspect < (uint32_t)in_canvas.height)
	{
		//release any previous texture (in order to support incoming canvases of varying sizes we just use the biggest one we've seen)
		SAFE_RELEASE(in_adapter.texture);

		//we want in_adapter power of 2 aspect texture
		in_adapter.texture_aspect = 1;
		while (in_adapter.texture_aspect < (uint32_t)in_canvas.width || in_adapter.texture_aspect < (uint32_t)in_canvas.height)
			in_adapter.texture_aspect *= 2;

		assert(win32_d3d9_state.m_d3d_device);

		//texture format depends on if we want alpha test support
		if (in_adapter.ALPHA)
		{
			VERIFY(win32_d3d9_state.m_d3d_device->CreateTexture(in_adapter.texture_aspect, in_adapter.texture_aspect, 1, 0, D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, &in_adapter.texture, nullptr));
		}
		else
		{
			VERIFY(win32_d3d9_state.m_d3d_device->CreateTexture(in_adapter.texture_aspect, in_adapter.texture_aspect, 1, 0, D3DFMT_R5G6B5, D3DPOOL_MANAGED, &in_adapter.texture, nullptr));
		}
	}

	//copy the canvas to the texture
	{
		::D3DLOCKED_RECT locked_rect;
		assert(in_adapter.texture);
		if (SUCCEEDED(in_adapter.texture->LockRect(0, &locked_rect, nullptr, D3DLOCK_DISCARD)))
		{
			if (in_adapter.ALPHA)
				__copy_alpha(in_canvas, locked_rect);
			else
				__copy(in_canvas, locked_rect);
			in_adapter.texture->UnlockRect(0);
		}
	}

	{
		//setup fixed function state in preparation for render
		::IDirect3DStateBlock9* sb = win32_d3d9_state_block_begin(in_mode, ::D3DCULL_CCW, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1, in_filter, in_adapter.texture);

		//render in_adapter quad
		{
			const uint16_t INDICES[6] =
			{
				0, 1, 2,
				2, 1, 3,
			};
			const float XF = (float)in_x;
			const float YF = (float)in_y;
			const float WIDTHF = (float)in_width - .5f;
			const float HEIGHTF = (float)in_height - .5f;
			const float U_MAX = (float)in_canvas.width / (float)in_adapter.texture_aspect;
			const float V_MAX = (float)in_canvas.height / (float)in_adapter.texture_aspect;
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
				{ XF + WIDTHF, YF + 0, 0.f, 1.f, in_color, U_MAX, 0.f },
				{ XF + 0, YF + HEIGHTF, 0.f, 1.f, in_color, 0.f, V_MAX },
				{ XF + WIDTHF, YF + HEIGHTF, 0.f, 1.f, in_color, U_MAX, V_MAX },
			};
			VERIFY(win32_d3d9_state.m_d3d_device->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, INDICES, D3DFMT_INDEX16, VERTICES, sizeof(VERTICES[0])));
		}

		//restore any state we changed
		VERIFY(sb->Apply());
		SAFE_RELEASE(sb);
	}
}

::IDirect3DStateBlock9* win32_d3d9_state_block_begin(const win32_d3d9_fixed_function_mode_t in_mode, const ::DWORD in_cull, const ::DWORD in_fvf, const bool in_filter, ::IDirect3DTexture9* in_texture)
{
	assert(win32_d3d9_state.m_d3d_device);

	::HRESULT hr;
	::IDirect3DStateBlock9* result = nullptr;

	VERIFY(win32_d3d9_state.m_d3d_device->CreateStateBlock(D3DSBT_ALL, &result));

	//mode
	switch (in_mode)
	{
	case win32_d3d9_fixed_function_mode_t::SET:
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, TRUE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
		break;

	case win32_d3d9_fixed_function_mode_t::SET_NO_Z:
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
		break;

	case win32_d3d9_fixed_function_mode_t::ADD:
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, TRUE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));
		break;

	case win32_d3d9_fixed_function_mode_t::SUB:
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, TRUE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR));
		break;

	case win32_d3d9_fixed_function_mode_t::ALPHA_BLEND:
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, TRUE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

		VERIFY(win32_d3d9_state.m_d3d_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE));
		break;

	case win32_d3d9_fixed_function_mode_t::ALPHA_TEST:
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, TRUE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHAREF, 0x7f));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL));
		break;

	case win32_d3d9_fixed_function_mode_t::ALPHA_TEST_NO_Z:
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHAREF, 0x7f));
		VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL));
		break;
	}

	//filter
	if (in_filter)
	{
		VERIFY(win32_d3d9_state.m_d3d_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
		VERIFY(win32_d3d9_state.m_d3d_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
		VERIFY(win32_d3d9_state.m_d3d_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));
	}
	else
	{
		VERIFY(win32_d3d9_state.m_d3d_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
		VERIFY(win32_d3d9_state.m_d3d_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
		VERIFY(win32_d3d9_state.m_d3d_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
	}

	//cull
	VERIFY(win32_d3d9_state.m_d3d_device->SetRenderState(::D3DRS_CULLMODE, in_cull));

	//vertex format
	VERIFY(win32_d3d9_state.m_d3d_device->SetFVF(in_fvf));

	//texture
	VERIFY(win32_d3d9_state.m_d3d_device->SetTexture(0, in_texture));

	return result;
}
