#pragma once

struct w32_d3d9_app_i;

//--------------------------------------------------------------------------------------
// Error codes
//--------------------------------------------------------------------------------------
#define WIN32_DXUTERR_NODIRECT3D              MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0901)
#define WIN32_DXUTERR_NOCOMPATIBLEDEVICES     MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0902)
#define WIN32_DXUTERR_MEDIANOTFOUND           MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0903)
#define WIN32_DXUTERR_NONZEROREFCOUNT         MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0904)
#define WIN32_DXUTERR_CREATINGDEVICE          MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0905)
#define WIN32_DXUTERR_RESETTINGDEVICE         MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0906)
#define WIN32_DXUTERR_CREATINGDEVICEOBJECTS   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0907)
#define WIN32_DXUTERR_RESETTINGDEVICEOBJECTS  MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0908)
#define WIN32_DXUTERR_INCORRECTVERSION        MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0909)

struct w32_d3d9_device_settings_t
{
	::UINT adapter_ordinal;
	::D3DDEVTYPE device_type;
	::D3DFORMAT adapter_format;
	::DWORD behavior_flags;
	::D3DPRESENT_PARAMETERS present_parameters;
};

::HRESULT w32_d3d9_init();
void w32_d3d9_pause(const char* aFunction, const char* aContext, const bool bPauseTime, const bool bPauseRendering);
void w32_d3d9_shutdown(const int32_t nExitCode = 0);
::HRESULT w32_d3d9_toggle_fullscreen(const bool aDoToggle = true);
::HRESULT w32_d3d9_create_window(const char* aWindowTitle, const ::DWORD aWindowStyle = 0);
void w32_d3d9_set_cursor_settings(const bool bShowCursorWhenFullScreen, const bool bClipCursorWhenFullScreen);
::HRESULT w32_d3d9_create_device(const uint32_t AdapterOrdinal, const bool bWindowed, const int32_t nSuggestedWidth, const int32_t nSuggestedHeight, w32_d3d9_app_i* anApp);
::HRESULT w32_d3d9_main_loop(w32_d3d9_app_i* anApp);
