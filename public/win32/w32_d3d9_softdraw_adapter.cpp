#include "stdafx.h"
#include "w32_d3d9_softdraw_adapter.h"

#include "w32_d3d9_state.h"

struct
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} __rgb_shift{}, __rgb_bits{};

//public
//public
//public
//public

void w32_d3d9_softdraw_adapter_t::w32_d3d9_resource_on_destroy_device()
{
	SAFE_RELEASE(texture);
}

w32_d3d9_softdraw_adapter_t::w32_d3d9_softdraw_adapter_t(const uint32_t in_alpha)
	:ALPHA(in_alpha)
{
	texture_aspect = 0;
	texture = nullptr;

	//this sets up color encoding to match the texture format we will be using
	//NOTE: only really applicable for 16bit use (the color conversion stuff is global, hence these functions_
	if (in_alpha)
	{
		__rgb_shift = { 10, 5, 0 };
		__rgb_bits = { 5, 5, 5 };
	}
	else
	{
		__rgb_shift = { 11, 5, 0 };
		__rgb_bits = { 5, 6, 5 };
	}
}

uint16_t w32_d3d9_softdraw_adapter_t::color_encode(const uint8_t aR, const uint8_t aG, const uint8_t aB) const
{
	return
		((aR >> (8 - __rgb_bits.r)) << __rgb_shift.r) |
		((aG >> (8 - __rgb_bits.g)) << __rgb_shift.g) |
		((aB >> (8 - __rgb_bits.b)) << __rgb_shift.b);
}

::IDirect3DStateBlock9* w32_d3d9_state_block_begin(const w32_d3d9_fixed_function_mode_t in_mode, const ::DWORD in_cull, const ::DWORD in_fvf, const bool in_filter, ::IDirect3DTexture9* in_texture)
{
	assert(w32_d3d9_state.m_d3d_device);

	::HRESULT hr;
	::IDirect3DStateBlock9* result = nullptr;

	VERIFY(w32_d3d9_state.m_d3d_device->CreateStateBlock(D3DSBT_ALL, &result));

	//mode
	switch (in_mode)
	{
	case w32_d3d9_fixed_function_mode_t::SET:
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, TRUE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
		break;

	case w32_d3d9_fixed_function_mode_t::SET_NO_Z:
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
		break;

	case w32_d3d9_fixed_function_mode_t::ADD:
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, TRUE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));
		break;

	case w32_d3d9_fixed_function_mode_t::SUB:
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, TRUE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR));
		break;

	case w32_d3d9_fixed_function_mode_t::ALPHA_BLEND:
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, TRUE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

		VERIFY(w32_d3d9_state.m_d3d_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE));
		break;

	case w32_d3d9_fixed_function_mode_t::ALPHA_TEST:
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, TRUE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHAREF, 0x7f));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL));
		break;

	case w32_d3d9_fixed_function_mode_t::ALPHA_TEST_NO_Z:
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHAREF, 0x7f));
		VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL));
		break;
	}

	//filter
	if (in_filter)
	{
		VERIFY(w32_d3d9_state.m_d3d_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
		VERIFY(w32_d3d9_state.m_d3d_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
		VERIFY(w32_d3d9_state.m_d3d_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));
	}
	else
	{
		VERIFY(w32_d3d9_state.m_d3d_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
		VERIFY(w32_d3d9_state.m_d3d_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
		VERIFY(w32_d3d9_state.m_d3d_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
	}

	//cull
	VERIFY(w32_d3d9_state.m_d3d_device->SetRenderState(::D3DRS_CULLMODE, in_cull));

	//vertex format
	VERIFY(w32_d3d9_state.m_d3d_device->SetFVF(in_fvf));

	//texture
	VERIFY(w32_d3d9_state.m_d3d_device->SetTexture(0, in_texture));

	return result;
}
