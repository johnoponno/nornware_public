#include "stdafx.h"

#include "state.h"
#include "app.h"

namespace dx9
{
	struct enum_depth_stencil_multisample_conflict_t
	{
		D3DFORMAT myDSFormat;
		D3DMULTISAMPLE_TYPE myMSType;
	};

	static int32_t __cdecl __sort_modes_callback(const void* arg1, const void* arg2)
	{
		D3DDISPLAYMODE* pdm1 = (D3DDISPLAYMODE*)arg1;
		D3DDISPLAYMODE* pdm2 = (D3DDISPLAYMODE*)arg2;

		if (pdm1->Width > pdm2->Width)
			return 1;

		if (pdm1->Width < pdm2->Width)
			return -1;

		if (pdm1->Height > pdm2->Height)
			return 1;

		if (pdm1->Height < pdm2->Height)
			return -1;

		if (pdm1->Format > pdm2->Format)
			return 1;

		if (pdm1->Format < pdm2->Format)
			return -1;

		if (pdm1->RefreshRate > pdm2->RefreshRate)
			return 1;

		if (pdm1->RefreshRate < pdm2->RefreshRate)
			return -1;

		return 0;
	}

	static void __clear_adapter_info_list(enumeration_t& e)
	{
		vector_set_delete_all_and_clear(e.adapter_info_list);
	}

	static void __build_present_interval_list(enum_device_info_t* pDeviceInfo, enum_device_settings_combo_t* pDeviceCombo, enumeration_t& e)
	{
		UINT pi;
		for (uint32_t ipi = 0; ipi < e.present_interval_list.size(); ++ipi)
		{
			pi = e.present_interval_list[ipi];
			if (pDeviceCombo->windowed)
			{
				if (pi == D3DPRESENT_INTERVAL_TWO ||
					pi == D3DPRESENT_INTERVAL_THREE ||
					pi == D3DPRESENT_INTERVAL_FOUR)
				{
					// These intervals are not supported in windowed mode.
					continue;
				}
			}
			// Note that D3DPRESENT_INTERVAL_DEFAULT is zero, so you
			// can't do a caps check for it -- it is always available.
			if (pi == D3DPRESENT_INTERVAL_DEFAULT || (pDeviceInfo->caps.PresentationIntervals & pi))
			{
				pDeviceCombo->present_intervals.push_back(pi);
			}
		}
	}

	static void __build_dsms_conflict_list(enum_device_settings_combo_t* pDeviceCombo)
	{
		enum_depth_stencil_multisample_conflict_t DSMSConflict;

		for (uint32_t iDS = 0; iDS<pDeviceCombo->depth_stencil_formats.size(); ++iDS)
		{
			const D3DFORMAT dsFmt = pDeviceCombo->depth_stencil_formats[iDS];

			for (uint32_t iMS = 0; iMS<pDeviceCombo->multisample_types.size(); ++iMS)
			{
				const D3DMULTISAMPLE_TYPE MSTYPE = pDeviceCombo->multisample_types[iMS];
				if (FAILED(state.m_d3d->CheckDeviceMultiSampleType(pDeviceCombo->adapter_ordinal, pDeviceCombo->device_type, dsFmt, pDeviceCombo->windowed, MSTYPE, nullptr)))
				{
					DSMSConflict.myDSFormat = dsFmt;
					DSMSConflict.myMSType = MSTYPE;
					pDeviceCombo->dsms_conflicts.push_back(DSMSConflict);
				}
			}
		}
	}

	static void __build_multisample_type_list(enum_device_settings_combo_t* pDeviceCombo, enumeration_t& e)
	{
		D3DMULTISAMPLE_TYPE msType;
		DWORD msQuality;
		for (uint32_t imst = 0; imst < e.multisample_type_list.size(); ++imst)
		{
			msType = e.multisample_type_list[imst];
			if (SUCCEEDED(state.m_d3d->CheckDeviceMultiSampleType(pDeviceCombo->adapter_ordinal, pDeviceCombo->device_type, pDeviceCombo->backbuffer_format, pDeviceCombo->windowed, msType, &msQuality)))
			{
				pDeviceCombo->multisample_types.push_back(msType);
				if (msQuality > e.multisample_quality_max + 1)
					msQuality = e.multisample_quality_max + 1;
				pDeviceCombo->multisample_qualities.push_back(msQuality);
			}
		}
	}

	static void __build_depth_stencil_format_list(enum_device_settings_combo_t* pDeviceCombo, enumeration_t& e)
	{
		D3DFORMAT depthStencilFmt;
		for (uint32_t idsf = 0; idsf < e.depth_stencil_possible_list.size(); ++idsf)
		{
			depthStencilFmt = e.depth_stencil_possible_list[idsf];
			if (SUCCEEDED(state.m_d3d->CheckDeviceFormat(pDeviceCombo->adapter_ordinal, pDeviceCombo->device_type, pDeviceCombo->adapter_format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, depthStencilFmt)))
			{
				if (SUCCEEDED(state.m_d3d->CheckDepthStencilMatch(pDeviceCombo->adapter_ordinal, pDeviceCombo->device_type, pDeviceCombo->adapter_format, pDeviceCombo->backbuffer_format, depthStencilFmt)))
				{
					pDeviceCombo->depth_stencil_formats.push_back(depthStencilFmt);
				}
			}
		}
	}

	static ::HRESULT __enumerate_device_combos(enum_adapter_info_t* pAdapterInfo, enum_device_info_t* pDeviceInfo, std::vector<D3DFORMAT>* pAdapterFormatList, enumeration_t& e)
	{
		const D3DFORMAT BACKBUFFER_FORMAT_ARRAY[] =
		{
			D3DFMT_A8R8G8B8,
			D3DFMT_X8R8G8B8,
			D3DFMT_A2R10G10B10,
			D3DFMT_R5G6B5,
			D3DFMT_A1R5G5B5,
			D3DFMT_X1R5G5B5
		};

		// See which adapter formats are supported by this device
		for (uint32_t iFormat = 0; iFormat < pAdapterFormatList->size(); ++iFormat)
		{
			const D3DFORMAT adapterFormat = (*pAdapterFormatList)[iFormat];

			for (UINT iBackBufferFormat = 0; iBackBufferFormat < _countof(BACKBUFFER_FORMAT_ARRAY); iBackBufferFormat++)
			{
				const D3DFORMAT backBufferFormat = BACKBUFFER_FORMAT_ARRAY[iBackBufferFormat];

				for (int32_t nWindowed = 0; nWindowed < 2; ++nWindowed)
				{
					if (0 == nWindowed && 0 == pAdapterInfo->display_mode_list.size())
						continue;

					if (FAILED(state.m_d3d->CheckDeviceType(pAdapterInfo->adapter_ordinal, pDeviceInfo->device_type, adapterFormat, backBufferFormat, nWindowed)))
					{
						continue;
					}

					if (e.require_post_pixel_shaders_blending)
					{
						// If the backbuffer format doesn't support D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING
						// then alpha test, pixel fog, render-target blending, color write enable, and dithering. 
						// are not supported.
						if (FAILED(state.m_d3d->CheckDeviceFormat(pAdapterInfo->adapter_ordinal, pDeviceInfo->device_type, adapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_TEXTURE, backBufferFormat)))
						{
							continue;
						}
					}

					// If an application callback function has been provided, make sure this device
					// is acceptable to the app.
					if (state.app)
					{
						if (!state.app->is_device_acceptable(pDeviceInfo->caps, adapterFormat, backBufferFormat, FALSE != nWindowed))
							continue;
					}

					// At this point, we have an adapter/device/adapterformat/backbufferformat/iswindowed
					// DeviceCombo that is supported by the system and acceptable to the app. We still 
					// need to find one or more suitable depth/stencil buffer format,
					// multisample type, and present interval.
					enum_device_settings_combo_t* pDeviceCombo = new enum_device_settings_combo_t;
					if (pDeviceCombo == nullptr)
						return E_OUTOFMEMORY;

					pDeviceCombo->adapter_ordinal = pAdapterInfo->adapter_ordinal;
					pDeviceCombo->device_type = pDeviceInfo->device_type;
					pDeviceCombo->adapter_format = adapterFormat;
					pDeviceCombo->backbuffer_format = backBufferFormat;
					pDeviceCombo->windowed = (nWindowed != 0);

					__build_depth_stencil_format_list(pDeviceCombo, e);
					__build_multisample_type_list(pDeviceCombo, e);
					if (0 == pDeviceCombo->multisample_types.size())
					{
						delete pDeviceCombo;
						continue;
					}
					__build_dsms_conflict_list(pDeviceCombo);
					__build_present_interval_list(pDeviceInfo, pDeviceCombo, e);
					pDeviceCombo->adapter_info = pAdapterInfo;
					pDeviceCombo->device_info = pDeviceInfo;

					pDeviceInfo->device_settings_combo_list.push_back(pDeviceCombo);

				}
			}
		}

		return S_OK;
	}

	static ::HRESULT __enumerate_devices(enum_adapter_info_t* pAdapterInfo, std::vector<D3DFORMAT>* pAdapterFormatList, enumeration_t& e)
	{
		const D3DDEVTYPE DEV_TYPE_ARRAY[] =
		{
			D3DDEVTYPE_HAL,
			D3DDEVTYPE_SW,
			D3DDEVTYPE_REF
		};

		// Enumerate each Direct3D device type
		::HRESULT hr;
		for (UINT iDeviceType = 0; iDeviceType < _countof(DEV_TYPE_ARRAY); ++iDeviceType)
		{
			enum_device_info_t* pDeviceInfo = new enum_device_info_t;
			if (pDeviceInfo == nullptr)
				return E_OUTOFMEMORY;

			// Fill struct w/ AdapterOrdinal and D3DDEVTYPE
			pDeviceInfo->device_type = DEV_TYPE_ARRAY[iDeviceType];

			// Store device caps
			if (FAILED(hr = state.m_d3d->GetDeviceCaps(pAdapterInfo->adapter_ordinal, pDeviceInfo->device_type, &pDeviceInfo->caps)))
			{
				delete pDeviceInfo;
				continue;
			}

			// create a dummy device to verify that we really can create of this type.
			D3DDISPLAYMODE displayMode;
			state.m_d3d->GetAdapterDisplayMode(0, &displayMode);
			D3DPRESENT_PARAMETERS pp;
			ZeroMemory(&pp, sizeof(D3DPRESENT_PARAMETERS));
			pp.BackBufferWidth = 1;
			pp.BackBufferHeight = 1;
			pp.BackBufferFormat = displayMode.Format;
			pp.BackBufferCount = 1;
			pp.SwapEffect = D3DSWAPEFFECT_COPY;
			pp.Windowed = TRUE;
			pp.hDeviceWindow = state.m_hwnd.focus;
			IDirect3DDevice9 *pDevice;
			if (FAILED(hr = state.m_d3d->CreateDevice(pAdapterInfo->adapter_ordinal, pDeviceInfo->device_type, state.m_hwnd.focus, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &pDevice)))
			{
				if (hr == D3DERR_NOTAVAILABLE)
				{
					delete pDeviceInfo;
					continue;
				}
			}
			SAFE_RELEASE(pDevice);

			// get info for each devicecombo on this device
			if (FAILED(hr = __enumerate_device_combos(pAdapterInfo, pDeviceInfo, pAdapterFormatList, e)))
			{
				delete pDeviceInfo;
				continue;
			}

			// If at least one devicecombo for this device is found, 
			// add the deviceInfo to the list
			if (pDeviceInfo->device_settings_combo_list.size() > 0)
				pAdapterInfo->device_info_list.push_back(pDeviceInfo);
			else
				delete pDeviceInfo;
		}

		return S_OK;
	}

	static const enum_device_info_t* __get_device_info(const UINT AdapterOrdinal, const D3DDEVTYPE DeviceType, const enumeration_t& e)
	{
		const enum_adapter_info_t* pAdapterInfo = get_adapter_info(AdapterOrdinal, e);
		if (pAdapterInfo)
		{
			for (uint32_t iDeviceInfo = 0; iDeviceInfo<pAdapterInfo->device_info_list.size(); ++iDeviceInfo)
			{
				enum_device_info_t* pDeviceInfo = pAdapterInfo->device_info_list[iDeviceInfo];
				if (pDeviceInfo->device_type == DeviceType)
					return pDeviceInfo;
			}
		}

		return nullptr;
	}

	enumeration_t* enumeration_t::instance()
	{
		// using an accessor function gives control of the construction order
		static enumeration_t enumeration;
		return &enumeration;
	}

	enumeration_t::enumeration_t()
	{
		this->require_post_pixel_shaders_blending = true;

		this->min_width = 640;
		this->min_height = 480;
		this->max_width = UINT32_MAX;
		this->max_height = UINT32_MAX;

		this->refresh_min = 0;
		this->refresh_max = UINT32_MAX;

		this->multisample_quality_max = 0xffff;

		assert(0 == this->depth_stencil_possible_list.size());
		this->depth_stencil_possible_list.push_back(D3DFMT_D16);
		this->depth_stencil_possible_list.push_back(D3DFMT_D15S1);
		this->depth_stencil_possible_list.push_back(D3DFMT_D24X8);
		this->depth_stencil_possible_list.push_back(D3DFMT_D24S8);
		this->depth_stencil_possible_list.push_back(D3DFMT_D24X4S4);
		this->depth_stencil_possible_list.push_back(D3DFMT_D32);

		assert(0 == this->multisample_type_list.size());
		this->multisample_type_list.push_back(D3DMULTISAMPLE_NONE);
		this->multisample_type_list.push_back(D3DMULTISAMPLE_NONMASKABLE);
		this->multisample_type_list.push_back(D3DMULTISAMPLE_2_SAMPLES);
		this->multisample_type_list.push_back(D3DMULTISAMPLE_3_SAMPLES);
		this->multisample_type_list.push_back(D3DMULTISAMPLE_4_SAMPLES);
		this->multisample_type_list.push_back(D3DMULTISAMPLE_5_SAMPLES);
		this->multisample_type_list.push_back(D3DMULTISAMPLE_6_SAMPLES);
		this->multisample_type_list.push_back(D3DMULTISAMPLE_7_SAMPLES);
		this->multisample_type_list.push_back(D3DMULTISAMPLE_8_SAMPLES);
		this->multisample_type_list.push_back(D3DMULTISAMPLE_9_SAMPLES);
		this->multisample_type_list.push_back(D3DMULTISAMPLE_10_SAMPLES);
		this->multisample_type_list.push_back(D3DMULTISAMPLE_11_SAMPLES);
		this->multisample_type_list.push_back(D3DMULTISAMPLE_12_SAMPLES);
		this->multisample_type_list.push_back(D3DMULTISAMPLE_13_SAMPLES);
		this->multisample_type_list.push_back(D3DMULTISAMPLE_14_SAMPLES);
		this->multisample_type_list.push_back(D3DMULTISAMPLE_15_SAMPLES);
		this->multisample_type_list.push_back(D3DMULTISAMPLE_16_SAMPLES);

		assert(0 == this->present_interval_list.size());
		this->present_interval_list.push_back(D3DPRESENT_INTERVAL_IMMEDIATE);
		this->present_interval_list.push_back(D3DPRESENT_INTERVAL_DEFAULT);
		this->present_interval_list.push_back(D3DPRESENT_INTERVAL_ONE);
		this->present_interval_list.push_back(D3DPRESENT_INTERVAL_TWO);
		this->present_interval_list.push_back(D3DPRESENT_INTERVAL_THREE);
		this->present_interval_list.push_back(D3DPRESENT_INTERVAL_FOUR);

		this->software_vp = true;
		this->hardware_vp = true;
		this->pure_hardware_vp = true;
		this->mixed_vp = false;
	}

	enumeration_t::~enumeration_t()
	{
		__clear_adapter_info_list(*this);
	}

	//--------------------------------------------------------------------------------------
	// Enumerates available D3D adapters, devices, modes, etc.
	//--------------------------------------------------------------------------------------
	HRESULT enumerate(enumeration_t& e)
	{
		__clear_adapter_info_list(e);
		std::vector<D3DFORMAT> adapterFormatList;

		const D3DFORMAT ALLOWED_ADAPTER_FORMAT_ARRAY[] =
		{
			D3DFMT_X8R8G8B8,
			D3DFMT_X1R5G5B5,
			D3DFMT_R5G6B5,
			D3DFMT_A2R10G10B10
		};

		const UINT NUM_ADAPTERS = state.m_d3d->GetAdapterCount();
		for (UINT adapterOrdinal = 0; adapterOrdinal < NUM_ADAPTERS; ++adapterOrdinal)
		{
			enum_adapter_info_t* pAdapterInfo = new enum_adapter_info_t;
			if (pAdapterInfo == nullptr)
				return E_OUTOFMEMORY;

			pAdapterInfo->adapter_ordinal = adapterOrdinal;
			state.m_d3d->GetAdapterIdentifier(adapterOrdinal, 0, &pAdapterInfo->adapter_identifier);

			// get list of all display modes on this adapter.  
			// Also build a temporary list of all display adapter formats.
			adapterFormatList.clear();

			for (UINT iFormatList = 0; iFormatList < _countof(ALLOWED_ADAPTER_FORMAT_ARRAY); ++iFormatList)
			{
				D3DFORMAT allowedAdapterFormat = ALLOWED_ADAPTER_FORMAT_ARRAY[iFormatList];
				UINT numAdapterModes = state.m_d3d->GetAdapterModeCount(adapterOrdinal, allowedAdapterFormat);
				for (UINT mode = 0; mode < numAdapterModes; mode++)
				{
					D3DDISPLAYMODE displayMode;
					state.m_d3d->EnumAdapterModes(adapterOrdinal, allowedAdapterFormat, mode, &displayMode);

					if (displayMode.Width < e.min_width ||
						displayMode.Height < e.min_height ||
						displayMode.Width > e.max_width ||
						displayMode.Height > e.max_height ||
						displayMode.RefreshRate < e.refresh_min ||
						displayMode.RefreshRate > e.refresh_max)
					{
						continue;
					}

					pAdapterInfo->display_mode_list.push_back(displayMode);

					/*
					FS_LOG("found displaymode: Width = %u, Height = %u, RefreshRate = %u, Format = %u", displayMode.Width, displayMode.Height, displayMode.RefreshRate, displayMode.Format);
					if ((float)displayMode.Width / (float)displayMode.Height == (16.f / 9.f))
					FS_LOG("16:9 displaymode: Width = %u, Height = %u, RefreshRate = %u, Format = %u", displayMode.Width, displayMode.Height, displayMode.RefreshRate, displayMode.Format);
					*/

					if (!vector_contains(adapterFormatList, displayMode.Format))
						adapterFormatList.push_back(displayMode.Format);
				}

			}

			{
				D3DDISPLAYMODE displayMode;
				state.m_d3d->GetAdapterDisplayMode(adapterOrdinal, &displayMode);
				if (!vector_contains(adapterFormatList, displayMode.Format))
					adapterFormatList.push_back(displayMode.Format);
			}

			// Sort displaymode list
			::qsort(pAdapterInfo->display_mode_list.data(), pAdapterInfo->display_mode_list.size(), sizeof(D3DDISPLAYMODE), __sort_modes_callback);

			// get info for each device on this adapter
			if (FAILED(__enumerate_devices(pAdapterInfo, &adapterFormatList, e)))
			{
				delete pAdapterInfo;
				continue;
			}

			// If at least one device on this adapter is available and compatible
			// with the app, add the adapterInfo to the list
			if (pAdapterInfo->device_info_list.size() > 0)
				e.adapter_info_list.push_back(pAdapterInfo);
			else
				delete pAdapterInfo;
		}

		bool bUniqueDesc = true;
		for (uint32_t i = 0; i < e.adapter_info_list.size(); ++i)
		{
			const enum_adapter_info_t* pAdapterInfo1 = e.adapter_info_list[i];

			for (uint32_t j = i + 1; j < e.adapter_info_list.size(); ++j)
			{
				const enum_adapter_info_t* pAdapterInfo2 = e.adapter_info_list[j];
				if (::_stricmp(pAdapterInfo1->adapter_identifier.Description, pAdapterInfo2->adapter_identifier.Description) == 0)
				{
					bUniqueDesc = false;
					break;
				}
			}

			if (!bUniqueDesc)
				break;
		}

		for (uint32_t i = 0; i < e.adapter_info_list.size(); ++i)
		{
			enum_adapter_info_t* pAdapterInfo = e.adapter_info_list[i];

			::strcpy_s(pAdapterInfo->description, 100, pAdapterInfo->adapter_identifier.Description);

			if (!bUniqueDesc)
			{
				char sz[100];
				::sprintf_s(sz, 100, " (#%d)", pAdapterInfo->adapter_ordinal);
				::strcat_s(pAdapterInfo->description, 256, sz);
			}
		}

		return S_OK;
	}

	const enum_adapter_info_t* get_adapter_info(const UINT AdapterOrdinal, const enumeration_t& e)
	{
		for (uint32_t iAdapter = 0; iAdapter < e.adapter_info_list.size(); ++iAdapter)
		{
			const enum_adapter_info_t* pAdapterInfo = e.adapter_info_list[iAdapter];
			if (pAdapterInfo->adapter_ordinal == AdapterOrdinal)
				return pAdapterInfo;
		}

		return nullptr;
	}

	const enum_device_settings_combo_t* get_device_settings_combo(
		const UINT AdapterOrdinal,
		const D3DDEVTYPE DeviceType,
		const D3DFORMAT AdapterFormat,
		const D3DFORMAT BackBufferFormat,
		const BOOL Windowed,
		const enumeration_t& e)
	{
		const enum_device_info_t* pDeviceInfo = __get_device_info(AdapterOrdinal, DeviceType, e);
		if (pDeviceInfo)
		{
			for (uint32_t iDeviceCombo = 0; iDeviceCombo<pDeviceInfo->device_settings_combo_list.size(); ++iDeviceCombo)
			{
				enum_device_settings_combo_t* pDeviceSettingsCombo = pDeviceInfo->device_settings_combo_list[iDeviceCombo];
				if (pDeviceSettingsCombo->adapter_format == AdapterFormat && pDeviceSettingsCombo->backbuffer_format == BackBufferFormat && pDeviceSettingsCombo->windowed == Windowed)
					return pDeviceSettingsCombo;
			}
		}

		return nullptr;
	}

	enum_adapter_info_t::~enum_adapter_info_t(void)
	{
		vector_set_delete_all_and_clear(device_info_list);
	}

	enum_device_info_t::~enum_device_info_t(void)
	{
		vector_set_delete_all_and_clear(device_settings_combo_list);
	}
}