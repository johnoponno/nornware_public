#include "stdafx.h"
#include "softdraw_adapter.h"

#include "../softdraw/sd_bitmap.h"
#include "state.h"

namespace dx9
{
	struct state_block_t
	{
		::IDirect3DStateBlock9* state;
	};

	struct vertex_t
	{
		explicit vertex_t(const float aX, const float aY, const float aU, const float aV, const uint32_t aColor)
			:rhw(1.f)
			, color(aColor)
		{
			pos = { aX, aY, 0.f };
			uv = { aU, aV };
		}

		//members
		vec3f_t pos;
		float rhw;
		uint32_t color;
		vec2f_t uv;
	};

	static state_block_t __state_block_begin(const ff_mode_t mode, const ::DWORD cull, const ::DWORD fvf, const bool filter, ::IDirect3DTexture9* texture)
	{
		assert(state.m_d3d_device);

		::HRESULT hr;
		state_block_t result{};

		VERIFY(state.m_d3d_device->CreateStateBlock(D3DSBT_ALL, &result.state));

		//mode
		switch (mode)
		{
		case ff_mode_t::set:
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, TRUE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
			break;

		case ff_mode_t::set_no_z:
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
			break;

		case ff_mode_t::add:
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, TRUE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));
			break;

		case ff_mode_t::sub:
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, TRUE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR));
			break;

		case ff_mode_t::alpha_blend:
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, TRUE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

			VERIFY(state.m_d3d_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));
			VERIFY(state.m_d3d_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE));
			break;

		case ff_mode_t::alpha_test:
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, TRUE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHAREF, 0x7f));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL));
			break;

		case ff_mode_t::alpha_test_no_z:
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHAREF, 0x7f));
			VERIFY(state.m_d3d_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL));
			break;
		}

		//filter
		if (filter)
		{
			VERIFY(state.m_d3d_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
			VERIFY(state.m_d3d_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
			VERIFY(state.m_d3d_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));
		}
		else
		{
			VERIFY(state.m_d3d_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
			VERIFY(state.m_d3d_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
			VERIFY(state.m_d3d_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
		}

		//cull
		VERIFY(state.m_d3d_device->SetRenderState(::D3DRS_CULLMODE, cull));

		//vertex format
		VERIFY(state.m_d3d_device->SetFVF(fvf));

		//texture
		VERIFY(state.m_d3d_device->SetTexture(0, texture));

		return result;
	}

	static void __state_block_end(state_block_t& sb)
	{
		::HRESULT hr;

		VERIFY(sb.state->Apply());
		SAFE_RELEASE(sb.state);
	}

	static void __create_texture(const sd_bitmap_t& aCanvas, softdraw_adapter_t& a)
	{
		SAFE_RELEASE(a.texture);

		a.texture_aspect = 1;
		while (a.texture_aspect < (uint32_t)aCanvas.width ||
			a.texture_aspect < (uint32_t)aCanvas.height)
		{
			a.texture_aspect *= 2;
		}

		assert(state.m_d3d_device);

		::HRESULT hr;
		if (a.alpha)
		{
			VERIFY(state.m_d3d_device->CreateTexture(a.texture_aspect, a.texture_aspect, 1, 0, D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, &a.texture, nullptr));
		}
		else
		{
			VERIFY(state.m_d3d_device->CreateTexture(a.texture_aspect, a.texture_aspect, 1, 0, D3DFMT_R5G6B5, D3DPOOL_MANAGED, &a.texture, nullptr));
		}
	}

	static void __copy(const sd_bitmap_t& canvas, ::D3DLOCKED_RECT& rect)
	{
#if 0
		const uint32_t PITCH = rect.Pitch / sizeof(uint16_t);
		uint16_t* dstStart((uint16_t*)rect.pBits);
		Concurrency::parallel_for(0, canvas.height,
			[&PITCH, &canvas, &dstStart](const int32_t y)
		{
			int32_t x = canvas.width;
			uint16_t* dst = dstStart + y * PITCH;
			const uint16_t* SRC = canvas.pixels + y * canvas.width;

			while (x)
			{
				*dst = *SRC;
				++SRC;
				++dst;
				--x;
			}
		}
		);
#else
		const uint32_t PITCH = rect.Pitch / sizeof(uint16_t);
		uint16_t* dstStart = (uint16_t*)rect.pBits;

		for (int32_t y = 0; y < canvas.height; ++y)
		{
			int32_t x = canvas.width;
			uint16_t* dst = dstStart + y * PITCH;
			const uint16_t* SRC = canvas.pixels + y * canvas.width;

			while (x)
			{
				*dst = *SRC;
				++SRC;
				++dst;
				--x;
			}
		}
#endif
	}

	static void __copy_alpha(const sd_bitmap_t& canvas, ::D3DLOCKED_RECT& rect)
	{
#if 0
		const uint32_t PITCH = rect.Pitch / sizeof(uint16_t);
		uint16_t* dstStart = (uint16_t*)rect.pBits;
		Concurrency::parallel_for(0, canvas.height,
			[&PITCH, &canvas, &dstStart](const int32_t y)
		{
			int32_t x = canvas.width;
			uint16_t* dst = dstStart + y * PITCH;
			const uint16_t* SRC = canvas.pixels + y * canvas.width;

			while (x)
			{
				if (*SRC != softdraw::that_pink)
					*dst = 0x8000 | *SRC;	//alpha bit 15
				else
					*dst = *SRC;
				++SRC;
				++dst;
				--x;
			}
		}
		);
#else
		const uint32_t PITCH = rect.Pitch / sizeof(uint16_t);
		uint16_t* dstStart = (uint16_t*)rect.pBits;

		for (int32_t y = 0; y < canvas.height; ++y)
		{
			int32_t x = canvas.width;
			uint16_t* dst = dstStart + y * PITCH;
			const uint16_t* SRC = canvas.pixels + y * canvas.width;

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
#endif
	}

	static void __copy_to_texture(const sd_bitmap_t& aCanvas, softdraw_adapter_t& a)
	{
		::D3DLOCKED_RECT locked_rect;

		assert(a.texture);
		if (SUCCEEDED(a.texture->LockRect(0, &locked_rect, nullptr, D3DLOCK_DISCARD)))
		{
			if (a.alpha)
				__copy_alpha(aCanvas, locked_rect);
			else
				__copy(aCanvas, locked_rect);
			a.texture->UnlockRect(0);
		}
	}

	static vec2f_t __uv_max(const sd_bitmap_t& aCanvas, const softdraw_adapter_t& a)
	{
		return{ (float)aCanvas.width / (float)a.texture_aspect, (float)aCanvas.height / (float)a.texture_aspect };
	}

	static ::IDirect3DTexture9* __prepare_texture(const sd_bitmap_t& aCanvas, softdraw_adapter_t& a)
	{
		if (!a.texture ||
			a.texture_aspect < (uint32_t)aCanvas.width ||
			a.texture_aspect < (uint32_t)aCanvas.height)
		{
			__create_texture(aCanvas, a);
		}

		__copy_to_texture(aCanvas, a);

		return a.texture;
	}

	static void __render2(const sd_bitmap_t& aCanvas, const vec2f_t& aPosition, const vec2f_t& aSize, const uint32_t aColor, const ff_mode_t aMode, const bool aFilter, softdraw_adapter_t& a)
	{
		const vec2f_t uv_max = __uv_max(aCanvas, a);
		const uint16_t indices[6] =
		{
			0, 1, 2,
			2, 1, 3,
		};
		const vertex_t vertices[4] =
		{
			vertex_t(aPosition.x + 0, aPosition.y + 0, 0.f, 0.f, aColor),
			vertex_t(aPosition.x + aSize.x, aPosition.y + 0, uv_max.x, 0.f, aColor),
			vertex_t(aPosition.x + 0, aPosition.y + aSize.y, 0.f, uv_max.y, aColor),
			vertex_t(aPosition.x + aSize.x, aPosition.y + aSize.y, uv_max.x, uv_max.y, aColor),
		};

		{
			state_block_t sb = __state_block_begin(aMode, ::D3DCULL_CCW, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1, aFilter, __prepare_texture(aCanvas, a));
			::HRESULT hr;
			VERIFY(state.m_d3d_device->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, indices, D3DFMT_INDEX16, vertices, sizeof(vertex_t)));
			__state_block_end(sb);
		}
	}

	//dxut::D3DResource
	void softdraw_adapter_t::on_destroy_device()
	{
		SAFE_RELEASE(texture);
	}

	//specific
	softdraw_adapter_t::softdraw_adapter_t(const uint32_t anAlpha)
		:alpha(anAlpha)
	{
		texture_aspect = 1;
		texture = nullptr;

		if (anAlpha)
			sd_set_555();
		else
			sd_set_565();
	}

	void softdraw_adapter_present_2d(const sd_bitmap_t& aCanvas, const int32_t aX, const int32_t aY, const int32_t aWidth, const int32_t aHeight, const uint32_t aColor, const ff_mode_t aMode, const bool aFilter, softdraw_adapter_t& a)
	{
		__render2(aCanvas, { (float)aX, (float)aY }, { (float)aWidth - .5f, (float)aHeight - .5f }, aColor, aMode, aFilter, a);
	}
}
