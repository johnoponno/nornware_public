#pragma once

namespace dx9
{
	struct app_i;

	struct device_settings_t
	{
		::UINT adapter_ordinal;
		::D3DDEVTYPE device_type;
		::D3DFORMAT adapter_format;
		::DWORD behavior_flags;
		::D3DPRESENT_PARAMETERS present_parameters;
	};

	//--------------------------------------------------------------------------------------
	// Error codes
	//--------------------------------------------------------------------------------------
#define DXUTERR_NODIRECT3D              MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0901)
#define DXUTERR_NOCOMPATIBLEDEVICES     MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0902)
#define DXUTERR_MEDIANOTFOUND           MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0903)
#define DXUTERR_NONZEROREFCOUNT         MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0904)
#define DXUTERR_CREATINGDEVICE          MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0905)
#define DXUTERR_RESETTINGDEVICE         MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0906)
#define DXUTERR_CREATINGDEVICEOBJECTS   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0907)
#define DXUTERR_RESETTINGDEVICEOBJECTS  MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0908)
#define DXUTERR_INCORRECTVERSION        MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0909)


	//--------------------------------------------------------------------------------------
	// Initialization
	//--------------------------------------------------------------------------------------
	::HRESULT init();

	// Choose either DXUTCreateWindow or DXUTSetWindow.  If using DXUTSetWindow, consider using StaticWndProc
	::HRESULT create_window(const char* aWindowTitle, const ::DWORD aWindowStyle = 0);

	// Choose either DXUTCreateDevice or DXUTSetDevice or DXUTCreateDeviceFromSettings
	::HRESULT create_device(const uint32_t AdapterOrdinal, const bool bWindowed, const int32_t nSuggestedWidth, const int32_t nSuggestedHeight, app_i* anApp);

	// Choose either MainLoop or implement your own main loop 
	::HRESULT main_loop(app_i* anApp);

	//--------------------------------------------------------------------------------------
	// Common Tasks 
	//--------------------------------------------------------------------------------------
	void set_cursor_settings(const bool bShowCursorWhenFullScreen, const bool bClipCursorWhenFullScreen);
	::HRESULT toggle_fullscreen(const bool aDoToggle = true);
	void pause(const char* aFunction, const char* aContext, const bool bPauseTime, const bool bPauseRendering);
	void shutdown(const int32_t nExitCode = 0);
}
