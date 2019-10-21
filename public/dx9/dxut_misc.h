#pragma once

namespace dx9
{
	//--------------------------------------------------------------------------------------
	// Shared code for samples to ask user if they want to use a REF device or quit
	//--------------------------------------------------------------------------------------
	extern void display_switching_to_ref_warning();

	extern ::HRESULT WINAPI trace(const CHAR* strFile, DWORD dwLine, HRESULT hr, const char* strMsg, bool bPopMsgBox);

	// These macros are very similar to dxerr's but it special cases the HRESULT defined
	// by DXUT to pop better message boxes. 
#if defined(DEBUG) || defined(_DEBUG)
#define DXUT_ERR(str,hr)           trace( __FILE__, (DWORD)__LINE__, hr, str, false )
#define DXUT_ERR_MSGBOX(str,hr)    trace( __FILE__, (DWORD)__LINE__, hr, str, true )
	//#define DXUTTRACE                  DebugString
#else
#define DXUT_ERR(str,hr)           (hr)
#define DXUT_ERR_MSGBOX(str,hr)    (hr)
#endif

	//--------------------------------------------------------------------------------------
	// Multimon handling to support OSes with or without multimon API support.  
	// Purposely avoiding the use of multimon.h so DXUT.lib doesn't require 
	// COMPILE_MULTIMON_STUBS and cause complication with MFC or other users of multimon.h
	//--------------------------------------------------------------------------------------
#ifndef MONITOR_DEFAULTTOPRIMARY
#define MONITORINFOF_PRIMARY        0x00000001
#define MONITOR_DEFAULTTONULL       0x00000000
#define MONITOR_DEFAULTTOPRIMARY    0x00000001
#define MONITOR_DEFAULTTONEAREST    0x00000002
	typedef struct tagMONITORINFO
	{
		DWORD   cbSize;
		RECT    rcMonitor;
		RECT    rcWork;
		DWORD   dwFlags;
	} MONITORINFO, *LPMONITORINFO;
	typedef struct tagMONITORINFOEXW : public tagMONITORINFO
	{
		char       szDevice[CCHDEVICENAME];
	} MONITORINFOEXW, *LPMONITORINFOEXW;
	typedef MONITORINFOEXW MONITORINFOEX;
	typedef LPMONITORINFOEXW LPMONITORINFOEX;
#endif

	extern HMONITOR monitor_from_window(HWND hWnd, DWORD dwFlags);
	extern BOOL get_monitor_info(HMONITOR hMonitor, LPMONITORINFO lpMonitorInfo);

	//extern void frustum_from_matrix(const ::D3DXMATRIX& aMatrix, const bool aNormalize, core::frustum_t& aFrustum);
}
