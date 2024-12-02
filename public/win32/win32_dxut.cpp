#include "stdafx.h"

#include "win32_d3d9_state.h"
#include "w32_timer.h"
#include "win32_d3d9_resource.h"
#include "win32_d3d9_app.h"

#define DXUT_MIN_WINDOW_SIZE_X 200
#define DXUT_MIN_WINDOW_SIZE_Y 200

#ifndef SM_REMOTESESSION  // needs WINVER >= 0x0500
#define SM_REMOTESESSION  0x1000
#endif

//--------------------------------------------------------------------------------------
// Finding valid device settings
//--------------------------------------------------------------------------------------
enum match_type_t
{
	IGNORE_INPUT,  // Use the closest valid value to a default 
	PRESERVE_INPUT,    // Use input without change, but may cause no valid device to be found
	CLOSEST_TO_INPUT   // Use the closest valid value to the input 
};

struct match_options_t
{
	match_type_t adapter_ordinal;
	match_type_t device_type;
	match_type_t windowed;
	match_type_t adapter_format;
	match_type_t vertex_processing;
	match_type_t resolution;
	match_type_t backbuffer_format;
	match_type_t backbuffer_count;
	match_type_t multi_sample;
	match_type_t swap_effect;
	match_type_t depth_format;
	match_type_t stencil_format;
	match_type_t present_flags;
	match_type_t refresh_rate;
	match_type_t present_interval;
};

static win32_d3d9_backbuffer_size_t __make(const D3DPRESENT_PARAMETERS& params)
{
	return { params.BackBufferWidth, params.BackBufferHeight };
}

static void __revert(win32_d3d9_device_settings_t& settings, match_options_t& options)
{
	const win32_d3d9_backbuffer_size_t size = settings.present_parameters.Windowed ? win32_d3d9_state.windowed_backbuffer_at_mode_change : win32_d3d9_state.fullscreen_backbuffer_at_mode_change;
	if (size.width > 0 && size.height > 0)
	{
		options.resolution = match_type_t::CLOSEST_TO_INPUT;
		settings.present_parameters.BackBufferWidth = size.width;
		settings.present_parameters.BackBufferHeight = size.height;
	}
	else
	{
		// No previous data, so just switch to defaults
		options.resolution = match_type_t::IGNORE_INPUT;
	}
}

w32_timer_t& __timer()
{
	// using an accessor function gives control of the construction order
	static w32_timer_t t;
	return t;
}

//forwards
typedef DECLSPEC_IMPORT UINT(WINAPI* LPTIMEBEGINPERIOD)(UINT uPeriod);

//--------------------------------------------------------------------------------------
// Internal helper function to find the closest allowed display mode to the optimal 
//--------------------------------------------------------------------------------------
static ::HRESULT __find_valid_resolution(const win32_d3d9_enum_device_settings_combo_t* aBestDeviceSettingsCombo, const D3DDISPLAYMODE aDisplayModeIn, D3DDISPLAYMODE* aBestDisplayMode)
{
	::D3DDISPLAYMODE bestDisplayMode;
	::ZeroMemory(&bestDisplayMode, sizeof(::D3DDISPLAYMODE));

	if (aBestDeviceSettingsCombo->windowed)
	{
		// In windowed mode, all resolutions are valid but restritions still apply 
		// on the size of the window.  See ChangeDevice() for details
		*aBestDisplayMode = aDisplayModeIn;
	}
	else
	{
		int32_t nBestRanking = 100000;
		int32_t nCurRanking;
		const std::vector<D3DDISPLAYMODE>* DISPLAY_MODE_LIST = &aBestDeviceSettingsCombo->adapter_info->display_mode_list;
		for (uint32_t iDisplayMode = 0; iDisplayMode < DISPLAY_MODE_LIST->size(); ++iDisplayMode)
		{
			const D3DDISPLAYMODE displayMode = (*DISPLAY_MODE_LIST)[iDisplayMode];

			// Skip display modes that don't match the combo's adapter format
			if (displayMode.Format != aBestDeviceSettingsCombo->adapter_format)
				continue;

			// find the delta between the current width/height and the optimal width/height
			nCurRanking =
				abs((int32_t)displayMode.Width - (int32_t)aDisplayModeIn.Width) +
				abs((int32_t)displayMode.Height - (int32_t)aDisplayModeIn.Height);

			if (nCurRanking < nBestRanking)
			{
				bestDisplayMode = displayMode;
				nBestRanking = nCurRanking;

				// stop if perfect match found
				if (0 == nBestRanking)
					break;
			}
		}

		if (0 == bestDisplayMode.Width)
		{
			*aBestDisplayMode = aDisplayModeIn;
			return E_FAIL; // No valid display modes found
		}

		*aBestDisplayMode = bestDisplayMode;
	}

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Internal helper function to prepare the enumeration object by creating it if it 
// didn't already exist and enumerating if desired.
//--------------------------------------------------------------------------------------
static win32_d3d9_enumeration_t* __prepare_enumeration_object(bool anEnumerate)
{
	// create a new CD3DEnumeration object and enumerate all devices unless its already been done
	if (nullptr == win32_d3d9_state.m_d3d_enumeration)
	{
		win32_d3d9_state.m_d3d_enumeration = win32_d3d9_enumeration_t::instance();

		anEnumerate = true;
	}

	if (anEnumerate)
	{
		// Enumerate for each adapter all of the supported display modes, 
		// device types, adapter formats, back buffer formats, window/full screen support, 
		// depth stencil formats, multisampling types/qualities, and presentations intervals.
		//
		// For each combination of device type (HAL/REF), adapter format, back buffer format, and
		// IsWindowed it will call the app's ConfirmDevice callback.  This allows the app
		// to reject or allow that combination based on its caps/etc.  It also allows the 
		// app to change the BehaviorFlags.  The BehaviorFlags defaults non-pure HWVP 
		// if supported otherwise it will default to SWVP, however the app can change this 
		// through the ConfirmDevice callback.
		win32_d3d9_enumerate(*win32_d3d9_state.m_d3d_enumeration);
	}

	return win32_d3d9_state.m_d3d_enumeration;
}

//--------------------------------------------------------------------------------------
// Updates the string which describes the device 
//--------------------------------------------------------------------------------------
static void __update_device_stats(const ::D3DDEVTYPE aDeviceType, const ::DWORD someBehaviorFlags, const ::D3DADAPTER_IDENTIFIER9* anAdapterIdentifier)
{
	// Store device description
	if (aDeviceType == D3DDEVTYPE_REF)
		::strcpy_s(win32_d3d9_state.m_device_stats, 256, "REF");
	else if (aDeviceType == D3DDEVTYPE_HAL)
		::strcpy_s(win32_d3d9_state.m_device_stats, 256, "HAL");
	else if (aDeviceType == D3DDEVTYPE_SW)
		::strcpy_s(win32_d3d9_state.m_device_stats, 256, "SW");

	if (someBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING && someBehaviorFlags & D3DCREATE_PUREDEVICE)
	{
		if (aDeviceType == D3DDEVTYPE_HAL)
			::strcat_s(win32_d3d9_state.m_device_stats, 256, " (pure hw vp)");
		else
			::strcat_s(win32_d3d9_state.m_device_stats, 256, " (simulated pure hw vp)");
	}
	else if (someBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
	{
		if (aDeviceType == D3DDEVTYPE_HAL)
			::strcat_s(win32_d3d9_state.m_device_stats, 256, " (hw vp)");
		else
			::strcat_s(win32_d3d9_state.m_device_stats, 256, " (simulated hw vp)");
	}
	else if (someBehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING)
	{
		if (aDeviceType == D3DDEVTYPE_HAL)
			::strcat_s(win32_d3d9_state.m_device_stats, 256, " (mixed vp)");
		else
			::strcat_s(win32_d3d9_state.m_device_stats, 256, " (simulated mixed vp)");
	}
	else if (someBehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING)
	{
		::strcat_s(win32_d3d9_state.m_device_stats, 256, " (sw vp)");
	}

	if (aDeviceType == D3DDEVTYPE_HAL)
	{
		// Be sure not to overflow m_strDeviceStats when appending the adapter 
		// description, since it can be long.  
		::strcat_s(win32_d3d9_state.m_device_stats, 256, ": ");

		// Try to get a unique description from the CD3DEnumDeviceSettingsCombo
		win32_d3d9_device_settings_t* pDeviceSettings = win32_d3d9_state.m_device_settings;
		win32_d3d9_enumeration_t* pd3dEnum = __prepare_enumeration_object(false);
		const win32_d3d9_enum_device_settings_combo_t* pDeviceSettingsCombo = win32_d3d9_get_device_settings_combo(
			pDeviceSettings->adapter_ordinal,
			pDeviceSettings->device_type,
			pDeviceSettings->adapter_format,
			pDeviceSettings->present_parameters.BackBufferFormat,
			pDeviceSettings->present_parameters.Windowed,
			*pd3dEnum
		);
		if (pDeviceSettingsCombo)
		{
			::strcat_s(win32_d3d9_state.m_device_stats, 256, pDeviceSettingsCombo->adapter_info->description);
		}
		else
		{
			::strcat_s(win32_d3d9_state.m_device_stats, 256, anAdapterIdentifier->Description);
		}
	}
}

//--------------------------------------------------------------------------------------
// Display an custom error msg box 
//--------------------------------------------------------------------------------------
static void __display_error_message(::HRESULT aResult)
{
	char strBuffer[512];

	bool bFound = true;
	switch (aResult)
	{
	case WIN32_DXUTERR_NODIRECT3D:
		win32_d3d9_state.m_exit_code = 2;
		::strcpy_s(strBuffer, "Could not initialize Direct3D. You may want to check that the latest version of DirectX is correctly installed on your system.  Also make sure that this program was compiled with header files that match the installed DirectX DLLs.");
		break;

	case WIN32_DXUTERR_INCORRECTVERSION:
		win32_d3d9_state.m_exit_code = 10;
		::strcpy_s(strBuffer, "Incorrect version of Direct3D and/or D3DX.");
		break;

	case WIN32_DXUTERR_MEDIANOTFOUND:
		win32_d3d9_state.m_exit_code = 4;
		::strcpy_s(strBuffer, "Could not find required media. Ensure that the DirectX SDK is correctly installed.");
		break;

	case WIN32_DXUTERR_NONZEROREFCOUNT:
		win32_d3d9_state.m_exit_code = 5;
		::strcpy_s(strBuffer, "The D3D device has a non-zero reference count, meaning some objects were not released.");
		break;

	case WIN32_DXUTERR_CREATINGDEVICE:
		win32_d3d9_state.m_exit_code = 6;
		::strcpy_s(strBuffer, "Failed creating the Direct3D device.");
		break;

	case WIN32_DXUTERR_RESETTINGDEVICE:
		win32_d3d9_state.m_exit_code = 7;
		::strcpy_s(strBuffer, "Failed resetting the Direct3D device.");
		break;

	case WIN32_DXUTERR_CREATINGDEVICEOBJECTS:
		win32_d3d9_state.m_exit_code = 8;
		::strcpy_s(strBuffer, "Failed creating Direct3D device objects.");
		break;

	case WIN32_DXUTERR_RESETTINGDEVICEOBJECTS:
		win32_d3d9_state.m_exit_code = 9;
		::strcpy_s(strBuffer, "Failed resetting Direct3D device objects.");
		break;

	case WIN32_DXUTERR_NOCOMPATIBLEDEVICES:
		win32_d3d9_state.m_exit_code = 3;
		if (0 != GetSystemMetrics(SM_REMOTESESSION))
			::strcpy_s(strBuffer, "Direct3D does not work over a remote session.");
		else
			::strcpy_s(strBuffer, "Could not find any compatible Direct3D devices.");
		break;

	default:
		bFound = false;
		win32_d3d9_state.m_exit_code = 1;
		break;
	}

	if (bFound)
	{
		if (win32_d3d9_state.m_window_title[0] == 0)
			::MessageBoxA(win32_d3d9_hwnd(), strBuffer, "DirectX Application", MB_ICONERROR | MB_OK);
		else
			::MessageBoxA(win32_d3d9_hwnd(), strBuffer, win32_d3d9_state.m_window_title, MB_ICONERROR | MB_OK);
	}

	//FS_ERROR(strBuffer);
}

//--------------------------------------------------------------------------------------
// Cleans up the 3D environment by:
//      - Calls the device lost callback 
//      - Calls the device destroyed callback 
//      - Releases the D3D device
//--------------------------------------------------------------------------------------
static void __cleanup_3d_environment(const bool aReleaseSettings)
{
	if (win32_d3d9_state.m_d3d_device != nullptr)
	{
		win32_d3d9_state.m_inside.device_callback = true;

		// Call the app's device lost callback
		if (win32_d3d9_state.m_device_objects.reset == true)
		{
			win32_d3d9_resource_callback_on_lost_device();
			win32_d3d9_state.m_device_objects.reset = false;
		}

		// Call the app's device destroyed callback
		if (win32_d3d9_state.m_device_objects.created == true)
		{
			win32_d3d9_resource_callback_on_destroy_device();
			win32_d3d9_state.m_device_objects.created = false;
		}

		win32_d3d9_state.m_inside.device_callback = false;

		// Release the D3D device and in debug configs, displays a message box if there 
		// are unrelease objects.
		if (win32_d3d9_state.m_d3d_device)
		{
			if (win32_d3d9_state.m_d3d_device->Release() > 0)
			{
				__display_error_message(WIN32_DXUTERR_NONZEROREFCOUNT);
				WIN32_DXUT_ERR("__cleanup_3d_environment", WIN32_DXUTERR_NONZEROREFCOUNT);
			}
		}
		win32_d3d9_state.m_d3d_device = nullptr;

		if (aReleaseSettings)
		{
			win32_d3d9_device_settings_t* oldDeviceSettings = win32_d3d9_state.m_device_settings;
			SAFE_DELETE(oldDeviceSettings);
			win32_d3d9_state.m_device_settings = nullptr;
		}

		::ZeroMemory(&win32_d3d9_state.m_backbuffer_surface_desc, sizeof(::D3DSURFACE_DESC));

		::ZeroMemory(&win32_d3d9_state.m_caps, sizeof(::D3DCAPS9));

		win32_d3d9_state.m_device.created = false;
	}
}

//--------------------------------------------------------------------------------------
// Returns the number of color channel bits in the specified D3DFORMAT
//--------------------------------------------------------------------------------------
static const UINT __color_channel_bits(const ::D3DFORMAT aFormat)
{
	switch (aFormat)
	{
	case ::D3DFMT_R8G8B8:
		return 8;
	case ::D3DFMT_A8R8G8B8:
		return 8;
	case ::D3DFMT_X8R8G8B8:
		return 8;
	case ::D3DFMT_R5G6B5:
		return 5;
	case ::D3DFMT_X1R5G5B5:
		return 5;
	case ::D3DFMT_A1R5G5B5:
		return 5;
	case ::D3DFMT_A4R4G4B4:
		return 4;
	case ::D3DFMT_R3G3B2:
		return 2;
	case ::D3DFMT_A8R3G3B2:
		return 2;
	case ::D3DFMT_X4R4G4B4:
		return 4;
	case ::D3DFMT_A2B10G10R10:
		return 10;
	case ::D3DFMT_A8B8G8R8:
		return 8;
	case ::D3DFMT_A2R10G10B10:
		return 10;
	case ::D3DFMT_A16B16G16R16:
		return 16;
	default:
		return 0;
	}
}

//--------------------------------------------------------------------------------------
// Returns the number of stencil bits in the specified D3DFORMAT
//--------------------------------------------------------------------------------------
static const UINT __stencil_bits(const ::D3DFORMAT aFormat)
{
	switch (aFormat)
	{
	case ::D3DFMT_D16_LOCKABLE:
	case ::D3DFMT_D16:
	case ::D3DFMT_D32F_LOCKABLE:
	case ::D3DFMT_D32:
	case ::D3DFMT_D24X8:
		return 0;

	case ::D3DFMT_D15S1:
		return 1;

	case ::D3DFMT_D24X4S4:
		return 4;

	case ::D3DFMT_D24S8:
	case ::D3DFMT_D24FS8:
		return 8;

	default:
		return 0;
	}
}

//--------------------------------------------------------------------------------------
// Returns the number of depth bits in the specified D3DFORMAT
//--------------------------------------------------------------------------------------
static const UINT __depth_bits(const ::D3DFORMAT aFormat)
{
	switch (aFormat)
	{
	case ::D3DFMT_D32F_LOCKABLE:
	case ::D3DFMT_D32:
		return 32;

	case ::D3DFMT_D24X8:
	case ::D3DFMT_D24S8:
	case ::D3DFMT_D24X4S4:
	case ::D3DFMT_D24FS8:
		return 24;

	case ::D3DFMT_D16_LOCKABLE:
	case ::D3DFMT_D16:
		return 16;

	case ::D3DFMT_D15S1:
		return 15;

	default:
		return 0;
	}
}

//--------------------------------------------------------------------------------------
// Look for an adapter ordinal that is tied to a HMONITOR
//--------------------------------------------------------------------------------------
static ::HRESULT __get_adapter_ordinal_from_monitor(::HMONITOR aMonitor, ::UINT* anAdapterOrdinal)
{
	*anAdapterOrdinal = 0;

	const win32_d3d9_enumeration_t* ENUMERATION = __prepare_enumeration_object(false);

	const std::vector<win32_d3d9_enum_adapter_info_t*>* ADAPTER_LIST = &ENUMERATION->adapter_info_list;
	for (uint32_t iAdapter = 0; iAdapter < ADAPTER_LIST->size(); ++iAdapter)
	{
		const win32_d3d9_enum_adapter_info_t* ADAPTER_INFO = (*ADAPTER_LIST)[iAdapter];
		const HMONITOR hAdapterMonitor = win32_d3d9_state.m_d3d->GetAdapterMonitor(ADAPTER_INFO->adapter_ordinal);
		if (hAdapterMonitor == aMonitor)
		{
			*anAdapterOrdinal = ADAPTER_INFO->adapter_ordinal;
			return S_OK;
		}
	}

	return E_FAIL;
}

//--------------------------------------------------------------------------------------
// Enables/disables Windows keys, and disables or restores the StickyKeys/ToggleKeys/FilterKeys 
// shortcut to help prevent accidental task switching
//--------------------------------------------------------------------------------------
static void __allow_shortcut_keys(const bool anAllowKeys)
{
	//win32_d3d9_state.m_allow_shortcut_keys = anAllowKeys;

	if (anAllowKeys)
	{
		// Restore StickyKeys/etc to original win32_d3d9_state and enable Windows key      
		::SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &win32_d3d9_state.m_startup_keys.sticky, 0);
		::SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &win32_d3d9_state.m_startup_keys.toggle, 0);
		::SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &win32_d3d9_state.m_startup_keys.filter, 0);
	}
	else
	{
		// Disable StickyKeys/etc shortcuts but if the accessibility feature is on, 
		// then leave the settings alone as its probably being usefully used

		STICKYKEYS skOff = win32_d3d9_state.m_startup_keys.sticky;
		if ((skOff.dwFlags & SKF_STICKYKEYSON) == 0)
		{
			// Disable the hotkey and the confirmation
			skOff.dwFlags &= ~SKF_HOTKEYACTIVE;
			skOff.dwFlags &= ~SKF_CONFIRMHOTKEY;

			::SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &skOff, 0);
		}

		TOGGLEKEYS tkOff = win32_d3d9_state.m_startup_keys.toggle;
		if ((tkOff.dwFlags & TKF_TOGGLEKEYSON) == 0)
		{
			// Disable the hotkey and the confirmation
			tkOff.dwFlags &= ~TKF_HOTKEYACTIVE;
			tkOff.dwFlags &= ~TKF_CONFIRMHOTKEY;

			::SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tkOff, 0);
		}

		FILTERKEYS fkOff = win32_d3d9_state.m_startup_keys.filter;
		if ((fkOff.dwFlags & FKF_FILTERKEYSON) == 0)
		{
			// Disable the hotkey and the confirmation
			fkOff.dwFlags &= ~FKF_HOTKEYACTIVE;
			fkOff.dwFlags &= ~FKF_CONFIRMHOTKEY;

			::SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fkOff, 0);
		}
	}
}

//--------------------------------------------------------------------------------------
// Stores back buffer surface desc in state_t::instance().GetBackBufferSurfaceDesc()
//--------------------------------------------------------------------------------------
static void __update_back_buffer_desc()
{
	::HRESULT hr;
	::IDirect3DSurface9* pBackBuffer;
	hr = win32_d3d9_state.m_d3d_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
	::ZeroMemory(&win32_d3d9_state.m_backbuffer_surface_desc, sizeof(D3DSURFACE_DESC));
	if (SUCCEEDED(hr))
	{
		pBackBuffer->GetDesc(&win32_d3d9_state.m_backbuffer_surface_desc);
		SAFE_RELEASE(pBackBuffer);
	}
}

//--------------------------------------------------------------------------------------
// Builds valid device settings using the match options, the input device settings, and the 
// best device settings combo found.
//--------------------------------------------------------------------------------------
static void __build_valid_device_settings(win32_d3d9_device_settings_t* someValidDeviceSettings, const win32_d3d9_enum_device_settings_combo_t* aBestDeviceSettingsCombo, const win32_d3d9_device_settings_t* someDeviceSettingsIn, const match_options_t* someMatchOptions)
{
	::D3DDISPLAYMODE adapterDesktopDisplayMode;
	win32_d3d9_state.m_d3d->GetAdapterDisplayMode(aBestDeviceSettingsCombo->adapter_ordinal, &adapterDesktopDisplayMode);

	// For each setting pick the best, taking into account the match options and 
	// what's supported by the device

	//---------------------
	// Adapter Ordinal
	//---------------------
	// Just using pBestDeviceSettingsCombo->AdapterOrdinal

	//---------------------
	// Device Type
	//---------------------
	// Just using pBestDeviceSettingsCombo->DeviceType

	//---------------------
	// Windowed 
	//---------------------
	// Just using pBestDeviceSettingsCombo->Windowed

	//---------------------
	// Adapter format
	//---------------------
	// Just using pBestDeviceSettingsCombo->AdapterFormat

	//---------------------
	// Vertex processing
	//---------------------
	::DWORD dwBestBehaviorFlags = 0;
	if (someMatchOptions->vertex_processing == match_type_t::PRESERVE_INPUT)
	{
		dwBestBehaviorFlags = someDeviceSettingsIn->behavior_flags;
	}
	else if (someMatchOptions->vertex_processing == match_type_t::IGNORE_INPUT)
	{
		// The framework defaults to HWVP if available otherwise use SWVP
		if ((aBestDeviceSettingsCombo->device_info->caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
			dwBestBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
		else
			dwBestBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	else // if( someMatchOptions->eVertexProcessing == CLOSEST_TO_INPUT )    
	{
		// Default to input, and fallback to SWVP if HWVP not available 
		dwBestBehaviorFlags = someDeviceSettingsIn->behavior_flags;
		if ((aBestDeviceSettingsCombo->device_info->caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 &&
			((dwBestBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0 ||
				(dwBestBehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING) != 0))
		{
			dwBestBehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
			dwBestBehaviorFlags &= ~D3DCREATE_MIXED_VERTEXPROCESSING;
			dwBestBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}

		// One of these must be selected
		if ((dwBestBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) == 0 &&
			(dwBestBehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING) == 0 &&
			(dwBestBehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING) == 0)
		{
			if ((aBestDeviceSettingsCombo->device_info->caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
				dwBestBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
			else
				dwBestBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}
	}

	//---------------------
	// Resolution
	//---------------------
	D3DDISPLAYMODE bestDisplayMode;
	if (someMatchOptions->resolution == match_type_t::PRESERVE_INPUT)
	{
		bestDisplayMode.Width = someDeviceSettingsIn->present_parameters.BackBufferWidth;
		bestDisplayMode.Height = someDeviceSettingsIn->present_parameters.BackBufferHeight;
	}
	else
	{
		D3DDISPLAYMODE displayModeIn;
		if (someMatchOptions->resolution == match_type_t::CLOSEST_TO_INPUT && someDeviceSettingsIn)
		{
			displayModeIn.Width = someDeviceSettingsIn->present_parameters.BackBufferWidth;
			displayModeIn.Height = someDeviceSettingsIn->present_parameters.BackBufferHeight;
		}
		else // if( someMatchOptions->eResolution == IGNORE_INPUT )   
		{
			if (aBestDeviceSettingsCombo->windowed)
			{
				// The framework defaults to 640x480 for windowed
				displayModeIn.Width = 640;
				displayModeIn.Height = 480;
			}
			else
			{
				// The framework defaults to desktop resolution for fullscreen to try to avoid slow mode change
				displayModeIn.Width = adapterDesktopDisplayMode.Width;
				displayModeIn.Height = adapterDesktopDisplayMode.Height;
			}
		}

		// Call a helper function to find the closest valid display mode to the optimal 
		__find_valid_resolution(aBestDeviceSettingsCombo, displayModeIn, &bestDisplayMode);
	}

	//---------------------
	// Back Buffer format
	//---------------------
	// Just using pBestDeviceSettingsCombo->BackBufferFormat

	//---------------------
	// Back buffer count
	//---------------------
	UINT bestBackBufferCount;
	if (someMatchOptions->backbuffer_count == match_type_t::PRESERVE_INPUT)
	{
		bestBackBufferCount = someDeviceSettingsIn->present_parameters.BackBufferCount;
	}
	else if (someMatchOptions->backbuffer_count == match_type_t::IGNORE_INPUT)
	{
		// The framework defaults to triple buffering 
		bestBackBufferCount = 2;
	}
	else // if( someMatchOptions->eBackBufferCount == CLOSEST_TO_INPUT )   
	{
		bestBackBufferCount = someDeviceSettingsIn->present_parameters.BackBufferCount;
		if (bestBackBufferCount > 3)
			bestBackBufferCount = 3;
		if (bestBackBufferCount < 1)
			bestBackBufferCount = 1;
	}

	//---------------------
	// Multisample
	//---------------------
	::D3DMULTISAMPLE_TYPE bestMultiSampleType;
	::DWORD bestMultiSampleQuality;
	if (someDeviceSettingsIn && someDeviceSettingsIn->present_parameters.SwapEffect != D3DSWAPEFFECT_DISCARD)
	{
		// Swap effect is not set to discard so multisampling has to off
		bestMultiSampleType = D3DMULTISAMPLE_NONE;
		bestMultiSampleQuality = 0;
	}
	else
	{
		if (someMatchOptions->multi_sample == match_type_t::PRESERVE_INPUT)
		{
			bestMultiSampleType = someDeviceSettingsIn->present_parameters.MultiSampleType;
			bestMultiSampleQuality = someDeviceSettingsIn->present_parameters.MultiSampleQuality;
		}
		else if (someMatchOptions->multi_sample == match_type_t::IGNORE_INPUT)
		{
			// Default to no multisampling (always supported)
			bestMultiSampleType = D3DMULTISAMPLE_NONE;
			bestMultiSampleQuality = 0;
		}
		else if (someMatchOptions->multi_sample == match_type_t::CLOSEST_TO_INPUT)
		{
			// Default to no multisampling (always supported)
			bestMultiSampleType = D3DMULTISAMPLE_NONE;
			bestMultiSampleQuality = 0;

			for (uint32_t i = 0; i < aBestDeviceSettingsCombo->multisample_types.size(); ++i)
			{
				const D3DMULTISAMPLE_TYPE type = aBestDeviceSettingsCombo->multisample_types[i];
				const DWORD qualityLevels = aBestDeviceSettingsCombo->multisample_qualities[i];

				// Check whether supported type is closer to the input than our current best
				if (abs(type - someDeviceSettingsIn->present_parameters.MultiSampleType) < abs(bestMultiSampleType - someDeviceSettingsIn->present_parameters.MultiSampleType))
				{
					bestMultiSampleType = type;
					bestMultiSampleQuality = __min(qualityLevels - 1, someDeviceSettingsIn->present_parameters.MultiSampleQuality);
				}
			}
		}
		else
		{
			// Error case
			bestMultiSampleType = D3DMULTISAMPLE_NONE;
			bestMultiSampleQuality = 0;
		}
	}

	//---------------------
	// Swap effect
	//---------------------
	::D3DSWAPEFFECT bestSwapEffect;
	if (someMatchOptions->swap_effect == match_type_t::PRESERVE_INPUT)
	{
		bestSwapEffect = someDeviceSettingsIn->present_parameters.SwapEffect;
	}
	else if (someMatchOptions->swap_effect == match_type_t::IGNORE_INPUT)
	{
		bestSwapEffect = D3DSWAPEFFECT_DISCARD;
	}
	else // if( someMatchOptions->eSwapEffect == CLOSEST_TO_INPUT )   
	{
		bestSwapEffect = someDeviceSettingsIn->present_parameters.SwapEffect;

		// Swap effect has to be one of these 3
		if (bestSwapEffect != D3DSWAPEFFECT_DISCARD &&
			bestSwapEffect != D3DSWAPEFFECT_FLIP &&
			bestSwapEffect != D3DSWAPEFFECT_COPY)
		{
			bestSwapEffect = D3DSWAPEFFECT_DISCARD;
		}
	}

	//---------------------
	// Depth stencil 
	//---------------------
	::D3DFORMAT bestDepthStencilFormat;
	::BOOL bestEnableAutoDepthStencil;

	std::vector< int32_t > depthStencilRanking;
	depthStencilRanking.reserve(aBestDeviceSettingsCombo->depth_stencil_formats.size());

	const ::UINT dwBackBufferBitDepth = __color_channel_bits(aBestDeviceSettingsCombo->backbuffer_format);
	::UINT dwInputDepthBitDepth = 0;
	if (someDeviceSettingsIn)
		dwInputDepthBitDepth = __depth_bits(someDeviceSettingsIn->present_parameters.AutoDepthStencilFormat);

	for (uint32_t i = 0; i < aBestDeviceSettingsCombo->depth_stencil_formats.size(); ++i)
	{
		const ::D3DFORMAT curDepthStencilFmt = aBestDeviceSettingsCombo->depth_stencil_formats[i];
		const ::DWORD dwCurDepthBitDepth = __depth_bits(curDepthStencilFmt);
		int32_t nRanking;

		if (someMatchOptions->depth_format == match_type_t::PRESERVE_INPUT)
		{
			// Need to match bit depth of input
			if (dwCurDepthBitDepth == dwInputDepthBitDepth)
				nRanking = 0;
			else
				nRanking = 10000;
		}
		else if (someMatchOptions->depth_format == match_type_t::IGNORE_INPUT)
		{
			// Prefer match of backbuffer bit depth
			nRanking = abs((int32_t)dwCurDepthBitDepth - (int32_t)dwBackBufferBitDepth * 4);
		}
		else // if( someMatchOptions->eDepthFormat == CLOSEST_TO_INPUT )
		{
			// Prefer match of input depth format bit depth
			nRanking = abs((int32_t)dwCurDepthBitDepth - (int32_t)dwInputDepthBitDepth);
		}

		depthStencilRanking.push_back(nRanking);
	}

	::UINT dwInputStencilBitDepth = 0;
	if (someDeviceSettingsIn)
		dwInputStencilBitDepth = __stencil_bits(someDeviceSettingsIn->present_parameters.AutoDepthStencilFormat);

	for (uint32_t i = 0; i < aBestDeviceSettingsCombo->depth_stencil_formats.size(); ++i)
	{
		const ::D3DFORMAT curDepthStencilFmt = aBestDeviceSettingsCombo->depth_stencil_formats[i];
		int32_t nRanking = depthStencilRanking[i];
		const ::DWORD dwCurStencilBitDepth = __stencil_bits(curDepthStencilFmt);

		if (someMatchOptions->stencil_format == match_type_t::PRESERVE_INPUT)
		{
			// Need to match bit depth of input
			if (dwCurStencilBitDepth == dwInputStencilBitDepth)
				nRanking += 0;
			else
				nRanking += 10000;
		}
		else if (someMatchOptions->stencil_format == match_type_t::IGNORE_INPUT)
		{
			// Prefer 0 stencil bit depth
			nRanking += dwCurStencilBitDepth;
		}
		else // if( someMatchOptions->eStencilFormat == CLOSEST_TO_INPUT )
		{
			// Prefer match of input stencil format bit depth
			nRanking += abs((int32_t)dwCurStencilBitDepth - (int32_t)dwInputStencilBitDepth);
		}

		//depthStencilRanking.setAt( i, nRanking );
		depthStencilRanking[i] = nRanking;
	}

	{
		int32_t nBestRanking = 100000;
		int32_t nBestIndex = -1;
		for (uint32_t i = 0; i < aBestDeviceSettingsCombo->depth_stencil_formats.size(); ++i)
		{
			const int32_t RANKING = depthStencilRanking[i];
			if (RANKING < nBestRanking)
			{
				nBestRanking = RANKING;
				nBestIndex = i;
			}
		}

		if (nBestIndex >= 0)
		{
			bestDepthStencilFormat = aBestDeviceSettingsCombo->depth_stencil_formats[nBestIndex];
			bestEnableAutoDepthStencil = true;
		}
		else
		{
			bestDepthStencilFormat = D3DFMT_UNKNOWN;
			bestEnableAutoDepthStencil = false;
		}
	}


	//---------------------
	// Present flags
	//---------------------
	::DWORD dwBestFlags;
	if (someMatchOptions->present_flags == match_type_t::PRESERVE_INPUT)
	{
		dwBestFlags = someDeviceSettingsIn->present_parameters.Flags;
	}
	else if (someMatchOptions->present_flags == match_type_t::IGNORE_INPUT)
	{
		dwBestFlags = 0;
		if (bestEnableAutoDepthStencil)
			dwBestFlags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	}
	else // if( someMatchOptions->ePresentFlags == CLOSEST_TO_INPUT )   
	{
		dwBestFlags = someDeviceSettingsIn->present_parameters.Flags;
		if (bestEnableAutoDepthStencil)
			dwBestFlags |= D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	}

	//---------------------
	// refresh rate
	//---------------------
	if (aBestDeviceSettingsCombo->windowed)
	{
		// Must be 0 for windowed
		bestDisplayMode.RefreshRate = 0;
	}
	else
	{
		if (someMatchOptions->refresh_rate == match_type_t::PRESERVE_INPUT)
		{
			bestDisplayMode.RefreshRate = someDeviceSettingsIn->present_parameters.FullScreen_RefreshRateInHz;
		}
		else
		{
			::UINT refreshRateMatch;
			if (someMatchOptions->refresh_rate == match_type_t::CLOSEST_TO_INPUT)
			{
				refreshRateMatch = someDeviceSettingsIn->present_parameters.FullScreen_RefreshRateInHz;
			}
			else // if( someMatchOptions->eRefreshRate == IGNORE_INPUT )   
			{
				refreshRateMatch = adapterDesktopDisplayMode.RefreshRate;
			}

			bestDisplayMode.RefreshRate = 0;

			if (refreshRateMatch != 0)
			{
				int32_t nBestRefreshRanking = 100000;
				const std::vector<D3DDISPLAYMODE>* DISPLAY_MODE_LIST = &aBestDeviceSettingsCombo->adapter_info->display_mode_list;
				for (uint32_t iDisplayMode = 0; iDisplayMode < DISPLAY_MODE_LIST->size(); ++iDisplayMode)
				{
					const ::D3DDISPLAYMODE DISPLAY_MODE = (*DISPLAY_MODE_LIST)[iDisplayMode];
					if (DISPLAY_MODE.Format != aBestDeviceSettingsCombo->adapter_format ||
						DISPLAY_MODE.Height != bestDisplayMode.Height ||
						DISPLAY_MODE.Width != bestDisplayMode.Width)
						continue; // Skip display modes that don't match 

								  // find the delta between the current refresh rate and the optimal refresh rate 
					const int32_t nCurRanking = ::abs((int32_t)DISPLAY_MODE.RefreshRate - (int32_t)refreshRateMatch);

					if (nCurRanking < nBestRefreshRanking)
					{
						bestDisplayMode.RefreshRate = DISPLAY_MODE.RefreshRate;
						nBestRefreshRanking = nCurRanking;

						// stop if perfect match found
						if (0 == nBestRefreshRanking)
							break;
					}
				}
			}
		}
	}

	//---------------------
	// Present interval
	//---------------------
	::UINT bestPresentInterval;
	if (someMatchOptions->present_interval == match_type_t::PRESERVE_INPUT)
	{
		bestPresentInterval = someDeviceSettingsIn->present_parameters.PresentationInterval;
	}
	else if (someMatchOptions->present_interval == match_type_t::IGNORE_INPUT)
	{
		// For windowed and fullscreen, default to D3DPRESENT_INTERVAL_DEFAULT
		// which will wait for the vertical retrace period to prevent tearing.
		// For benchmarking, use D3DPRESENT_INTERVAL_DEFAULT  which will
		// will wait not for the vertical retrace period but may introduce tearing.
		bestPresentInterval = D3DPRESENT_INTERVAL_DEFAULT;
	}
	else
	{
		if (vector_contains(aBestDeviceSettingsCombo->present_intervals, someDeviceSettingsIn->present_parameters.PresentationInterval))
		{
			bestPresentInterval = someDeviceSettingsIn->present_parameters.PresentationInterval;
		}
		else
		{
			bestPresentInterval = D3DPRESENT_INTERVAL_DEFAULT;
		}
	}

	// Fill the device settings struct
	::ZeroMemory(someValidDeviceSettings, sizeof(win32_d3d9_device_settings_t));
	someValidDeviceSettings->adapter_ordinal = aBestDeviceSettingsCombo->adapter_ordinal;
	someValidDeviceSettings->device_type = aBestDeviceSettingsCombo->device_type;
	someValidDeviceSettings->adapter_format = aBestDeviceSettingsCombo->adapter_format;
	someValidDeviceSettings->behavior_flags = dwBestBehaviorFlags;
	someValidDeviceSettings->present_parameters.BackBufferWidth = bestDisplayMode.Width;
	someValidDeviceSettings->present_parameters.BackBufferHeight = bestDisplayMode.Height;
	someValidDeviceSettings->present_parameters.BackBufferFormat = aBestDeviceSettingsCombo->backbuffer_format;
	someValidDeviceSettings->present_parameters.BackBufferCount = bestBackBufferCount;
	someValidDeviceSettings->present_parameters.MultiSampleType = bestMultiSampleType;
	someValidDeviceSettings->present_parameters.MultiSampleQuality = bestMultiSampleQuality;
	someValidDeviceSettings->present_parameters.SwapEffect = bestSwapEffect;
	someValidDeviceSettings->present_parameters.hDeviceWindow = aBestDeviceSettingsCombo->windowed ? win32_d3d9_state.m_hwnd.device_windowed : win32_d3d9_state.m_hwnd.device_fullscreen;
	someValidDeviceSettings->present_parameters.Windowed = aBestDeviceSettingsCombo->windowed;
	someValidDeviceSettings->present_parameters.EnableAutoDepthStencil = bestEnableAutoDepthStencil;
	someValidDeviceSettings->present_parameters.AutoDepthStencilFormat = bestDepthStencilFormat;
	someValidDeviceSettings->present_parameters.Flags = dwBestFlags;
	someValidDeviceSettings->present_parameters.FullScreen_RefreshRateInHz = bestDisplayMode.RefreshRate;
	someValidDeviceSettings->present_parameters.PresentationInterval = bestPresentInterval;
}

//--------------------------------------------------------------------------------------
// Returns a ranking number that describes how closely this device 
// combo matches the optimal combo based on the match options and the optimal device settings
//--------------------------------------------------------------------------------------
static float __rank_device_combo(const win32_d3d9_enum_device_settings_combo_t* aDeviceSettingsCombo, const win32_d3d9_device_settings_t* someOptimalDeviceSettings, const ::D3DDISPLAYMODE* anAdapterDesktopDisplayMode)
{
	float fCurRanking = 0.0f;

	// Arbitrary weights.  Gives preference to the ordinal, device type, and windowed
	const float fAdapterOrdinalWeight = 1000.0f;
	const float fDeviceTypeWeight = 100.0f;
	const float fWindowWeight = 10.0f;
	const float fAdapterFormatWeight = 1.0f;
	const float fVertexProcessingWeight = 1.0f;
	const float fResolutionWeight = 1.0f;
	const float fBackBufferFormatWeight = 1.0f;
	const float fMultiSampleWeight = 1.0f;
	const float fDepthStencilWeight = 1.0f;
	const float fRefreshRateWeight = 1.0f;
	const float fPresentIntervalWeight = 1.0f;

	//---------------------
	// Adapter ordinal
	//---------------------
	if (aDeviceSettingsCombo->adapter_ordinal == someOptimalDeviceSettings->adapter_ordinal)
		fCurRanking += fAdapterOrdinalWeight;

	//---------------------
	// Device type
	//---------------------
	if (aDeviceSettingsCombo->device_type == someOptimalDeviceSettings->device_type)
		fCurRanking += fDeviceTypeWeight;
	// Slightly prefer HAL 
	if (aDeviceSettingsCombo->device_type == D3DDEVTYPE_HAL)
		fCurRanking += 0.1f;

	//---------------------
	// Windowed
	//---------------------
	if (aDeviceSettingsCombo->windowed == someOptimalDeviceSettings->present_parameters.Windowed)
		fCurRanking += fWindowWeight;

	//---------------------
	// Adapter format
	//---------------------
	if (aDeviceSettingsCombo->adapter_format == someOptimalDeviceSettings->adapter_format)
	{
		fCurRanking += fAdapterFormatWeight;
	}
	else
	{
		int32_t nBitDepthDelta = ::abs((long)__color_channel_bits(aDeviceSettingsCombo->adapter_format) - (long)__color_channel_bits(someOptimalDeviceSettings->adapter_format));
		float fScale = __max(0.9f - (float)nBitDepthDelta * 0.2f, 0.0f);
		fCurRanking += fScale * fAdapterFormatWeight;
	}

	if (!aDeviceSettingsCombo->windowed)
	{
		// Slightly prefer when it matches the desktop format or is D3DFMT_X8R8G8B8
		bool bAdapterOptimalMatch;
		if (__color_channel_bits(anAdapterDesktopDisplayMode->Format) >= 8)
			bAdapterOptimalMatch = (aDeviceSettingsCombo->adapter_format == anAdapterDesktopDisplayMode->Format);
		else
			bAdapterOptimalMatch = (aDeviceSettingsCombo->adapter_format == D3DFMT_X8R8G8B8);

		if (bAdapterOptimalMatch)
			fCurRanking += 0.1f;
	}

	//---------------------
	// Vertex processing
	//---------------------
	if ((someOptimalDeviceSettings->behavior_flags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0 ||
		(someOptimalDeviceSettings->behavior_flags & D3DCREATE_MIXED_VERTEXPROCESSING) != 0)
	{
		if ((aDeviceSettingsCombo->device_info->caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
			fCurRanking += fVertexProcessingWeight;
	}
	// Slightly prefer HW T&L
	if ((aDeviceSettingsCombo->device_info->caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
		fCurRanking += 0.1f;

	//---------------------
	// Resolution
	//---------------------
	bool bResolutionFound = false;
	for (uint32_t idm = 0; idm < aDeviceSettingsCombo->adapter_info->display_mode_list.size(); ++idm)
	{
		const ::D3DDISPLAYMODE DISPLAY_MODE = aDeviceSettingsCombo->adapter_info->display_mode_list[idm];
		if (DISPLAY_MODE.Format != aDeviceSettingsCombo->adapter_format)
			continue;
		if (DISPLAY_MODE.Width == someOptimalDeviceSettings->present_parameters.BackBufferWidth && DISPLAY_MODE.Height == someOptimalDeviceSettings->present_parameters.BackBufferHeight)
			bResolutionFound = true;
	}
	if (bResolutionFound)
		fCurRanking += fResolutionWeight;

	//---------------------
	// Back buffer format
	//---------------------
	if (aDeviceSettingsCombo->backbuffer_format == someOptimalDeviceSettings->present_parameters.BackBufferFormat)
	{
		fCurRanking += fBackBufferFormatWeight;
	}
	else
	{
		const int32_t nBitDepthDelta = ::abs((long)__color_channel_bits(aDeviceSettingsCombo->backbuffer_format) - (long)__color_channel_bits(someOptimalDeviceSettings->present_parameters.BackBufferFormat));
		const float fScale = __max(0.9f - (float)nBitDepthDelta * 0.2f, 0.0f);
		fCurRanking += fScale * fBackBufferFormatWeight;
	}

	// Check if this back buffer format is the same as 
	// the adapter format since this is preferred.
	const bool bAdapterMatchesBB = (aDeviceSettingsCombo->backbuffer_format == aDeviceSettingsCombo->adapter_format);
	if (bAdapterMatchesBB)
		fCurRanking += 0.1f;

	//---------------------
	// Back buffer count
	//---------------------
	// No caps for the back buffer count

	//---------------------
	// Multisample
	//---------------------
	bool bMultiSampleFound = false;
	for (uint32_t i = 0; i < aDeviceSettingsCombo->multisample_types.size(); ++i)
	{
		const ::D3DMULTISAMPLE_TYPE MS_TYPE = aDeviceSettingsCombo->multisample_types[i];
		const ::DWORD MS_QUALITY = aDeviceSettingsCombo->multisample_qualities[i];

		if (MS_TYPE == someOptimalDeviceSettings->present_parameters.MultiSampleType &&
			MS_QUALITY >= someOptimalDeviceSettings->present_parameters.MultiSampleQuality)
		{
			bMultiSampleFound = true;
			break;
		}
	}
	if (bMultiSampleFound)
		fCurRanking += fMultiSampleWeight;

	//---------------------
	// Swap effect
	//---------------------
	// No caps for swap effects

	//---------------------
	// Depth stencil 
	//---------------------
	if (vector_contains(aDeviceSettingsCombo->depth_stencil_formats, someOptimalDeviceSettings->present_parameters.AutoDepthStencilFormat))
		fCurRanking += fDepthStencilWeight;

	//---------------------
	// Present flags
	//---------------------
	// No caps for the present flags

	//---------------------
	// refresh rate
	//---------------------
	bool bRefreshFound = false;
	for (uint32_t idm = 0; idm < aDeviceSettingsCombo->adapter_info->display_mode_list.size(); ++idm)
	{
		const D3DDISPLAYMODE DISPLAY_MODE = aDeviceSettingsCombo->adapter_info->display_mode_list[idm];
		if (DISPLAY_MODE.Format != aDeviceSettingsCombo->adapter_format)
			continue;
		if (DISPLAY_MODE.RefreshRate == someOptimalDeviceSettings->present_parameters.FullScreen_RefreshRateInHz)
			bRefreshFound = true;
	}
	if (bRefreshFound)
		fCurRanking += fRefreshRateWeight;

	//---------------------
	// Present interval
	//---------------------
	// If keep present interval then check that the present interval is supported by this combo
	/*
	if( pDeviceSettingsCombo->presentIntervalList.contains( someOptimalDeviceSettings->myPresentParameters.PresentationInterval ) )
	fCurRanking += fPresentIntervalWeight;
	*/
	if (vector_contains(aDeviceSettingsCombo->present_intervals, someOptimalDeviceSettings->present_parameters.PresentationInterval))
		fCurRanking += fPresentIntervalWeight;

	return fCurRanking;
}

//--------------------------------------------------------------------------------------
// Returns false for any CD3DEnumDeviceSettingsCombo that doesn't meet the preserve 
// match options against the input someDeviceSettingsIn.
//--------------------------------------------------------------------------------------
static bool __does_device_combo_match_preserve_options(const win32_d3d9_enum_device_settings_combo_t* aDeviceSettingsCombo, const win32_d3d9_device_settings_t* someDeviceSettingsIn, const match_options_t* someMatchOptions)
{
	//---------------------
	// Adapter ordinal
	//---------------------
	if (someMatchOptions->adapter_ordinal == match_type_t::PRESERVE_INPUT && (aDeviceSettingsCombo->adapter_ordinal != someDeviceSettingsIn->adapter_ordinal))
		return false;

	//---------------------
	// Device type
	//---------------------
	if (someMatchOptions->device_type == match_type_t::PRESERVE_INPUT && (aDeviceSettingsCombo->device_type != someDeviceSettingsIn->device_type))
		return false;

	//---------------------
	// Windowed
	//---------------------
	if (someMatchOptions->windowed == match_type_t::PRESERVE_INPUT && (aDeviceSettingsCombo->windowed != someDeviceSettingsIn->present_parameters.Windowed))
		return false;

	//---------------------
	// Adapter format
	//---------------------
	if (someMatchOptions->adapter_format == match_type_t::PRESERVE_INPUT && (aDeviceSettingsCombo->adapter_format != someDeviceSettingsIn->adapter_format))
		return false;

	//---------------------
	// Vertex processing
	//---------------------
	// If keep VP and input has HWVP, then skip if this combo doesn't have HWTL 
	if (someMatchOptions->vertex_processing == match_type_t::PRESERVE_INPUT &&
		((someDeviceSettingsIn->behavior_flags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0) &&
		((aDeviceSettingsCombo->device_info->caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0))
		return false;

	//---------------------
	// Resolution
	//---------------------
	// If keep resolution then check that width and height supported by this combo
	if (someMatchOptions->resolution == match_type_t::PRESERVE_INPUT)
	{
		bool bFound = false;
		for (uint32_t i = 0; i < aDeviceSettingsCombo->adapter_info->display_mode_list.size(); ++i)
		{
			const ::D3DDISPLAYMODE DISPLAY_MODE = aDeviceSettingsCombo->adapter_info->display_mode_list[i];
			if (DISPLAY_MODE.Format != aDeviceSettingsCombo->adapter_format)
				continue; // Skip this display mode if it doesn't match the combo's adapter format

			if (DISPLAY_MODE.Width == someDeviceSettingsIn->present_parameters.BackBufferWidth &&
				DISPLAY_MODE.Height == someDeviceSettingsIn->present_parameters.BackBufferHeight)
			{
				bFound = true;
				break;
			}
		}

		// If the width and height are not supported by this combo, return false
		if (!bFound)
			return false;
	}

	//---------------------
	// Back buffer format
	//---------------------
	if (someMatchOptions->backbuffer_format == match_type_t::PRESERVE_INPUT && aDeviceSettingsCombo->backbuffer_format != someDeviceSettingsIn->present_parameters.BackBufferFormat)
		return false;

	//---------------------
	// Back buffer count
	//---------------------
	// No caps for the back buffer count

	//---------------------
	// Multisample
	//---------------------
	if (someMatchOptions->multi_sample == match_type_t::PRESERVE_INPUT)
	{
		bool bFound = false;
		for (uint32_t i = 0; i < aDeviceSettingsCombo->multisample_types.size(); ++i)
		{
			const ::D3DMULTISAMPLE_TYPE MS_TYPE = aDeviceSettingsCombo->multisample_types[i];
			const ::DWORD MS_QUALITY = aDeviceSettingsCombo->multisample_qualities[i];

			if (MS_TYPE == someDeviceSettingsIn->present_parameters.MultiSampleType && MS_QUALITY >= someDeviceSettingsIn->present_parameters.MultiSampleQuality)
			{
				bFound = true;
				break;
			}
		}

		// If multisample type/quality not supported by this combo, then return false
		if (!bFound)
			return false;
	}

	//---------------------
	// Swap effect
	//---------------------
	// No caps for swap effects

	//---------------------
	// Depth stencil 
	//---------------------
	// If keep depth stencil format then check that the depth stencil format is supported by this combo
	if (someMatchOptions->depth_format == match_type_t::PRESERVE_INPUT && someMatchOptions->stencil_format == match_type_t::PRESERVE_INPUT)
	{
		if (someDeviceSettingsIn->present_parameters.AutoDepthStencilFormat != D3DFMT_UNKNOWN &&
			!vector_contains(aDeviceSettingsCombo->depth_stencil_formats, someDeviceSettingsIn->present_parameters.AutoDepthStencilFormat))
			return false;
	}

	// If keep depth format then check that the depth format is supported by this combo
	if (someMatchOptions->depth_format == match_type_t::PRESERVE_INPUT && someDeviceSettingsIn->present_parameters.AutoDepthStencilFormat != D3DFMT_UNKNOWN)
	{
		bool bFound = false;
		const ::UINT DEPTH_BITS = __depth_bits(someDeviceSettingsIn->present_parameters.AutoDepthStencilFormat);
		for (uint32_t i = 0; i < aDeviceSettingsCombo->depth_stencil_formats.size(); ++i)
		{
			const ::D3DFORMAT DEPTH_STENCIL_FORMAT = aDeviceSettingsCombo->depth_stencil_formats[i];
			const ::UINT CURRENT_DEPTH_BITS = __depth_bits(DEPTH_STENCIL_FORMAT);
			if (CURRENT_DEPTH_BITS - DEPTH_BITS == 0)
				bFound = true;
		}

		if (!bFound)
			return false;
	}

	// If keep depth format then check that the depth format is supported by this combo
	if (someMatchOptions->stencil_format == match_type_t::PRESERVE_INPUT && someDeviceSettingsIn->present_parameters.AutoDepthStencilFormat != D3DFMT_UNKNOWN)
	{
		bool bFound = false;
		const UINT STENCIL_BITS = __stencil_bits(someDeviceSettingsIn->present_parameters.AutoDepthStencilFormat);
		for (uint32_t i = 0; i < aDeviceSettingsCombo->depth_stencil_formats.size(); i++)
		{
			const D3DFORMAT DEPTH_STENCIL_FORMAT = aDeviceSettingsCombo->depth_stencil_formats[i];
			const UINT CURRENT_STENCIL_BITS = __stencil_bits(DEPTH_STENCIL_FORMAT);
			if (CURRENT_STENCIL_BITS - STENCIL_BITS == 0)
				bFound = true;
		}

		if (!bFound)
			return false;
	}

	//---------------------
	// Present flags
	//---------------------
	// No caps for the present flags

	//---------------------
	// refresh rate
	//---------------------
	// If keep refresh rate then check that the resolution is supported by this combo
	if (someMatchOptions->refresh_rate == match_type_t::PRESERVE_INPUT)
	{
		bool bFound = false;
		for (uint32_t i = 0; i < aDeviceSettingsCombo->adapter_info->display_mode_list.size(); ++i)
		{
			const ::D3DDISPLAYMODE DISPLAY_MODE = aDeviceSettingsCombo->adapter_info->display_mode_list[i];
			if (DISPLAY_MODE.Format != aDeviceSettingsCombo->adapter_format)
				continue;
			if (DISPLAY_MODE.RefreshRate == someDeviceSettingsIn->present_parameters.FullScreen_RefreshRateInHz)
			{
				bFound = true;
				break;
			}
		}

		// If refresh rate not supported by this combo, then return false
		if (!bFound)
			return false;
	}

	//---------------------
	// Present interval
	//---------------------
	// If keep present interval then check that the present interval is supported by this combo
	if (someMatchOptions->present_interval == match_type_t::PRESERVE_INPUT &&
		!vector_contains(aDeviceSettingsCombo->present_intervals, someDeviceSettingsIn->present_parameters.PresentationInterval))
		return false;

	return true;
}

//--------------------------------------------------------------------------------------
// Internal helper function to build a device settings structure based upon the match 
// options.  If the match option is set to ignore, then a optimal default value is used.
// The default value may not exist on the system, but later this will be taken 
// into account.
//--------------------------------------------------------------------------------------
static void __build_optimal_device_settings(win32_d3d9_device_settings_t* someOptimalDeviceSettings, const win32_d3d9_device_settings_t* someDeviceSettingsIn, const match_options_t* someMatchOptions)
{
	::D3DDISPLAYMODE adapterDesktopDisplayMode;

	::ZeroMemory(someOptimalDeviceSettings, sizeof(win32_d3d9_device_settings_t));

	//---------------------
	// Adapter ordinal
	//---------------------    
	if (someMatchOptions->adapter_ordinal == match_type_t::IGNORE_INPUT)
		someOptimalDeviceSettings->adapter_ordinal = D3DADAPTER_DEFAULT;
	else
		someOptimalDeviceSettings->adapter_ordinal = someDeviceSettingsIn->adapter_ordinal;

	//---------------------
	// Device type
	//---------------------
	if (someMatchOptions->device_type == match_type_t::IGNORE_INPUT)
		someOptimalDeviceSettings->device_type = D3DDEVTYPE_HAL;
	else
		someOptimalDeviceSettings->device_type = someDeviceSettingsIn->device_type;

	//---------------------
	// Windowed
	//---------------------
	if (someMatchOptions->windowed == match_type_t::IGNORE_INPUT)
		someOptimalDeviceSettings->present_parameters.Windowed = TRUE;
	else
		someOptimalDeviceSettings->present_parameters.Windowed = someDeviceSettingsIn->present_parameters.Windowed;

	//---------------------
	// Adapter format
	//---------------------
	if (someMatchOptions->adapter_format == match_type_t::IGNORE_INPUT)
	{
		// If windowed, default to the desktop display mode
		// If fullscreen, default to the desktop display mode for quick mode change or 
		// default to D3DFMT_X8R8G8B8 if the desktop display mode is < 32bit
		win32_d3d9_state.m_d3d->GetAdapterDisplayMode(someOptimalDeviceSettings->adapter_ordinal, &adapterDesktopDisplayMode);
		if (someOptimalDeviceSettings->present_parameters.Windowed || __color_channel_bits(adapterDesktopDisplayMode.Format) >= 8)
			someOptimalDeviceSettings->adapter_format = adapterDesktopDisplayMode.Format;
		else
			someOptimalDeviceSettings->adapter_format = D3DFMT_X8R8G8B8;
	}
	else
	{
		someOptimalDeviceSettings->adapter_format = someDeviceSettingsIn->adapter_format;
	}

	//---------------------
	// Vertex processing
	//---------------------
	if (someMatchOptions->vertex_processing == match_type_t::IGNORE_INPUT)
		someOptimalDeviceSettings->behavior_flags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		someOptimalDeviceSettings->behavior_flags = someDeviceSettingsIn->behavior_flags;

	//---------------------
	// Resolution
	//---------------------
	if (someMatchOptions->resolution == match_type_t::IGNORE_INPUT)
	{
		// If windowed, default to 640x480
		// If fullscreen, default to the desktop res for quick mode change
		if (someOptimalDeviceSettings->present_parameters.Windowed)
		{
			someOptimalDeviceSettings->present_parameters.BackBufferWidth = 640;
			someOptimalDeviceSettings->present_parameters.BackBufferHeight = 480;
		}
		else
		{
			win32_d3d9_state.m_d3d->GetAdapterDisplayMode(someOptimalDeviceSettings->adapter_ordinal, &adapterDesktopDisplayMode);
			someOptimalDeviceSettings->present_parameters.BackBufferWidth = adapterDesktopDisplayMode.Width;
			someOptimalDeviceSettings->present_parameters.BackBufferHeight = adapterDesktopDisplayMode.Height;
		}
	}
	else
	{
		someOptimalDeviceSettings->present_parameters.BackBufferWidth = someDeviceSettingsIn->present_parameters.BackBufferWidth;
		someOptimalDeviceSettings->present_parameters.BackBufferHeight = someDeviceSettingsIn->present_parameters.BackBufferHeight;
	}

	//---------------------
	// Back buffer format
	//---------------------
	if (someMatchOptions->backbuffer_format == match_type_t::IGNORE_INPUT)
		someOptimalDeviceSettings->present_parameters.BackBufferFormat = someOptimalDeviceSettings->adapter_format; // Default to match the adapter format
	else
		someOptimalDeviceSettings->present_parameters.BackBufferFormat = someDeviceSettingsIn->present_parameters.BackBufferFormat;

	//---------------------
	// Back buffer count
	//---------------------
	if (someMatchOptions->backbuffer_count == match_type_t::IGNORE_INPUT)
		someOptimalDeviceSettings->present_parameters.BackBufferCount = 2; // Default to triple buffering for perf gain
	else
		someOptimalDeviceSettings->present_parameters.BackBufferCount = someDeviceSettingsIn->present_parameters.BackBufferCount;

	//---------------------
	// Multisample
	//---------------------
	if (someMatchOptions->multi_sample == match_type_t::IGNORE_INPUT)
	{
		// Default to no multisampling 
		someOptimalDeviceSettings->present_parameters.MultiSampleType = D3DMULTISAMPLE_NONE;
		someOptimalDeviceSettings->present_parameters.MultiSampleQuality = 0;
	}
	else
	{
		someOptimalDeviceSettings->present_parameters.MultiSampleType = someDeviceSettingsIn->present_parameters.MultiSampleType;
		someOptimalDeviceSettings->present_parameters.MultiSampleQuality = someDeviceSettingsIn->present_parameters.MultiSampleQuality;
	}

	//---------------------
	// Swap effect
	//---------------------
	if (someMatchOptions->swap_effect == match_type_t::IGNORE_INPUT)
		someOptimalDeviceSettings->present_parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	else
		someOptimalDeviceSettings->present_parameters.SwapEffect = someDeviceSettingsIn->present_parameters.SwapEffect;

	//---------------------
	// Depth stencil 
	//---------------------
	if (someMatchOptions->depth_format == match_type_t::IGNORE_INPUT && someMatchOptions->stencil_format == match_type_t::IGNORE_INPUT)
	{
		UINT nBackBufferBits = __color_channel_bits(someOptimalDeviceSettings->present_parameters.BackBufferFormat);
		if (nBackBufferBits >= 8)
			someOptimalDeviceSettings->present_parameters.AutoDepthStencilFormat = D3DFMT_D32;
		else
			someOptimalDeviceSettings->present_parameters.AutoDepthStencilFormat = D3DFMT_D16;
	}
	else
	{
		someOptimalDeviceSettings->present_parameters.AutoDepthStencilFormat = someDeviceSettingsIn->present_parameters.AutoDepthStencilFormat;
	}

	//---------------------
	// Present flags
	//---------------------
	if (someMatchOptions->present_flags == match_type_t::IGNORE_INPUT)
		someOptimalDeviceSettings->present_parameters.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	else
		someOptimalDeviceSettings->present_parameters.Flags = someDeviceSettingsIn->present_parameters.Flags;

	//---------------------
	// refresh rate
	//---------------------
	if (someMatchOptions->refresh_rate == match_type_t::IGNORE_INPUT)
		someOptimalDeviceSettings->present_parameters.FullScreen_RefreshRateInHz = 0;
	else
		someOptimalDeviceSettings->present_parameters.FullScreen_RefreshRateInHz = someDeviceSettingsIn->present_parameters.FullScreen_RefreshRateInHz;

	//---------------------
	// Present interval
	//---------------------
	if (someMatchOptions->present_interval == match_type_t::IGNORE_INPUT)
	{
		// For windowed and fullscreen, default to D3DPRESENT_INTERVAL_DEFAULT
		// which will wait for the vertical retrace period to prevent tearing.
		// For benchmarking, use D3DPRESENT_INTERVAL_DEFAULT  which will
		// will wait not for the vertical retrace period but may introduce tearing.
		someOptimalDeviceSettings->present_parameters.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	}
	else
	{
		someOptimalDeviceSettings->present_parameters.PresentationInterval = someDeviceSettingsIn->present_parameters.PresentationInterval;
	}
}

//--------------------------------------------------------------------------------------
// This function tries to find valid device settings based upon the input device settings 
// struct and the match options.  For each device setting a match option in the 
// match_options_t struct specifies how the function makes decisions.  For example, if 
// the caller wants a HAL device with a back buffer format of D3DFMT_A2B10G10R10 but the 
// HAL device on the system does not support D3DFMT_A2B10G10R10 however a REF device is 
// installed that does, then the function has a choice to either use REF or to change to 
// a back buffer format to compatible with the HAL device.  The match options lets the 
// caller control how these choices are made.
//
// Each match option must be one of the following types: 
//      IGNORE_INPUT: Uses the closest valid value to a default 
//      PRESERVE_INPUT: Uses the input without change, but may cause no valid device to be found
//      CLOSEST_TO_INPUT: Uses the closest valid value to the input 
//
// If someMatchOptions is nullptr then, all of the match options are assumed to be IGNORE_INPUT.  
// The function returns failure if no valid device settings can be found otherwise 
// the function returns success and the valid device settings are written to pOut.
//--------------------------------------------------------------------------------------
static HRESULT __find_valid_device_settings(win32_d3d9_device_settings_t* someSettingsOut, const win32_d3d9_device_settings_t* someSettingsIn, const match_options_t* someMatchOptions)
{
	if (someSettingsOut == nullptr)
		return WIN32_DXUT_ERR_MSGBOX("FindValidDeviceSettings", E_INVALIDARG);

	const win32_d3d9_enumeration_t* ENUMERATION = __prepare_enumeration_object(false);

	// Default to IGNORE_INPUT for everything unless someMatchOptions isn't nullptr
	match_options_t defaultMatchOptions;
	if (nullptr == someMatchOptions)
	{
		::ZeroMemory(&defaultMatchOptions, sizeof(match_options_t));
		someMatchOptions = &defaultMatchOptions;
	}

	// build an optimal device settings structure based upon the match 
	// options.  If the match option is set to ignore, then a optimal default value is used.
	// The default value may not exist on the system, but later this will be taken 
	// into account.
	win32_d3d9_device_settings_t optimalDeviceSettings;
	__build_optimal_device_settings(&optimalDeviceSettings, someSettingsIn, someMatchOptions);

	// find the best combination of:
	//      Adapter Ordinal
	//      Device Type
	//      Adapter format
	//      Back Buffer format
	//      Windowed
	// given what's available on the system and the match options combined with the device settings input.
	// This combination of settings is encapsulated by the CD3DEnumDeviceSettingsCombo struct.
	float fBestRanking = -1.0f;
	const win32_d3d9_enum_device_settings_combo_t* BEST_DEVICE_SETTINGS_COMBO = nullptr;
	::D3DDISPLAYMODE adapterDesktopDisplayMode;

	const std::vector<win32_d3d9_enum_adapter_info_t*>* ADAPTER_LIST = &ENUMERATION->adapter_info_list;
	for (uint32_t iAdapter = 0; iAdapter < ADAPTER_LIST->size(); ++iAdapter)
	{
		const win32_d3d9_enum_adapter_info_t* ADAPTER_INFO = (*ADAPTER_LIST)[iAdapter];

		// get the desktop display mode of adapter 
		win32_d3d9_state.m_d3d->GetAdapterDisplayMode(ADAPTER_INFO->adapter_ordinal, &adapterDesktopDisplayMode);
		win32_d3d9_state.desktop_resolution = { adapterDesktopDisplayMode.Width, adapterDesktopDisplayMode.Height };

		// Enum all the device types supported by this adapter to find the best device settings
		for (uint32_t iDeviceInfo = 0; iDeviceInfo < ADAPTER_INFO->device_info_list.size(); ++iDeviceInfo)
		{
			const win32_d3d9_enum_device_info_t* DEVICE_INFO = ADAPTER_INFO->device_info_list[iDeviceInfo];

			// Enum all the device settings combinations.  A device settings combination is 
			// a unique set of an adapter format, back buffer format, and IsWindowed.
			for (uint32_t iDeviceCombo = 0; iDeviceCombo < DEVICE_INFO->device_settings_combo_list.size(); ++iDeviceCombo)
			{
				const win32_d3d9_enum_device_settings_combo_t* DEVICE_SETTINGS_COMBO = DEVICE_INFO->device_settings_combo_list[iDeviceCombo];

				// If windowed mode the adapter format has to be the same as the desktop 
				// display mode format so skip any that don't match
				if (DEVICE_SETTINGS_COMBO->windowed && (DEVICE_SETTINGS_COMBO->adapter_format != adapterDesktopDisplayMode.Format))
					continue;

				// Skip any combo that doesn't meet the preserve match options
				if (!__does_device_combo_match_preserve_options(DEVICE_SETTINGS_COMBO, someSettingsIn, someMatchOptions))
					continue;

				// get a ranking number that describes how closely this device combo matches the optimal combo
				float fCurRanking = __rank_device_combo(DEVICE_SETTINGS_COMBO, &optimalDeviceSettings, &adapterDesktopDisplayMode);

				// If this combo better matches the input device settings then save it
				if (fCurRanking > fBestRanking)
				{
					BEST_DEVICE_SETTINGS_COMBO = DEVICE_SETTINGS_COMBO;
					fBestRanking = fCurRanking;
				}
			}
		}
	}

	// If no best device combination was found then fail
	if (BEST_DEVICE_SETTINGS_COMBO == nullptr)
		return WIN32_DXUTERR_NOCOMPATIBLEDEVICES;

	// using the best device settings combo found, build valid device settings taking heed of 
	// the match options and the input device settings
	win32_d3d9_device_settings_t validDeviceSettings;
	__build_valid_device_settings(&validDeviceSettings, BEST_DEVICE_SETTINGS_COMBO, someSettingsIn, someMatchOptions);
	*someSettingsOut = validDeviceSettings;

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Setup cursor based on current settings (window/fullscreen mode, show cursor win32_d3d9_state, clip cursor win32_d3d9_state)
//--------------------------------------------------------------------------------------
static void __setup_cursor()
{
	// Show the cursor again if returning to fullscreen 
	if (!win32_d3d9_is_windowed() && win32_d3d9_state.m_d3d_device)
	{
		if (win32_d3d9_state.m_show_cursor_when_fullscreen)
		{
			::SetCursor(::LoadCursor(nullptr, IDC_ARROW));

			win32_d3d9_state.m_d3d_device->ShowCursor(true);
		}
		else
		{
			::SetCursor(nullptr); // Turn off Windows cursor in full screen mode
			win32_d3d9_state.m_d3d_device->ShowCursor(false);
		}
	}

	// Clip cursor if requested
	if (!win32_d3d9_is_windowed() && win32_d3d9_state.m_clip_cursor_when_fullscreen)
	{
		// Confine cursor to full screen window
		::RECT rcWindow;
		::GetWindowRect(win32_d3d9_state.m_hwnd.device_fullscreen, &rcWindow);
		::ClipCursor(&rcWindow);
	}
	else
	{
		::ClipCursor(nullptr);
	}
}

//--------------------------------------------------------------------------------------
// Resets the 3D environment by:
//      - Calls the device lost callback 
//      - Resets the device
//      - Stores the back buffer description
//      - Sets up the full screen Direct3D cursor if requested
//      - Calls the device reset callback 
//--------------------------------------------------------------------------------------
static ::HRESULT __reset_3d_environment()
{
	::HRESULT hr;

	assert(win32_d3d9_state.m_d3d_device != nullptr);

	// Call the app's device lost callback
	if (win32_d3d9_state.m_device_objects.reset == true)
	{
		win32_d3d9_state.m_inside.device_callback = true;
		win32_d3d9_resource_callback_on_lost_device();
		win32_d3d9_state.m_device_objects.reset = false;
		win32_d3d9_state.m_inside.device_callback = false;
	}

	// reset the device
	win32_d3d9_device_settings_t* deviceSettings = win32_d3d9_state.m_device_settings;
	hr = win32_d3d9_state.m_d3d_device->Reset(&deviceSettings->present_parameters);
	if (FAILED(hr))
	{
		if (hr == D3DERR_DEVICELOST)
			return D3DERR_DEVICELOST; // reset could legitimately fail if the device is lost
		else
			return WIN32_DXUT_ERR("reset", WIN32_DXUTERR_RESETTINGDEVICE);
	}

	// update back buffer desc before calling app's device callbacks
	__update_back_buffer_desc();

	// Setup cursor based on current settings (window/fullscreen mode, show cursor win32_d3d9_state, clip cursor win32_d3d9_state)
	__setup_cursor();

	// Call the app's OnDeviceReset callback
	win32_d3d9_state.m_inside.device_callback = true;
	//const D3DSURFACE_DESC* pbackBufferSurfaceDesc = s.getBackBufferSurfaceDesc();
	hr = win32_d3d9_resource_callback_on_reset_device();
	win32_d3d9_state.m_inside.device_callback = false;
	if (FAILED(hr))
	{
		// If callback failed, cleanup
		WIN32_DXUT_ERR("DeviceResetCallback", hr);
		if (hr != WIN32_DXUTERR_MEDIANOTFOUND)
			hr = WIN32_DXUTERR_RESETTINGDEVICEOBJECTS;

		win32_d3d9_state.m_inside.device_callback = true;
		win32_d3d9_resource_callback_on_lost_device();
		win32_d3d9_state.m_inside.device_callback = false;

		//GetGlobalResourceCache().onLostDevice();       
		return hr;
	}

	// Success
	win32_d3d9_state.m_device_objects.reset = true;

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Creates the 3D environment
//--------------------------------------------------------------------------------------
static ::HRESULT __create_3d_environment(::IDirect3DDevice9* aDeviceFromApp)
{
	::HRESULT hr = S_OK;

	assert(nullptr == win32_d3d9_state.m_d3d_device);
	win32_d3d9_device_settings_t* newDeviceSettings = win32_d3d9_state.m_device_settings;

	// Only create a Direct3D device if one hasn't been supplied by the app
	if (aDeviceFromApp == nullptr)
	{
		// Try to create the device with the chosen settings
		hr = win32_d3d9_state.m_d3d->CreateDevice(
			newDeviceSettings->adapter_ordinal,
			newDeviceSettings->device_type,
			win32_d3d9_state.m_hwnd.focus,
			newDeviceSettings->behavior_flags,
			&newDeviceSettings->present_parameters,
			&win32_d3d9_state.m_d3d_device
		);
		if (hr == D3DERR_DEVICELOST)
		{
			win32_d3d9_state.m_device_lost = true;
			return S_OK;
		}
		else if (FAILED(hr))
		{
			WIN32_DXUT_ERR("CreateDevice", hr);
			return WIN32_DXUTERR_CREATINGDEVICE;
		}
	}
	else
	{
		aDeviceFromApp->AddRef();
		win32_d3d9_state.m_d3d_device = aDeviceFromApp;
	}

	assert(win32_d3d9_state.m_d3d_device);

	// If switching to REF, set the exit code to 11.  If switching to HAL and exit code was 11, then set it back to 0.
	if (newDeviceSettings->device_type == D3DDEVTYPE_REF && win32_d3d9_state.m_exit_code == 0)
		win32_d3d9_state.m_exit_code = 11;
	else if (newDeviceSettings->device_type == D3DDEVTYPE_HAL && win32_d3d9_state.m_exit_code == 11)
		win32_d3d9_state.m_exit_code = 0;

	// update back buffer desc before calling app's device callbacks
	__update_back_buffer_desc();

	// Setup cursor based on current settings (window/fullscreen mode, show cursor win32_d3d9_state, clip cursor win32_d3d9_state)
	__setup_cursor();

	// update s's copy of D3D caps 
	win32_d3d9_state.m_d3d_device->GetDeviceCaps(&win32_d3d9_state.m_caps);

	// update the device stats text
	win32_d3d9_enumeration_t* enumeration = __prepare_enumeration_object(false);
	const win32_d3d9_enum_adapter_info_t* ADAPTER_INFO = win32_d3d9_get_adapter_info(newDeviceSettings->adapter_ordinal, *enumeration);
	__update_device_stats(newDeviceSettings->device_type, newDeviceSettings->behavior_flags, &ADAPTER_INFO->adapter_identifier);

	// Call the app's device created callback if non-nullptr
	//const D3DSURFACE_DESC* pbackBufferSurfaceDesc = s.getBackBufferSurfaceDesc();
	win32_d3d9_state.m_inside.device_callback = true;
	hr = win32_d3d9_resource_callback_on_create_device();
	win32_d3d9_state.m_inside.device_callback = false;
	if (win32_d3d9_state.m_d3d_device == nullptr) // Handle shutdown from inside callback
		return E_FAIL;
	if (FAILED(hr))
	{
		WIN32_DXUT_ERR("DeviceCreated callback", hr);
		return (hr == WIN32_DXUTERR_MEDIANOTFOUND) ? WIN32_DXUTERR_MEDIANOTFOUND : WIN32_DXUTERR_CREATINGDEVICEOBJECTS;
	}
	win32_d3d9_state.m_device_objects.created = true;

	// Call the app's device reset callback if non-nullptr
	win32_d3d9_state.m_inside.device_callback = true;
	hr = win32_d3d9_resource_callback_on_reset_device();
	win32_d3d9_state.m_inside.device_callback = false;
	if (win32_d3d9_state.m_d3d_device == nullptr) // Handle shutdown from inside callback
		return E_FAIL;
	if (FAILED(hr))
	{
		WIN32_DXUT_ERR("DeviceReset callback", hr);
		return (hr == WIN32_DXUTERR_MEDIANOTFOUND) ? WIN32_DXUTERR_MEDIANOTFOUND : WIN32_DXUTERR_RESETTINGDEVICEOBJECTS;
	}
	win32_d3d9_state.m_device_objects.reset = true;

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Return the device settings of the current device.  If no device exists yet, then
// return blank device settings 
//--------------------------------------------------------------------------------------
static win32_d3d9_device_settings_t __get_device_settings()
{
	const win32_d3d9_device_settings_t* DEVICE_SETTINGS = win32_d3d9_state.m_device_settings;
	if (DEVICE_SETTINGS)
		return *DEVICE_SETTINGS;

	win32_d3d9_device_settings_t ds;
	::ZeroMemory(&ds, sizeof(win32_d3d9_device_settings_t));
	return ds;
}

//--------------------------------------------------------------------------------------
// Change to a Direct3D device created from the device settings or passed in.
// The framework will only reset if the device is similar to the previous device 
// otherwise it will cleanup the previous device (if there is one) and recreate the 
// scene using the app's device callbacks.
//--------------------------------------------------------------------------------------
static ::HRESULT __change_device(win32_d3d9_device_settings_t* pNewDeviceSettings, ::IDirect3DDevice9* aDeviceFromApp, bool aForceRecreate, bool aClipWindowToSingleAdapter)
{
	/*
	{
		const enum_adapter_info_t* INFO = get_adapter_info(pNewDeviceSettings->adapter_ordinal, *enumeration_t::instance());
		if (INFO)
			FS_LOG("selecting adapter ordinal %u = %s", pNewDeviceSettings->adapter_ordinal, INFO->description);
	}
	*/

	::HRESULT hr;
	win32_d3d9_device_settings_t* oldDeviceSettings = win32_d3d9_state.m_device_settings;

	if (win32_d3d9_state.m_d3d == nullptr)
		return S_FALSE;

	// Make a copy of the pNewDeviceSettings on the heap
	win32_d3d9_device_settings_t* newDeviceSettingsOnHeap = new win32_d3d9_device_settings_t;
	if (newDeviceSettingsOnHeap == nullptr)
		return E_OUTOFMEMORY;
	::memcpy(newDeviceSettingsOnHeap, pNewDeviceSettings, sizeof(win32_d3d9_device_settings_t));
	pNewDeviceSettings = newDeviceSettingsOnHeap;

	// If the ModifyDeviceSettings callback is non-nullptr, then call it to let the app 
	// change the settings or reject the device change by returning false.
	if (win32_d3d9_state.app)
	{
		::D3DCAPS9 caps;
		win32_d3d9_state.m_d3d->GetDeviceCaps(pNewDeviceSettings->adapter_ordinal, pNewDeviceSettings->device_type, &caps);

		const bool bContinue = win32_d3d9_state.app->win32_d3d9_app_modify_device_settings(caps, *pNewDeviceSettings);
		if (!bContinue)
		{
			// The app rejected the device change by returning false, so just use the current device if there is one.
			if (nullptr == oldDeviceSettings)
				__display_error_message(WIN32_DXUTERR_NOCOMPATIBLEDEVICES);
			SAFE_DELETE(pNewDeviceSettings);
			return E_ABORT;
		}
		if (win32_d3d9_state.m_d3d == nullptr)
		{
			SAFE_DELETE(pNewDeviceSettings);
			return S_FALSE;
		}
	}

	win32_d3d9_state.m_device_settings = pNewDeviceSettings;

	win32_d3d9_pause(__FUNCTION__, "starting", true, true);

	// When a WM_SIZE message is received, it calls DXUTCheckForWindowSizeChange().
	// A WM_SIZE message might be sent when adjusting the window, so tell 
	// DXUTCheckForWindowSizeChange() to ignore size changes temporarily
	win32_d3d9_state.m_ignore_size_change = true;

	//no longer supported
	// update thread safety on/off depending on Direct3D device's thread safety
	//state_t::lock_t::is_thread_safe = 0 != (pNewDeviceSettings->behavior_flags & D3DCREATE_MULTITHREADED);
	assert(0 == (pNewDeviceSettings->behavior_flags & D3DCREATE_MULTITHREADED));
	//no longer supported

	// Take note if the backbuffer width & height are 0 now as they will change after pd3dDevice->reset()
	bool bKeepCurrentWindowSize = false;
	if (pNewDeviceSettings->present_parameters.BackBufferWidth == 0 && pNewDeviceSettings->present_parameters.BackBufferHeight == 0)
		bKeepCurrentWindowSize = true;

	//////////////////////////
	// Before reset
	/////////////////////////
	if (pNewDeviceSettings->present_parameters.Windowed)
	{
		// Going to windowed mode

		if (oldDeviceSettings && !oldDeviceSettings->present_parameters.Windowed)
		{
			// Going from fullscreen -> windowed
			win32_d3d9_state.fullscreen_backbuffer_at_mode_change = __make(oldDeviceSettings->present_parameters);

			// Restore windowed mode style
			::SetWindowLong(win32_d3d9_state.m_hwnd.device_windowed, GWL_STYLE, win32_d3d9_state.m_windowed_style_at_mode_change);
		}

		// If different device windows are used for windowed mode and fullscreen mode,
		// hide the fullscreen window so that it doesn't obscure the screen.
		if (win32_d3d9_state.m_hwnd.device_fullscreen != win32_d3d9_state.m_hwnd.device_windowed)
			::ShowWindow(win32_d3d9_state.m_hwnd.device_fullscreen, SW_HIDE);

		// If using the same window for windowed and fullscreen mode, reattach menu if one exists
		if (win32_d3d9_state.m_hwnd.device_fullscreen == win32_d3d9_state.m_hwnd.device_windowed)
		{
			if (win32_d3d9_state.m_menu != nullptr)
				::SetMenu(win32_d3d9_state.m_hwnd.device_windowed, win32_d3d9_state.m_menu);
		}
	}
	else
	{
		// Going to fullscreen mode

		if (oldDeviceSettings == nullptr || (oldDeviceSettings && oldDeviceSettings->present_parameters.Windowed))
		{
			// Transistioning to full screen mode from a standard window so 
			// save current window position/size/style now in case the user toggles to windowed mode later 
			::WINDOWPLACEMENT* pwp = &win32_d3d9_state.m_windowed_placement;
			::ZeroMemory(pwp, sizeof(::WINDOWPLACEMENT));
			pwp->length = sizeof(::WINDOWPLACEMENT);
			::GetWindowPlacement(win32_d3d9_state.m_hwnd.device_windowed, pwp);
			win32_d3d9_state.m_topmost_while_windowed = ((::GetWindowLong(win32_d3d9_state.m_hwnd.device_windowed, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0);
			DWORD dwStyle = ::GetWindowLong(win32_d3d9_state.m_hwnd.device_windowed, GWL_STYLE);
			dwStyle &= ~WS_MAXIMIZE & ~WS_MINIMIZE; // remove minimize/maximize style
			win32_d3d9_state.m_windowed_style_at_mode_change = dwStyle;
			if (oldDeviceSettings)
				win32_d3d9_state.windowed_backbuffer_at_mode_change = __make(oldDeviceSettings->present_parameters);
		}

		// Hide the window to avoid animation of blank windows
		::ShowWindow(win32_d3d9_state.m_hwnd.device_fullscreen, SW_HIDE);

		// set FS window style
		::SetWindowLong(win32_d3d9_state.m_hwnd.device_fullscreen, GWL_STYLE, WS_POPUP | WS_SYSMENU);

		// If using the same window for windowed and fullscreen mode, save and remove menu 
		if (win32_d3d9_state.m_hwnd.device_fullscreen == win32_d3d9_state.m_hwnd.device_windowed)
		{
			::HMENU hMenu = ::GetMenu(win32_d3d9_state.m_hwnd.device_fullscreen);
			win32_d3d9_state.m_menu = hMenu;
			::SetMenu(win32_d3d9_state.m_hwnd.device_fullscreen, nullptr);
		}

		::WINDOWPLACEMENT wpFullscreen;
		::ZeroMemory(&wpFullscreen, sizeof(::WINDOWPLACEMENT));
		wpFullscreen.length = sizeof(::WINDOWPLACEMENT);
		::GetWindowPlacement(win32_d3d9_state.m_hwnd.device_fullscreen, &wpFullscreen);
		if ((wpFullscreen.flags & WPF_RESTORETOMAXIMIZED) != 0)
		{
			// Restore the window to normal if the window was maximized then minimized.  This causes the 
			// WPF_RESTORETOMAXIMIZED flag to be set which will cause SW_RESTORE to restore the 
			// window from minimized to maxmized which isn't what we want
			wpFullscreen.flags &= ~WPF_RESTORETOMAXIMIZED;
			wpFullscreen.showCmd = SW_RESTORE;
			::SetWindowPlacement(win32_d3d9_state.m_hwnd.device_fullscreen, &wpFullscreen);
		}
	}

	// If AdapterOrdinal and DeviceType are the same, we can just do a reset().
	// If they've changed, we need to do a complete device tear down/rebuild.
	// Also only allow a reset if pd3dDevice is the same as the current device 
	if (!aForceRecreate &&
		(aDeviceFromApp == nullptr || aDeviceFromApp == win32_d3d9_state.m_d3d_device) &&
		win32_d3d9_state.m_d3d_device &&
		oldDeviceSettings &&
		oldDeviceSettings->adapter_ordinal == pNewDeviceSettings->adapter_ordinal &&
		oldDeviceSettings->device_type == pNewDeviceSettings->device_type &&
		oldDeviceSettings->behavior_flags == pNewDeviceSettings->behavior_flags)
	{
		// reset the Direct3D device and call the app's device callbacks
		hr = __reset_3d_environment();
		if (FAILED(hr))
		{
			//FS_ERROR("__reset3DEnvironment() failed");

			if (D3DERR_DEVICELOST == hr)
			{
				// The device is lost, just mark it as so and continue on with 
				// capturing the win32_d3d9_state and resizing the window/etc.
				win32_d3d9_state.m_device_lost = true;
			}
			else if (WIN32_DXUTERR_RESETTINGDEVICEOBJECTS == hr || WIN32_DXUTERR_MEDIANOTFOUND == hr)
			{
				// Something bad happened in the app callbacks
				SAFE_DELETE(oldDeviceSettings);
				__display_error_message(hr);
				win32_d3d9_shutdown();
				return hr;
			}
			else // DXUTERR_RESETTINGDEVICE
			{
				// reset failed and the device wasn't lost and it wasn't the apps fault, 
				// so recreate the device to try to recover
				win32_d3d9_state.m_device_settings = oldDeviceSettings;
				if (FAILED(__change_device(pNewDeviceSettings, aDeviceFromApp, true, aClipWindowToSingleAdapter)))
				{
					// If that fails, then shutdown
					SAFE_DELETE(oldDeviceSettings);
					win32_d3d9_shutdown();
					return WIN32_DXUTERR_CREATINGDEVICE;
				}
				else
				{
					SAFE_DELETE(oldDeviceSettings);
					return S_OK;
				}
			}
		}
	}
	else
	{
		// cleanup if not first device created
		if (oldDeviceSettings)
			__cleanup_3d_environment(false);

		// create the D3D device and call the app's device callbacks
		hr = __create_3d_environment(aDeviceFromApp);
		if (FAILED(hr))
		{
			SAFE_DELETE(oldDeviceSettings);
			__cleanup_3d_environment(true);
			__display_error_message(hr);
			win32_d3d9_pause(__FUNCTION__, "__create3DEnvironment() FAILED", false, false);
			win32_d3d9_state.m_ignore_size_change = false;
			return hr;
		}
	}

	// Enable/disable StickKeys shortcut, ToggleKeys shortcut, FilterKeys shortcut, and Windows key 
	// to prevent accidental task switching
	__allow_shortcut_keys(pNewDeviceSettings->present_parameters.Windowed ? true : false);

	win32_d3d9_state.m_adapter_monitor = win32_d3d9_state.m_d3d->GetAdapterMonitor(pNewDeviceSettings->adapter_ordinal);

	// update the device stats text
	//updateStaticFrameStats();

	if (oldDeviceSettings && !oldDeviceSettings->present_parameters.Windowed && pNewDeviceSettings->present_parameters.Windowed)
	{
		// Going from fullscreen -> windowed

		// Restore the show win32_d3d9_state, and positions/size of the window to what it was
		// It is important to adjust the window size 
		// after resetting the device rather than beforehand to ensure 
		// that the monitor resolution is correct and does not limit the size of the new window.
		::WINDOWPLACEMENT* pwp = &win32_d3d9_state.m_windowed_placement;
		::SetWindowPlacement(win32_d3d9_state.m_hwnd.device_windowed, pwp);

		// Also restore the z-order of window to previous win32_d3d9_state
		::HWND hWndInsertAfter = win32_d3d9_state.m_topmost_while_windowed ? HWND_TOPMOST : HWND_NOTOPMOST;
		::SetWindowPos(win32_d3d9_state.m_hwnd.device_windowed, hWndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSIZE);
	}

	// Check to see if the window needs to be resized.  
	// Handle cases where the window is minimized and maxmimized as well.
	bool bNeedToResize = false;
	if (pNewDeviceSettings->present_parameters.Windowed && // only resize if in windowed mode
		!bKeepCurrentWindowSize)          // only resize if myPresentParameters.BackbufferWidth/Height were not 0
	{
		::UINT nClientWidth, nClientHeight;
		if (::IsIconic(win32_d3d9_state.m_hwnd.device_windowed))
		{
			// Window is currently minimized. To tell if it needs to resize, 
			// get the client rect of window when its restored the 
			// hard way using GetWindowPlacement()
			::WINDOWPLACEMENT wp;
			::ZeroMemory(&wp, sizeof(::WINDOWPLACEMENT));
			wp.length = sizeof(::WINDOWPLACEMENT);
			::GetWindowPlacement(win32_d3d9_state.m_hwnd.device_windowed, &wp);

			if ((wp.flags & WPF_RESTORETOMAXIMIZED) != 0 && wp.showCmd == SW_SHOWMINIMIZED)
			{
				// WPF_RESTORETOMAXIMIZED means that when the window is restored it will
				// be maximized.  So maximize the window temporarily to get the client rect 
				// when the window is maximized.  GetSystemMetrics( SM_CXMAXIMIZED ) will give this 
				// information if the window is on the primary but this will work on multimon.
				::ShowWindow(win32_d3d9_state.m_hwnd.device_windowed, SW_RESTORE);
				::RECT rcClient;
				::GetClientRect(win32_d3d9_state.m_hwnd.device_windowed, &rcClient);
				nClientWidth = (UINT)(rcClient.right - rcClient.left);
				nClientHeight = (UINT)(rcClient.bottom - rcClient.top);
				::ShowWindow(win32_d3d9_state.m_hwnd.device_windowed, SW_MINIMIZE);
			}
			else
			{
				// Use wp.rcNormalPosition to get the client rect, but wp.rcNormalPosition 
				// includes the window frame so subtract it
				RECT rcFrame = { 0 };
				::AdjustWindowRect(&rcFrame, win32_d3d9_state.m_windowed_style_at_mode_change, win32_d3d9_state.m_menu != nullptr);
				LONG nFrameWidth = rcFrame.right - rcFrame.left;
				LONG nFrameHeight = rcFrame.bottom - rcFrame.top;
				nClientWidth = (UINT)(wp.rcNormalPosition.right - wp.rcNormalPosition.left - nFrameWidth);
				nClientHeight = (UINT)(wp.rcNormalPosition.bottom - wp.rcNormalPosition.top - nFrameHeight);
			}
		}
		else
		{
			// Window is restored or maximized so just get its client rect
			RECT rcClient;
			::GetClientRect(win32_d3d9_state.m_hwnd.device_windowed, &rcClient);
			nClientWidth = (UINT)(rcClient.right - rcClient.left);
			nClientHeight = (UINT)(rcClient.bottom - rcClient.top);
		}

		// Now that we know the client rect, compare it against the back buffer size
		// to see if the client rect is already the right size
		if (nClientWidth != pNewDeviceSettings->present_parameters.BackBufferWidth ||
			nClientHeight != pNewDeviceSettings->present_parameters.BackBufferHeight)
		{
			bNeedToResize = true;
		}

		if (aClipWindowToSingleAdapter && !::IsIconic(win32_d3d9_state.m_hwnd.device_windowed))
		{
			// get the rect of the monitor attached to the adapter
			MONITORINFO miAdapter;
			miAdapter.cbSize = sizeof(MONITORINFO);
			::HMONITOR hAdapterMonitor = win32_d3d9_state.m_d3d->GetAdapterMonitor(pNewDeviceSettings->adapter_ordinal);
			win32_d3d9_get_monitor_info(hAdapterMonitor, &miAdapter);
			::HMONITOR hWindowMonitor = win32_d3d9_monitor_from_window(win32_d3d9_hwnd(), MONITOR_DEFAULTTOPRIMARY);

			// get the rect of the window
			::RECT rcWindow;
			::GetWindowRect(win32_d3d9_state.m_hwnd.device_windowed, &rcWindow);

			// Check if the window rect is fully inside the adapter's vitural screen rect
			if ((rcWindow.left < miAdapter.rcWork.left ||
				rcWindow.right > miAdapter.rcWork.right ||
				rcWindow.top < miAdapter.rcWork.top ||
				rcWindow.bottom > miAdapter.rcWork.bottom))
			{
				if (hWindowMonitor == hAdapterMonitor && ::IsZoomed(win32_d3d9_state.m_hwnd.device_windowed))
				{
					// If the window is maximized and on the same monitor as the adapter, then 
					// no need to clip to single adapter as the window is already clipped 
					// even though the rcWindow rect is outside of the miAdapter.rcWork
				}
				else
				{
					bNeedToResize = true;
				}
			}
		}
	}

	// Only resize window if needed 
	if (bNeedToResize)
	{
		// Need to resize, so if window is maximized or minimized then restore the window
		if (::IsIconic(win32_d3d9_state.m_hwnd.device_windowed))
			::ShowWindow(win32_d3d9_state.m_hwnd.device_windowed, SW_RESTORE);
		if (::IsZoomed(win32_d3d9_state.m_hwnd.device_windowed)) // doing the IsIconic() check first also handles the WPF_RESTORETOMAXIMIZED case
			::ShowWindow(win32_d3d9_state.m_hwnd.device_windowed, SW_RESTORE);

		if (aClipWindowToSingleAdapter)
		{
			// get the rect of the monitor attached to the adapter
			MONITORINFO miAdapter;
			miAdapter.cbSize = sizeof(MONITORINFO);
			win32_d3d9_get_monitor_info(win32_d3d9_state.m_d3d->GetAdapterMonitor(pNewDeviceSettings->adapter_ordinal), &miAdapter);

			// get the rect of the monitor attached to the window
			MONITORINFO miWindow;
			miWindow.cbSize = sizeof(MONITORINFO);
			win32_d3d9_get_monitor_info(win32_d3d9_monitor_from_window(win32_d3d9_hwnd(), MONITOR_DEFAULTTOPRIMARY), &miWindow);

			// Do something reasonable if the BackBuffer size is greater than the monitor size
			int32_t nAdapterMonitorWidth = miAdapter.rcWork.right - miAdapter.rcWork.left;
			int32_t nAdapterMonitorHeight = miAdapter.rcWork.bottom - miAdapter.rcWork.top;

			int32_t nClientWidth = pNewDeviceSettings->present_parameters.BackBufferWidth;
			int32_t nClientHeight = pNewDeviceSettings->present_parameters.BackBufferHeight;

			// get the rect of the window
			::RECT rcWindow;
			::GetWindowRect(win32_d3d9_state.m_hwnd.device_windowed, &rcWindow);

			// Make a window rect with a client rect that is the same size as the backbuffer
			::RECT rcResizedWindow;
			rcResizedWindow.left = 0;
			rcResizedWindow.right = nClientWidth;
			rcResizedWindow.top = 0;
			rcResizedWindow.bottom = nClientHeight;
			::AdjustWindowRect(&rcResizedWindow, ::GetWindowLong(win32_d3d9_state.m_hwnd.device_windowed, GWL_STYLE), win32_d3d9_state.m_menu != nullptr);

			int32_t nWindowWidth = rcResizedWindow.right - rcResizedWindow.left;
			int32_t nWindowHeight = rcResizedWindow.bottom - rcResizedWindow.top;

			if (nWindowWidth > nAdapterMonitorWidth)
				nWindowWidth = (nAdapterMonitorWidth - 0);
			if (nWindowHeight > nAdapterMonitorHeight)
				nWindowHeight = (nAdapterMonitorHeight - 0);

			if (rcResizedWindow.left < miAdapter.rcWork.left ||
				rcResizedWindow.top < miAdapter.rcWork.top ||
				rcResizedWindow.right > miAdapter.rcWork.right ||
				rcResizedWindow.bottom > miAdapter.rcWork.bottom)
			{
				int32_t nWindowOffsetX = (nAdapterMonitorWidth - nWindowWidth) / 2;
				int32_t nWindowOffsetY = (nAdapterMonitorHeight - nWindowHeight) / 2;

				rcResizedWindow.left = miAdapter.rcWork.left + nWindowOffsetX;
				rcResizedWindow.top = miAdapter.rcWork.top + nWindowOffsetY;
				rcResizedWindow.right = miAdapter.rcWork.left + nWindowOffsetX + nWindowWidth;
				rcResizedWindow.bottom = miAdapter.rcWork.top + nWindowOffsetY + nWindowHeight;
			}

			// Resize the window.  It is important to adjust the window size 
			// after resetting the device rather than beforehand to ensure 
			// that the monitor resolution is correct and does not limit the size of the new window.
			::SetWindowPos(win32_d3d9_state.m_hwnd.device_windowed, 0, rcResizedWindow.left, rcResizedWindow.top, nWindowWidth, nWindowHeight, SWP_NOZORDER);
		}
		else
		{
			// Make a window rect with a client rect that is the same size as the backbuffer
			::RECT rcWindow = { 0 };
			rcWindow.right = (long)(pNewDeviceSettings->present_parameters.BackBufferWidth);
			rcWindow.bottom = (long)(pNewDeviceSettings->present_parameters.BackBufferHeight);
			::AdjustWindowRect(&rcWindow, ::GetWindowLong(win32_d3d9_state.m_hwnd.device_windowed, GWL_STYLE), win32_d3d9_state.m_menu != nullptr);

			// Resize the window.  It is important to adjust the window size 
			// after resetting the device rather than beforehand to ensure 
			// that the monitor resolution is correct and does not limit the size of the new window.
			int32_t cx = (int32_t)(rcWindow.right - rcWindow.left);
			int32_t cy = (int32_t)(rcWindow.bottom - rcWindow.top);
			::SetWindowPos(win32_d3d9_state.m_hwnd.device_windowed, 0, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOMOVE);
		}

		// Its possible that the new window size is not what we asked for.  
		// No window can be sized larger than the desktop, so see see if the Windows OS resized the 
		// window to something smaller to fit on the desktop.  Also if WM_GETMINMAXINFO
		// will put a limit on the smallest/largest window size.
		::RECT rcClient;
		::GetClientRect(win32_d3d9_state.m_hwnd.device_windowed, &rcClient);
		::UINT nClientWidth = (::UINT)(rcClient.right - rcClient.left);
		::UINT nClientHeight = (::UINT)(rcClient.bottom - rcClient.top);
		if (nClientWidth != pNewDeviceSettings->present_parameters.BackBufferWidth || nClientHeight != pNewDeviceSettings->present_parameters.BackBufferHeight)
		{
			// If its different, then resize the backbuffer again.  This time create a backbuffer that matches the 
			// client rect of the current window w/o resizing the window.
			win32_d3d9_device_settings_t deviceSettings(__get_device_settings());
			deviceSettings.present_parameters.BackBufferWidth = 0;
			deviceSettings.present_parameters.BackBufferHeight = 0;
			hr = __change_device(&deviceSettings, nullptr, false, aClipWindowToSingleAdapter);
			if (FAILED(hr))
			{
				SAFE_DELETE(oldDeviceSettings);
				__cleanup_3d_environment(true);
				win32_d3d9_pause(__FUNCTION__, "__changeDevice() FAILED", false, false);
				win32_d3d9_state.m_ignore_size_change = false;
				return hr;
			}
		}
	}

	// Make the window visible
	if (!::IsWindowVisible(win32_d3d9_hwnd()))
		::ShowWindow(win32_d3d9_hwnd(), SW_SHOW);

	// Make the window visible
	if (!::IsWindowVisible(win32_d3d9_hwnd()))
		::ShowWindow(win32_d3d9_hwnd(), SW_SHOW);

	// Ensure that the display doesn't power down when fullscreen but does when windowed
	if (!win32_d3d9_is_windowed())
		::SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS);
	else
		::SetThreadExecutionState(ES_CONTINUOUS);

	SAFE_DELETE(oldDeviceSettings);
	win32_d3d9_state.m_ignore_size_change = false;
	win32_d3d9_pause(__FUNCTION__, "finished", false, false);
	win32_d3d9_state.m_device.created = true;

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Checks if the window client rect has changed and if it has, then reset the device
//--------------------------------------------------------------------------------------
static void __check_for_window_size_change()
{
	// Skip the check for various reasons
	if (win32_d3d9_state.m_ignore_size_change ||
		!win32_d3d9_state.m_device.created ||
		!win32_d3d9_state.m_device_settings->present_parameters.Windowed)
		return;

	::RECT rcCurrentClient;
	::GetClientRect(win32_d3d9_hwnd(), &rcCurrentClient);

	if ((UINT)rcCurrentClient.right != win32_d3d9_state.m_device_settings->present_parameters.BackBufferWidth ||
		(UINT)rcCurrentClient.bottom != win32_d3d9_state.m_device_settings->present_parameters.BackBufferHeight)
	{
		// A new window size will require a new backbuffer size
		// size, so the device must be reset and the D3D structures updated accordingly.

		// Tell ChangeDevice and D3D to size according to the HWND's client rect
		win32_d3d9_device_settings_t deviceSettings(__get_device_settings());
		deviceSettings.present_parameters.BackBufferWidth = 0;
		deviceSettings.present_parameters.BackBufferHeight = 0;
		__change_device(&deviceSettings, nullptr, false, false);
	}
}

//--------------------------------------------------------------------------------------
// Checks to see if the HWND changed monitors, and if it did it creates a device 
// from the monitor's adapter and recreates the scene.
//--------------------------------------------------------------------------------------
static void __check_for_window_changing_monitors()
{
	// Skip this check for various reasons
	if (!win32_d3d9_state.m_auto_change_adapter || win32_d3d9_state.m_ignore_size_change || !win32_d3d9_state.m_device.created || !win32_d3d9_state.m_device_settings->present_parameters.Windowed)
		return;

	::HRESULT hr;
	HMONITOR hWindowMonitor = win32_d3d9_monitor_from_window(win32_d3d9_hwnd(), MONITOR_DEFAULTTOPRIMARY);
	HMONITOR hAdapterMonitor = win32_d3d9_state.m_adapter_monitor;
	if (hWindowMonitor != hAdapterMonitor)
	{
		UINT newOrdinal;
		if (SUCCEEDED(__get_adapter_ordinal_from_monitor(hWindowMonitor, &newOrdinal)))
		{
			// find the closest valid device settings with the new ordinal
			win32_d3d9_device_settings_t deviceSettings(__get_device_settings());
			deviceSettings.adapter_ordinal = newOrdinal;

			match_options_t matchOptions;
			matchOptions.adapter_ordinal = match_type_t::PRESERVE_INPUT;
			matchOptions.device_type = match_type_t::CLOSEST_TO_INPUT;
			matchOptions.windowed = match_type_t::CLOSEST_TO_INPUT;
			matchOptions.adapter_format = match_type_t::CLOSEST_TO_INPUT;
			matchOptions.vertex_processing = match_type_t::CLOSEST_TO_INPUT;
			matchOptions.resolution = match_type_t::CLOSEST_TO_INPUT;
			matchOptions.backbuffer_format = match_type_t::CLOSEST_TO_INPUT;
			matchOptions.backbuffer_count = match_type_t::CLOSEST_TO_INPUT;
			matchOptions.multi_sample = match_type_t::CLOSEST_TO_INPUT;
			matchOptions.swap_effect = match_type_t::CLOSEST_TO_INPUT;
			matchOptions.depth_format = match_type_t::CLOSEST_TO_INPUT;
			matchOptions.stencil_format = match_type_t::CLOSEST_TO_INPUT;
			matchOptions.present_flags = match_type_t::CLOSEST_TO_INPUT;
			matchOptions.refresh_rate = match_type_t::CLOSEST_TO_INPUT;
			matchOptions.present_interval = match_type_t::CLOSEST_TO_INPUT;

			hr = __find_valid_device_settings(&deviceSettings, &deviceSettings, &matchOptions);
			if (SUCCEEDED(hr))
			{
				// create a Direct3D device using the new device settings.  
				// If there is an existing device, then it will either reset or recreate the scene.
				hr = __change_device(&deviceSettings, nullptr, false, false);

				// If hr == E_ABORT, this means the app rejected the device settings in the ModifySettingsCallback
				if (hr == E_ABORT)
				{
					// so nothing changed and keep from attempting to switch adapters next time
					win32_d3d9_state.m_auto_change_adapter = false;
				}
				else if (FAILED(hr))
				{
					win32_d3d9_shutdown();
					win32_d3d9_pause(__FUNCTION__, "__changeDevice() FAILED", false, false);
					return;
				}
			}
		}
	}
}
//--------------------------------------------------------------------------------------
// Handles window messages 
//--------------------------------------------------------------------------------------
static ::LRESULT CALLBACK __static_wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Pass all messages to the app's MsgProc callback, and don't 
	// process further messages if the apps says not to.
	if (win32_d3d9_state.app)
	{
		bool bNoFurtherProcessing = false;
		LRESULT nResult = win32_d3d9_state.app->win32_d3d9_app_msg_proc(hWnd, uMsg, wParam, lParam, &bNoFurtherProcessing);
		if (bNoFurtherProcessing)
			return nResult;
	}

	switch (uMsg)
	{
	case WM_PAINT:
	{
		// Handle paint messages when the app is paused
		if (win32_d3d9_state.m_d3d_device && win32_d3d9_is_rendering_paused() && win32_d3d9_state.m_device_objects.created && win32_d3d9_state.m_device_objects.reset)
		{
			::HRESULT hr;

			if (win32_d3d9_state.app)
				win32_d3d9_state.app->win32_d3d9_app_frame_render(win32_d3d9_state.m_time, win32_d3d9_state.m_elapsed_time);

			hr = win32_d3d9_state.m_d3d_device->Present(nullptr, nullptr, nullptr, nullptr);
			if (D3DERR_DEVICELOST == hr)
			{
				win32_d3d9_state.m_device_lost = true;
			}
			else if (D3DERR_DRIVERINTERNALERROR == hr)
			{
				// When D3DERR_DRIVERINTERNALERROR is returned from Present(),
				// the application can do one of the following:
				// 
				// - End, with the pop-up window saying that the application cannot continue 
				//   because of problems in the display adapter and that the user should 
				//   contact the adapter manufacturer.
				//
				// - Attempt to restart by calling IDirect3DDevice9::reset, which is essentially the same 
				//   path as recovering from a lost device. If IDirect3DDevice9::reset fails with 
				//   D3DERR_DRIVERINTERNALERROR, the application should end immediately with the message 
				//   that the user should contact the adapter manufacturer.
				// 
				// The framework attempts the path of resetting the device
				// 
				win32_d3d9_state.m_device_lost = true;
			}
		}
		break;
	}

	case WM_SIZE:
		if (SIZE_MINIMIZED == wParam)
		{
			if (win32_d3d9_state.app)
			{
				win32_d3d9_pause(__FUNCTION__, "WM_SIZE == SIZE_MINIMIZED, using app callback", win32_d3d9_state.app->win32_d3d9_app_pause_time_when_minimized(), true);	//optional to pause time, always pause rendering...
			}
			else
			{
				win32_d3d9_pause(__FUNCTION__, "WM_SIZE == SIZE_MINIMIZED, no app", true, true); // pause while we're minimized
			}

			win32_d3d9_state.m_minimized = true;
			win32_d3d9_state.m_maximized = false;
		}
		else
		{
			RECT rcCurrentClient;
			::GetClientRect(win32_d3d9_hwnd(), &rcCurrentClient);
			if (rcCurrentClient.top == 0 && rcCurrentClient.bottom == 0)
			{
				// Rapidly clicking the task bar to minimize and restore a window
				// can cause a WM_SIZE message with SIZE_RESTORED when 
				// the window has actually become minimized due to rapid change
				// so just ignore this message
			}
			else if (SIZE_MAXIMIZED == wParam)
			{
				if (win32_d3d9_state.m_minimized)
					win32_d3d9_pause(__FUNCTION__, "WM_SIZE == SIZE_MAXIMIZED", false, false); // Unpause since we're no longer minimized
				win32_d3d9_state.m_minimized = false;
				win32_d3d9_state.m_maximized = true;
				__check_for_window_size_change();
				__check_for_window_changing_monitors();
			}
			else if (SIZE_RESTORED == wParam)
			{
				if (win32_d3d9_state.m_maximized)
				{
					win32_d3d9_state.m_maximized = false;
					__check_for_window_size_change();
					__check_for_window_changing_monitors();
				}
				else if (win32_d3d9_state.m_minimized)
				{
					win32_d3d9_pause(__FUNCTION__, "WM_SIZE == SIZE_RESTORED", false, false); // Unpause since we're no longer minimized
					win32_d3d9_state.m_minimized = false;
					__check_for_window_size_change();
					__check_for_window_changing_monitors();
				}
				else if (win32_d3d9_state.m_inside.size_move)
				{
					// If we're neither maximized nor minimized, the window size 
					// is changing by the user dragging the window edges.  In this 
					// case, we don't reset the device yet -- we wait until the 
					// user stops dragging, and a WM_EXITSIZEMOVE message comes.
				}
				else
				{
					// This WM_SIZE come from resizing the window via an API like SetWindowPos() so 
					// resize and reset the device now.
					__check_for_window_size_change();
					__check_for_window_changing_monitors();
				}
			}
		}
		break;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = DXUT_MIN_WINDOW_SIZE_X;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = DXUT_MIN_WINDOW_SIZE_Y;
		break;

	case WM_ENTERSIZEMOVE:
		// Halt frame movement while the app is sizing or moving
		win32_d3d9_pause(__FUNCTION__, "WM_ENTERSIZEMOVE", true, true);
		win32_d3d9_state.m_inside.size_move = true;
		break;

	case WM_EXITSIZEMOVE:
		win32_d3d9_pause(__FUNCTION__, "WM_EXITSIZEMOVE", false, false);
		__check_for_window_size_change();
		__check_for_window_changing_monitors();
		win32_d3d9_state.m_inside.size_move = false;
		break;

	case WM_MOUSEMOVE:
		if (win32_d3d9_state.m_active && !win32_d3d9_is_windowed())
		{
			if (win32_d3d9_state.m_d3d_device)
			{
				::POINT ptCursor;
				::GetCursorPos(&ptCursor);
				win32_d3d9_state.m_d3d_device->SetCursorPosition(ptCursor.x, ptCursor.y, 0);
			}
		}
		break;

	case WM_SETCURSOR:
		if (win32_d3d9_state.m_active && !win32_d3d9_is_windowed())
		{
			if (win32_d3d9_state.m_d3d_device && win32_d3d9_state.m_show_cursor_when_fullscreen)
				win32_d3d9_state.m_d3d_device->ShowCursor(true);
			return true; // prevent Windows from setting cursor to window struct cursor
		}
		break;

	case WM_ACTIVATEAPP:
		if (wParam == TRUE && !win32_d3d9_state.m_active) // Handle only if previously not active 
		{
			win32_d3d9_state.m_active = true;

			// The GetMinimizedWhileFullscreen() varible is used instead of !IsWindowed()
			// to handle the rare case toggling to windowed mode while the fullscreen application 
			// is minimized and thus making the pause count wrong
			if (win32_d3d9_state.m_minimized_while_fullscreen)
			{
				win32_d3d9_pause(__FUNCTION__, "WM_ACTIVATEAPP == TRUE", false, false); // Unpause since we're no longer minimized
				win32_d3d9_state.m_minimized_while_fullscreen = false;
			}

			// Upon returning to this app, potentially disable shortcut keys 
			// (Windows key, accessibility shortcuts) 
			__allow_shortcut_keys(win32_d3d9_is_windowed() ? true : false);

		}
		else if (wParam == FALSE && win32_d3d9_state.m_active) // Handle only if previously active 
		{
			win32_d3d9_state.m_active = false;

			// Disable any controller rumble when de-activating app
			//stopRumbleOnAllControllers();

			if (!win32_d3d9_is_windowed())
			{
				// Going from full screen to a minimized win32_d3d9_state 
				::ClipCursor(nullptr);      // don't limit the cursor anymore

											// pause while we're minimized (take care not to pause twice by handling this message twice)
				if (win32_d3d9_state.app)
					win32_d3d9_pause(__FUNCTION__, "WM_ACTIVATEAPP == FALSE, not windowed, using app callback", win32_d3d9_state.app->win32_d3d9_app_pause_time_when_minimized(), true);
				else
					win32_d3d9_pause(__FUNCTION__, "WM_ACTIVATEAPP == FALSE, not windowed, no app", true, true);

				win32_d3d9_state.m_minimized_while_fullscreen = true;
			}

			// Restore shortcut keys (Windows key, accessibility shortcuts) to original win32_d3d9_state
			//
			// This is important to call here if the shortcuts are disabled, 
			// because if this is not done then the Windows key will continue to 
			// be disabled while this app is running which is very bad.
			// If the app crashes, the Windows key will return to normal.
			__allow_shortcut_keys(true);
		}
		break;

	case WM_ENTERMENULOOP:
		// pause the app when menus are displayed
		win32_d3d9_pause(__FUNCTION__, "WM_ENTERMENULOOP", true, true);
		break;

	case WM_EXITMENULOOP:
		win32_d3d9_pause(__FUNCTION__, "WM_EXITMENULOOP", false, false);
		break;

	case WM_MENUCHAR:
		// A menu is active and the user presses a key that does not correspond to any mnemonic or accelerator key
		// So just ignore and don't beep
		return MAKELRESULT(0, MNC_CLOSE);
		break;

	case WM_NCHITTEST:
		// Prevent the user from selecting the menu in full screen mode
		if (!win32_d3d9_is_windowed())
			return HTCLIENT;
		break;

	case WM_POWERBROADCAST:
		switch (wParam)
		{
#ifndef PBT_APMQUERYSUSPEND
#define PBT_APMQUERYSUSPEND 0x0000
#endif
		case PBT_APMQUERYSUSPEND:
			// At this point, the app should save any data for open
			// network connections, files, etc., and prepare to go into
			// a suspended mode.  The app can use the MsgProc callback
			// to handle this if desired.
			return true;

#ifndef PBT_APMRESUMESUSPEND
#define PBT_APMRESUMESUSPEND 0x0007
#endif
		case PBT_APMRESUMESUSPEND:
			// At this point, the app should recover any data, network
			// connections, files, etc., and resume running from when
			// the app was suspended. The app can use the MsgProc callback
			// to handle this if desired.

			// QPC may lose consistency when suspending, so reset the timer
			// upon resume.
			__timer().reset();
			//win32_d3d9_state.m_last_stats_update_time = 0;
			return true;
		}
		break;

	case WM_SYSCOMMAND:
		// Prevent moving/sizing in full screen mode
		switch (wParam)
		{
		case SC_MOVE:
		case SC_SIZE:
		case SC_MAXIMIZE:
		case SC_KEYMENU:
			if (!win32_d3d9_is_windowed())
				return 0;
			break;
		}
		break;

	case WM_SYSKEYDOWN:
	{
		switch (wParam)
		{
		case VK_RETURN:
		{
			// Toggle full screen upon alt-enter 
			const ::DWORD dwMask = (1 << 29);
			if ((lParam & dwMask) != 0) // Alt is down also
			{
				// Toggle the full screen/window mode
				win32_d3d9_pause(__FUNCTION__, "WM_SYSKEYDOWN, win32_d3d9_state.getHandleAltEnter() == true, PRE-toggle_fullscreen()", true, true);
				win32_d3d9_toggle_fullscreen();
				win32_d3d9_pause(__FUNCTION__, "WM_SYSKEYDOWN, win32_d3d9_state.getHandleAltEnter() == true, POST-toggle_fullscreen()", false, false);
				return 0;
			}
		}
		}
		break;
	}

	case WM_KEYDOWN:
	{
		//win32_d3d9_state.m_handle_default_hotkeys;
		break;
	}

	case WM_CLOSE:
	{
		HMENU hMenu;
		hMenu = ::GetMenu(hWnd);
		if (hMenu != nullptr)
			::DestroyMenu(hMenu);
		::DestroyWindow(hWnd);
		::UnregisterClass("Direct3DWindowClass", nullptr);
		win32_d3d9_state.m_hwnd = {};
		return 0;
	}

	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	}

	// Don't allow the F10 key to act as a shortcut to the menu bar
	// by not passing these messages to the DefWindowProc only when
	// there's no menu present
	if (!true || win32_d3d9_state.m_menu == nullptr && (uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP) && wParam == VK_F10)
		return 0;

	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//--------------------------------------------------------------------------------------
// Optionally parses the command line and sets if default hotkeys are handled
//
//       Possible command line parameters are:
//          -adapter:#              forces app to use this adapter # (fails if the adapter doesn't exist)
//          -windowed               forces app to start windowed
//          -fullscreen             forces app to start full screen
//          -forcehal               forces app to use HAL (fails if HAL doesn't exist)
//          -forceref               forces app to use REF (fails if REF doesn't exist)
//          -forcepurehwvp          forces app to use pure HWVP (fails if device doesn't support it)
//          -forcehwvp              forces app to use HWVP (fails if device doesn't support it)
//          -forceswvp              forces app to use SWVP 
//          -forcevsync:#           if # is 0, forces app to use D3DPRESENT_INTERVAL_IMMEDIATE otherwise force use of D3DPRESENT_INTERVAL_DEFAULT 
//          -width:#                forces app to use # for width. for full screen, it will pick the closest possible supported mode
//          -height:#               forces app to use # for height. for full screen, it will pick the closest possible supported mode
//          -startx:#               forces app to use # for the x coord of the window position for windowed mode
//          -starty:#               forces app to use # for the y coord of the window position for windowed mode
//          -constantframetime:#    forces app to use constant frame time, where # is the time/frame in seconds
//          -quitafterframe:x       forces app to quit after # frames
//          -noerrormsgboxes        prevents the display of message boxes generated by the framework so the application can be run without user interaction
//          -nostats                prevents the display of the stats
//          -relaunchmce            re-launches the MCE UI after the app exits
//          -automation             every CDXUTDialog created will have EnableKeyboardInput(true) called, enabling UI navigation with keyboard
//                                  This is useful when automating application testing.
//
//      Hotkeys handled by default are:
//          Alt-Enter           toggle between full screen & windowed (hotkey always enabled)
//          ESC                 exit app 
//          F3                  toggle HAL/REF
//          F8                  toggle wire-frame mode
//          pause               pause time
//--------------------------------------------------------------------------------------
typedef IDirect3D9* (WINAPI* LPDIRECT3DCREATE9) (UINT);
::HRESULT win32_d3d9_init()
{
	win32_d3d9_state.m_init.called = true;

	// Not always needed, but lets the app create GDI dialogs
	::InitCommonControls();

	// Save the current sticky/toggle/filter key settings so DXUT can restore them later
	win32_d3d9_state.m_startup_keys.sticky.cbSize = sizeof(::STICKYKEYS);
	::SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(::STICKYKEYS), &win32_d3d9_state.m_startup_keys.sticky, 0);

	win32_d3d9_state.m_startup_keys.toggle.cbSize = sizeof(::TOGGLEKEYS);
	::SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(::TOGGLEKEYS), &win32_d3d9_state.m_startup_keys.toggle, 0);

	win32_d3d9_state.m_startup_keys.filter.cbSize = sizeof(::FILTERKEYS);
	SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(::FILTERKEYS), &win32_d3d9_state.m_startup_keys.filter, 0);

	// Increase the accuracy of Sleep() without needing to link to winmm.lib
	{
		char path[MAX_PATH + 1];
		if (::GetSystemDirectoryA(path, sizeof(path)))
		{
			::strcat_s(path, MAX_PATH, "\\winmm.dll");
			::HINSTANCE hInstWinMM = LoadLibraryA(path);
			if (hInstWinMM)
			{
				LPTIMEBEGINPERIOD pTimeBeginPeriod = (LPTIMEBEGINPERIOD)::GetProcAddress(hInstWinMM, "timeBeginPeriod");
				if (nullptr != pTimeBeginPeriod)
					pTimeBeginPeriod(1);

				::FreeLibrary(hInstWinMM);
			}
		}
	}

	//win32_d3d9_state.m_show_msg_box_on_error = true;
	//win32_d3d9_state.m_handle_default_hotkeys = true;
	//win32_d3d9_state.m_handle_alt_enter = true;

	// Verify D3DX version
	if (!::D3DXCheckVersion(D3D_SDK_VERSION, D3DX_SDK_VERSION))
	{
		//FS_ERROR("D3DXCheckVersion() failed");
		__display_error_message(WIN32_DXUTERR_INCORRECTVERSION);
		return WIN32_DXUT_ERR("D3DXCheckVersion", WIN32_DXUTERR_INCORRECTVERSION);
	}

	// create a Direct3D object if one has not already been created
	if (nullptr == win32_d3d9_state.m_d3d)
	{
		// This may fail if DirectX 9 isn't installed
		// This may fail if the DirectX headers are out of sync with the installed DirectX DLLs
		char systemPath[MAX_PATH + 1];
		if (::GetSystemDirectory(systemPath, sizeof(systemPath)))
		{
			::StringCchCat(systemPath, MAX_PATH, "\\d3d9.dll");
			::HMODULE library = ::LoadLibrary(systemPath);
			if (library)
			{
				LPDIRECT3DCREATE9 function;
				function = (LPDIRECT3DCREATE9)::GetProcAddress(library, "Direct3DCreate9");
				if (function)
					win32_d3d9_state.m_d3d = function(D3D_SDK_VERSION);
				//else
				//	FS_ERROR("::GetProcAddress() failed: Direct3DCreate9");

				//NOTE: we need to keep the library around, since we are caching and using the object returned!
				//::FreeLibrary(library);
			}
			else
			{
				//FS_ERROR("::LoadLibrary() failed: %s", systemPath);
			}
		}
		else
		{
			//FS_ERROR("failed to get system directory");
		}
	}

	if (nullptr == win32_d3d9_state.m_d3d)
	{
		// If still nullptr, then something went wrong
		//FS_ERROR("failed to acquire IDirect3D9 interface");
		__display_error_message(WIN32_DXUTERR_NODIRECT3D);
		return WIN32_DXUT_ERR("Direct3DCreate9", WIN32_DXUTERR_NODIRECT3D);
	}

	// reset the timer
	__timer().reset();

	win32_d3d9_state.m_init.created = true;

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Creates a window with the specified window title, icon, menu, and 
// starting position.  If DXUTInit() has not already been called, it will
// call it with the default parameters.  Instead of calling this, you can 
// call DXUTSetWindow() to use an existing window.  
//--------------------------------------------------------------------------------------
::HRESULT win32_d3d9_create_window(const char* aWindowTitle, const ::DWORD aWindowStyle)
{
	::HRESULT hr;

	// Not allowed to call this from inside the device callbacks
	if (win32_d3d9_state.m_inside.device_callback)
		return WIN32_DXUT_ERR_MSGBOX("DXUTCreateWindow", E_FAIL);

	win32_d3d9_state.m_window.called = true;

	if (!win32_d3d9_state.m_init.created)
	{
		// If DXUTInit() was already called and failed, then fail.
		// DXUTInit() must first succeed for this function to succeed
		if (win32_d3d9_state.m_init.called)
			return E_FAIL;

		// If DXUTInit() hasn't been called, then automatically call it
		// with default params
		hr = win32_d3d9_init();
		if (FAILED(hr))
			return hr;
	}

	if (win32_d3d9_state.m_hwnd.focus == nullptr)
	{
		const ::HINSTANCE INSTANCE_HANDLE = (::HINSTANCE)::GetModuleHandle(nullptr);
		win32_d3d9_state.m_hinstance = INSTANCE_HANDLE;

		char szExePath[MAX_PATH];
		::GetModuleFileNameA(nullptr, szExePath, MAX_PATH);
		const ::HICON ICON_HANDLE = ::ExtractIconA(INSTANCE_HANDLE, szExePath, 0);

		// Register the windows struct
		::WNDCLASS wndClass;
		wndClass.style = CS_DBLCLKS;
		wndClass.lpfnWndProc = __static_wnd_proc;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = INSTANCE_HANDLE;
		wndClass.hIcon = ICON_HANDLE;
		wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wndClass.lpszMenuName = nullptr;
		wndClass.lpszClassName = "Direct3DWindowClass";

		if (!::RegisterClass(&wndClass))
		{
			::DWORD dwError = ::GetLastError();
			if (dwError != ERROR_CLASS_ALREADY_EXISTS)
				return WIN32_DXUT_ERR_MSGBOX("RegisterClass", HRESULT_FROM_WIN32(dwError));
		}

		//int32_t x = CW_USEDEFAULT;
		//int32_t y = CW_USEDEFAULT;
		// Override the window's initial & size position if there were cmd line args
		//if (win32_d3d9_state.m_override_start_x != -1)
		//	x = win32_d3d9_state.m_override_start_x;
		//if (win32_d3d9_state.m_override_start_y != -1)
		//	y = win32_d3d9_state.m_override_start_y;

		//win32_d3d9_state.m_window_created_with_default_positions = false;
		//if (x == CW_USEDEFAULT && y == CW_USEDEFAULT)
		//	win32_d3d9_state.m_window_created_with_default_positions = true;

		// find the window's initial size, but it might be changed later
		//int32_t nDefaultWidth = 640;
		//int32_t nDefaultHeight = 480;
		//if (win32_d3d9_state.m_override_width != 0)
		//	nDefaultWidth = win32_d3d9_state.m_override_width;
		//if (win32_d3d9_state.m_override_height != 0)
		//	nDefaultHeight = win32_d3d9_state.m_override_height;
		::RECT rc{ 0, 0, 640, 480 };

		::AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);	//no menu

		::strcpy_s(win32_d3d9_state.m_window_title, 256, aWindowTitle);

		// create the render window
		::HWND hWnd = ::CreateWindowA(
			"Direct3DWindowClass",
			aWindowTitle,
			aWindowStyle,	//regular style is WS_OVERLAPPEDWINDOW...
			CW_USEDEFAULT, CW_USEDEFAULT,
			(rc.right - rc.left), (rc.bottom - rc.top),
			0,
			nullptr,	//no menu
			INSTANCE_HANDLE,
			0
		);
		if (hWnd == nullptr)
		{
			DWORD dwError = GetLastError();
			return WIN32_DXUT_ERR_MSGBOX("CreateWindow", HRESULT_FROM_WIN32(dwError));
		}

		win32_d3d9_state.m_window.created = true;
		win32_d3d9_state.m_hwnd.focus = hWnd;
		win32_d3d9_state.m_hwnd.device_fullscreen = hWnd;
		win32_d3d9_state.m_hwnd.device_windowed = hWnd;
	}

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Creates a Direct3D device. If DXUTCreateWindow() or DXUTSetWindow() has not already 
// been called, it will call DXUTCreateWindow() with the default parameters.  
// Instead of calling this, you can call DXUTSetDevice() or DXUTCreateDeviceFromSettings() 
//--------------------------------------------------------------------------------------
::HRESULT win32_d3d9_create_device(
	UINT anAdapterOrdinal, bool windowed, int32_t aSuggestedWidth, int32_t aSuggestedHeight,
	win32_d3d9_app_i* anApp)
{
	::HRESULT hr;

	// Not allowed to call this from inside the device callbacks
	if (win32_d3d9_state.m_inside.device_callback)
		return WIN32_DXUT_ERR_MSGBOX("DXUTCreateWindow", E_FAIL);

	// Record the app pointer in the global win32_d3d9_state 
	win32_d3d9_state.app = anApp;

	win32_d3d9_state.m_device.called = true;

	// If DXUTCreateWindow() or DXUTSetWindow() has not already been called, 
	// then call DXUTCreateWindow() with the default parameters.         
	if (!win32_d3d9_state.m_window.created)
	{
		// If DXUTCreateWindow() or DXUTSetWindow() was already called and failed, then fail.
		// DXUTCreateWindow() or DXUTSetWindow() must first succeed for this function to succeed
		if (win32_d3d9_state.m_window.called)
			return E_FAIL;

		// If DXUTCreateWindow() or DXUTSetWindow() hasn't been called, then 
		// automatically call DXUTCreateWindow() with default params
		hr = win32_d3d9_create_window("AutoGeneratedWindow");
		if (FAILED(hr))
			return hr;
	}

	// Force an enumeration with the new IsDeviceAcceptable callback
	__prepare_enumeration_object(true);

	/*
	//log what we enumerated
	{
		const enumeration_t* ENUM = enumeration_t::instance();
		const std::vector<enum_adapter_info_t*>* ADAPTERS = &ENUM->adapter_info_list;
		FS_LOG("found %u display adapter(s)", ADAPTERS->size());
		fs::log::indent();
		for (const enum_adapter_info_t* ADAPTER : *ADAPTERS)
			FS_LOG("adapter ordinal %u = %s", ADAPTER->adapter_ordinal, ADAPTER->description);
		fs::log::undent();
	}
	*/

	match_options_t matchOptions;
	matchOptions.adapter_ordinal = match_type_t::PRESERVE_INPUT;
	matchOptions.device_type = match_type_t::IGNORE_INPUT;
	matchOptions.windowed = match_type_t::PRESERVE_INPUT;
	matchOptions.adapter_format = match_type_t::IGNORE_INPUT;
	matchOptions.vertex_processing = match_type_t::IGNORE_INPUT;
	if (windowed || (aSuggestedWidth != 0 && aSuggestedHeight != 0))
		matchOptions.resolution = match_type_t::CLOSEST_TO_INPUT;
	else
		matchOptions.resolution = match_type_t::IGNORE_INPUT;
	matchOptions.backbuffer_format = match_type_t::IGNORE_INPUT;
	matchOptions.backbuffer_count = match_type_t::IGNORE_INPUT;
	matchOptions.multi_sample = match_type_t::IGNORE_INPUT;
	matchOptions.swap_effect = match_type_t::IGNORE_INPUT;
	matchOptions.depth_format = match_type_t::IGNORE_INPUT;
	matchOptions.stencil_format = match_type_t::IGNORE_INPUT;
	matchOptions.present_flags = match_type_t::IGNORE_INPUT;
	matchOptions.refresh_rate = match_type_t::IGNORE_INPUT;
	matchOptions.present_interval = match_type_t::IGNORE_INPUT;

	win32_d3d9_device_settings_t deviceSettings;
	::ZeroMemory(&deviceSettings, sizeof(win32_d3d9_device_settings_t));
	deviceSettings.adapter_ordinal = anAdapterOrdinal;
	deviceSettings.present_parameters.Windowed = windowed;
	deviceSettings.present_parameters.BackBufferWidth = aSuggestedWidth;
	deviceSettings.present_parameters.BackBufferHeight = aSuggestedHeight;

	// Override with settings from the command line
	//if (win32_d3d9_state.m_override_width != 0)
	//	deviceSettings.present_parameters.BackBufferWidth = win32_d3d9_state.m_override_width;
	//if (win32_d3d9_state.m_override_height != 0)
	//	deviceSettings.present_parameters.BackBufferHeight = win32_d3d9_state.m_override_height;

	//if (win32_d3d9_state.m_override_adapter_ordinal != -1)
	//	deviceSettings.adapter_ordinal = win32_d3d9_state.m_override_adapter_ordinal;

	/*
	if (win32_d3d9_state.m_override_fullscreen)
	{
		deviceSettings.present_parameters.Windowed = FALSE;
		if (win32_d3d9_state.getOverrideWidth() == 0 && win32_d3d9_state.getOverrideHeight() == 0)
			matchOptions.resolution = match_type_t::IGNORE_INPUT;
	}
	*/
	//if (win32_d3d9_state.m_override_windowed)
	//	deviceSettings.present_parameters.Windowed = TRUE;

	/*
	if (win32_d3d9_state.m_override_force_hal)
	{
		deviceSettings.device_type = D3DDEVTYPE_HAL;
		matchOptions.device_type = match_type_t::PRESERVE_INPUT;
	}
	if (win32_d3d9_state.m_override_force_ref)
	{
		deviceSettings.device_type = D3DDEVTYPE_REF;
		matchOptions.device_type = match_type_t::PRESERVE_INPUT;
	}

	if (win32_d3d9_state.m_override_force_pure_hwvp)
	{
		deviceSettings.behavior_flags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
		matchOptions.vertex_processing = match_type_t::PRESERVE_INPUT;
	}
	else if (win32_d3d9_state.m_override_force_hwvp)
	{
		deviceSettings.behavior_flags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		matchOptions.vertex_processing = match_type_t::PRESERVE_INPUT;
	}
	else if (win32_d3d9_state.m_override_force_swvp)
	{
		deviceSettings.behavior_flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		matchOptions.vertex_processing = match_type_t::PRESERVE_INPUT;
	}

	if (win32_d3d9_state.getOverrideForceVsync() == 0)
	{
		deviceSettings.present_parameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		matchOptions.present_interval = match_type_t::PRESERVE_INPUT;
	}
	else if (win32_d3d9_state.getOverrideForceVsync() == 1)
	{
		deviceSettings.present_parameters.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		matchOptions.present_interval = match_type_t::PRESERVE_INPUT;
	}
	*/

	hr = __find_valid_device_settings(&deviceSettings, &deviceSettings, &matchOptions);
	if (FAILED(hr)) // the call will fail if no valid devices were found
	{
		__display_error_message(hr);
		return WIN32_DXUT_ERR("__find_valid_device_settings", hr);
	}

	// Change to a Direct3D device created from the new device settings.  
	// If there is an existing device, then either reset or recreated the scene
	hr = __change_device(&deviceSettings, nullptr, false, true);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Toggle between full screen and windowed
//--------------------------------------------------------------------------------------
::HRESULT win32_d3d9_toggle_fullscreen(const bool aDoToggle)
{
	::HRESULT hr;

	// get the current device settings and flip the windowed win32_d3d9_state then
	// find the closest valid device settings with this change
	win32_d3d9_device_settings_t deviceSettings(__get_device_settings());
	if (aDoToggle)
		deviceSettings.present_parameters.Windowed = !deviceSettings.present_parameters.Windowed;

	match_options_t matchOptions;
	matchOptions.adapter_ordinal = match_type_t::PRESERVE_INPUT;
	matchOptions.device_type = match_type_t::CLOSEST_TO_INPUT;
	matchOptions.windowed = match_type_t::PRESERVE_INPUT;
	matchOptions.adapter_format = match_type_t::IGNORE_INPUT;
	matchOptions.vertex_processing = match_type_t::CLOSEST_TO_INPUT;
	matchOptions.backbuffer_format = match_type_t::IGNORE_INPUT;
	matchOptions.backbuffer_count = match_type_t::CLOSEST_TO_INPUT;
	matchOptions.multi_sample = match_type_t::CLOSEST_TO_INPUT;
	matchOptions.swap_effect = match_type_t::CLOSEST_TO_INPUT;
	matchOptions.depth_format = match_type_t::CLOSEST_TO_INPUT;
	matchOptions.stencil_format = match_type_t::CLOSEST_TO_INPUT;
	matchOptions.present_flags = match_type_t::CLOSEST_TO_INPUT;
	matchOptions.refresh_rate = match_type_t::IGNORE_INPUT;
	matchOptions.present_interval = match_type_t::CLOSEST_TO_INPUT;

	// Go back to previous win32_d3d9_state
	__revert(deviceSettings, matchOptions);

	hr = __find_valid_device_settings(&deviceSettings, &deviceSettings, &matchOptions);
	if (SUCCEEDED(hr))
	{
		// create a Direct3D device using the new device settings.  
		// If there is an existing device, then it will either reset or recreate the scene.
		hr = __change_device(&deviceSettings, nullptr, false, false);

		// If hr == E_ABORT, this means the app rejected the device settings in the ModifySettingsCallback so nothing changed
		if (FAILED(hr) && (hr != E_ABORT))
		{
			// Failed creating device, try to switch back.
			if (aDoToggle)
				deviceSettings.present_parameters.Windowed = !deviceSettings.present_parameters.Windowed;

			__revert(deviceSettings, matchOptions);

			__find_valid_device_settings(&deviceSettings, &deviceSettings, &matchOptions);

			HRESULT hr2 = __change_device(&deviceSettings, nullptr, false, false);
			if (FAILED(hr2))
			{
				// If this failed, then shutdown
				win32_d3d9_shutdown();
			}
		}
	}

	return hr;
}

//--------------------------------------------------------------------------------------
// Pauses time or rendering.  Keeps a ref count so pausing can be layered
//--------------------------------------------------------------------------------------
void win32_d3d9_pause(const char* /*aFunction*/, const char* /*aContext*/, const bool aPauseTime, const bool aPauseRendering)
{
	//FS_LOG("aFunction = %s, aContext = %s, aPauseTime = %u, aPauseRendering = %u", aFunction, aContext, aPauseTime, aPauseRendering);

	//int32_t nPauseTimeCount = win32_d3d9_state.m_pause_time_count;
	win32_d3d9_state.m_pause_time_count += (aPauseTime ? +1 : -1);
	if (win32_d3d9_state.m_pause_time_count < 0)
		win32_d3d9_state.m_pause_time_count = 0;
	//win32_d3d9_state.m_pause_time_count = nPauseTimeCount;

	//int32_t nPauseRenderingCount = win32_d3d9_state.m_pause_rendering_count;
	win32_d3d9_state.m_pause_rendering_count += (aPauseRendering ? +1 : -1);
	if (win32_d3d9_state.m_pause_rendering_count < 0)
		win32_d3d9_state.m_pause_rendering_count = 0;
	//win32_d3d9_state.m_pause_rendering_count = nPauseRenderingCount;

	if (win32_d3d9_state.m_pause_time_count > 0)
	{
		// stop the scene from animating
		__timer().stop();
	}
	else
	{
		// Restart the timer
		__timer().start();
	}

	//win32_d3d9_state.m_rendering_paused = win32_d3d9_state.m_pause_rendering_count > 0;
	//win32_d3d9_state.m_time_paused = nPauseTimeCount > 0;
}

static void __do_frame(::IDirect3DDevice9* device, const float now, const float elapsed)
{
	assert(device);

	// Animate the scene by calling the app's frame move callback
	if (win32_d3d9_state.app)
	{
		win32_d3d9_state.app->win32_d3d9_app_frame_move(now, elapsed);
		++win32_d3d9_state.app->_win32_d3d9_app_frame_moves;
		if (nullptr == win32_d3d9_state.m_d3d_device)// Handle shutdown from inside callback
			return;
	}

	if (!win32_d3d9_is_rendering_paused())
	{
		::HRESULT hr;
		bool shouldPresent = false;

		// render the scene by calling the app's render callback
		if (win32_d3d9_state.app)
		{
			shouldPresent = win32_d3d9_state.app->win32_d3d9_app_frame_render(now, elapsed);
			if (nullptr == win32_d3d9_state.m_d3d_device)// Handle shutdown from inside callback
				return;
		}

#if defined(DEBUG) || defined(_DEBUG)
		// The back buffer should always match the client rect 
		// if the Direct3D backbuffer covers the entire window
		::RECT rcClient;
		::GetClientRect(win32_d3d9_hwnd(), &rcClient);
		if (!::IsIconic(win32_d3d9_hwnd()))
		{
			::GetClientRect(win32_d3d9_hwnd(), &rcClient);
			assert(win32_d3d9_state.m_backbuffer_surface_desc.Width == (UINT)rcClient.right);
			assert(win32_d3d9_state.m_backbuffer_surface_desc.Height == (UINT)rcClient.bottom);
		}
#endif

		// Show the frame on the primary surface?
		if (shouldPresent)
		{
			hr = device->Present(nullptr, nullptr, nullptr, nullptr);
			if (FAILED(hr))
			{
				if (D3DERR_DEVICELOST == hr)
				{
					win32_d3d9_state.m_device_lost = true;
				}
				else if (D3DERR_DRIVERINTERNALERROR == hr)
				{
					// When D3DERR_DRIVERINTERNALERROR is returned from Present(),
					// the application can do one of the following:
					// 
					// - End, with the pop-up window saying that the application cannot continue 
					//   because of problems in the display adapter and that the user should 
					//   contact the adapter manufacturer.
					//
					// - Attempt to restart by calling IDirect3DDevice9::reset, which is essentially the same 
					//   path as recovering from a lost device. If IDirect3DDevice9::reset fails with 
					//   D3DERR_DRIVERINTERNALERROR, the application should end immediately with the message 
					//   that the user should contact the adapter manufacturer.
					// 
					// The framework attempts the path of resetting the device
					// 
					win32_d3d9_state.m_device_lost = true;
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------
// render the 3D environment by:
//      - Checking if the device is lost and trying to reset it if it is
//      - get the elapsed time since the last frame
//      - Calling the app's framemove and render callback
//      - Calling Present()
//--------------------------------------------------------------------------------------
static void __do_idle_time(win32_d3d9_app_i* anApp)
{
	::HRESULT hr;

	if (win32_d3d9_state.m_device_lost || win32_d3d9_is_rendering_paused() || !win32_d3d9_state.m_active)
	{
		// Window is minimized or paused so yield CPU time to other processes
		if (!anApp || !anApp->win32_d3d9_app_is_fixed_tick_rate())
		{
			::Sleep(50);	//default 50 msecs
		}
		else
		{
			//now this is up to the app; in some cases you don't want any sleeping in the background
			const uint32_t BGSM = anApp->win32_d3d9_app_fixed_tick_background_sleep_milliseconds();
			if (BGSM)
				::Sleep(BGSM);
		}
	}

	if (nullptr == win32_d3d9_state.m_d3d_device)
	{
		if (win32_d3d9_state.m_device_lost)
		{
			win32_d3d9_device_settings_t deviceSettings(__get_device_settings());
			__change_device(&deviceSettings, nullptr, false, true);
		}

		return;
	}

	if (win32_d3d9_state.m_device_lost && !win32_d3d9_is_rendering_paused())
	{
		// Test the cooperative level to see if it's okay to render
		if (FAILED(hr = win32_d3d9_state.m_d3d_device->TestCooperativeLevel()))
		{
			if (D3DERR_DEVICELOST == hr)
			{
				// The device has been lost but cannot be reset at this time.  
				// So wait until it can be reset.
				return;
			}

			// If we are windowed, read the desktop format and 
			// ensure that the Direct3D device is using the same format 
			// since the user could have changed the desktop bitdepth 
			if (win32_d3d9_is_windowed())
			{
				::D3DDISPLAYMODE adapterDesktopDisplayMode;
				const win32_d3d9_device_settings_t* DEVICE_SETTINGS = win32_d3d9_state.m_device_settings;
				win32_d3d9_state.m_d3d->GetAdapterDisplayMode(DEVICE_SETTINGS->adapter_ordinal, &adapterDesktopDisplayMode);
				if (DEVICE_SETTINGS->adapter_format != adapterDesktopDisplayMode.Format)
				{
					match_options_t matchOptions;
					matchOptions.adapter_ordinal = match_type_t::PRESERVE_INPUT;
					matchOptions.device_type = match_type_t::PRESERVE_INPUT;
					matchOptions.windowed = match_type_t::PRESERVE_INPUT;
					matchOptions.adapter_format = match_type_t::PRESERVE_INPUT;
					matchOptions.vertex_processing = match_type_t::CLOSEST_TO_INPUT;
					matchOptions.resolution = match_type_t::CLOSEST_TO_INPUT;
					matchOptions.backbuffer_format = match_type_t::CLOSEST_TO_INPUT;
					matchOptions.backbuffer_count = match_type_t::CLOSEST_TO_INPUT;
					matchOptions.multi_sample = match_type_t::CLOSEST_TO_INPUT;
					matchOptions.swap_effect = match_type_t::CLOSEST_TO_INPUT;
					matchOptions.depth_format = match_type_t::CLOSEST_TO_INPUT;
					matchOptions.stencil_format = match_type_t::CLOSEST_TO_INPUT;
					matchOptions.present_flags = match_type_t::CLOSEST_TO_INPUT;
					matchOptions.refresh_rate = match_type_t::CLOSEST_TO_INPUT;
					matchOptions.present_interval = match_type_t::CLOSEST_TO_INPUT;

					win32_d3d9_device_settings_t deviceSettings = __get_device_settings();
					deviceSettings.adapter_format = adapterDesktopDisplayMode.Format;

					hr = __find_valid_device_settings(&deviceSettings, &deviceSettings, &matchOptions);
					if (FAILED(hr)) // the call will fail if no valid devices were found
					{
						__display_error_message(WIN32_DXUTERR_NOCOMPATIBLEDEVICES);
						win32_d3d9_shutdown();
					}

					// Change to a Direct3D device created from the new device settings.  
					// If there is an existing device, then either reset or recreate the scene
					hr = __change_device(&deviceSettings, nullptr, false, false);
					if (FAILED(hr))
					{
						// If this fails, try to go fullscreen and if this fails also shutdown.
						if (FAILED(win32_d3d9_toggle_fullscreen()))
							win32_d3d9_shutdown();
					}

					return;
				}
			}

			// Try to reset the device
			if (FAILED(hr = __reset_3d_environment()))
			{
				//FS_ERROR("__reset3DEnvironment() failed");

				if (D3DERR_DEVICELOST == hr)
				{
					// The device was lost again, so continue waiting until it can be reset.
					return;
				}
				else if (WIN32_DXUTERR_RESETTINGDEVICEOBJECTS == hr || WIN32_DXUTERR_MEDIANOTFOUND == hr)
				{
					__display_error_message(hr);
					win32_d3d9_shutdown();
					return;
				}
				else
				{
					// reset failed, but the device wasn't lost so something bad happened, 
					// so recreate the device to try to recover
					if (FAILED(__change_device(win32_d3d9_state.m_device_settings, nullptr, true, false)))
					{
						win32_d3d9_shutdown();
						return;
					}
				}
			}
		}

		win32_d3d9_state.m_device_lost = false;
	}

	//fixed update central
	//fixed update central
	//fixed update central
	//fixed update central
	{
		// get the app's time, in seconds. Skip rendering if no time elapsed
		const w32_timer_values_t TIME_VALUES = __timer().mutate_and_get_values();

		if (anApp && anApp->win32_d3d9_app_is_fixed_tick_rate())
		{
			static float elapsed = 0.f;
			static double now = 0.f;
			const float FIXED_ELAPSED = anApp->win32_d3d9_app_seconds_per_fixed_tick();

			//don't want to handle huge time spans, better to wait for it to stabilize
			if (TIME_VALUES.elapsed < FIXED_ELAPSED)
			{
				elapsed += TIME_VALUES.elapsed;

				//if we need to run a fixed tick...
				if (elapsed > FIXED_ELAPSED)
				{
					uint32_t ticks = 1;
					while (elapsed > FIXED_ELAPSED)
					{
						assert(1 == ticks);

						__do_frame(win32_d3d9_state.m_d3d_device, (float)now, FIXED_ELAPSED);
						now += FIXED_ELAPSED;

						elapsed -= FIXED_ELAPSED;
						assert(elapsed < FIXED_ELAPSED);
						++ticks;
					}
				}
				//otherwise we do "off tick" processing...
				else
				{
					anApp->win32_d3d9_app_do_off_tick_processing(now);
				}
			}
			else
			{
				++anApp->_win32_d3d9_app_frame_drops;
			}
		}
		else
		{
			//do frame
			__do_frame(win32_d3d9_state.m_d3d_device, (float)TIME_VALUES.time, TIME_VALUES.elapsed);

			//do "off tick" processing also
			if (anApp)
				anApp->win32_d3d9_app_do_off_tick_processing(TIME_VALUES.time);
		}
	}
	//fixed update central
	//fixed update central
	//fixed update central
	//fixed update central

	/*
	// update current frame #
	{
		++win32_d3d9_state.m_current_frame_number;

		// Check to see if the app should shutdown due to cmdline
		if (0 != win32_d3d9_state.getOverrideQuitAfterFrame())
		{
			if (win32_d3d9_state.m_current_frame_number > win32_d3d9_state.getOverrideQuitAfterFrame())
				shutdown();
		}
	}
	*/
}

//--------------------------------------------------------------------------------------
// Handles app's message loop and rendering when idle.  If DXUTCreateDevice*() or DXUTSetDevice() 
// has not already been called, it will call DXUTCreateWindow() with the default parameters.  
//--------------------------------------------------------------------------------------
::HRESULT win32_d3d9_main_loop(win32_d3d9_app_i* anApp)
{
	// Not allowed to call this from inside the device callbacks or reenter
	if (win32_d3d9_state.m_inside.device_callback || win32_d3d9_state.m_inside.main_loop)
	{
		if ((win32_d3d9_state.m_exit_code == 0) || (win32_d3d9_state.m_exit_code == 11))
			win32_d3d9_state.m_exit_code = 1;
		return WIN32_DXUT_ERR_MSGBOX("MainLoop", E_FAIL);
	}

	win32_d3d9_state.m_inside.main_loop = true;

	// If DXUTCreateDevice*() or DXUTSetDevice() has not already been called, 
	// then call DXUTCreateDevice() with the default parameters.         
	if (!win32_d3d9_state.m_device.created)
		return E_FAIL;

	::HWND hWnd = win32_d3d9_hwnd();

	// DXUTInit() must have been called and succeeded for this function to proceed
	// DXUTCreateWindow() or DXUTSetWindow() must have been called and succeeded for this function to proceed
	// DXUTCreateDevice() or DXUTCreateDeviceFromSettings() or DXUTSetDevice() must have been called and succeeded for this function to proceed
	if (!win32_d3d9_state.m_init.created || !win32_d3d9_state.m_window.created || !win32_d3d9_state.m_device.created)
	{
		if ((win32_d3d9_state.m_exit_code == 0) || (win32_d3d9_state.m_exit_code == 11))
			win32_d3d9_state.m_exit_code = 1;
		return WIN32_DXUT_ERR_MSGBOX("MainLoop", E_FAIL);
	}

	// Now we're ready to receive and process Windows messages.
	bool bGotMsg;
	MSG  msg;
	msg.message = WM_NULL;
	PeekMessage(&msg, nullptr, 0U, 0U, PM_NOREMOVE);

	while (WM_QUIT != msg.message)
	{
		// Use PeekMessage() so we can use idle time to render the scene. 
		bGotMsg = (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE) != 0);

		if (bGotMsg)
		{
			// Translate and dispatch the message
			if (hWnd == nullptr || 0 == TranslateAccelerator(hWnd, nullptr, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			// render a frame during idle time (no messages are waiting)
			__do_idle_time(anApp);
		}
	}

	win32_d3d9_state.m_inside.main_loop = false;

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Closes down the window.  When the window closes, it will cleanup everything
//--------------------------------------------------------------------------------------
void win32_d3d9_shutdown(const int32_t nExitCode)
{
	HWND hWnd = win32_d3d9_hwnd();
	if (hWnd != nullptr)
		::SendMessage(hWnd, WM_CLOSE, 0, 0);

	win32_d3d9_state.m_exit_code = nExitCode;

	__cleanup_3d_environment(true);

	// Restore shortcut keys (Windows key, accessibility shortcuts) to original win32_d3d9_state
	// This is important to call here if the shortcuts are disabled, 
	// because accessibility setting changes are permanent.
	// This means that if this is not done then the accessibility settings 
	// might not be the same as when the app was started. 
	// If the app crashes without restoring the settings, this is also true so it
	// would be wise to backup/restore the settings from a file so they can be 
	// restored when the crashed app is run again.
	__allow_shortcut_keys(true);

	win32_d3d9_state.m_d3d_enumeration = nullptr;

	SAFE_RELEASE(win32_d3d9_state.m_d3d);
	win32_d3d9_state.m_d3d = nullptr;
}

void win32_d3d9_set_cursor_settings(const bool bShowCursorWhenFullScreen, const bool bClipCursorWhenFullScreen)
{
	win32_d3d9_state.m_clip_cursor_when_fullscreen = bClipCursorWhenFullScreen;
	win32_d3d9_state.m_show_cursor_when_fullscreen = bShowCursorWhenFullScreen;
	__setup_cursor();
}
