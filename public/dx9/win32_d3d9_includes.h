#pragma once

#ifndef STRICT
#define STRICT
#endif

// If app hasn't chosen, set to work with Windows 98, Windows Me, Windows 2000, Windows XP and beyond
#ifndef WINVER
#define WINVER         0x0410
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410 
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT   0x0500 
#endif

// #define DXUT_AUTOLIB to automatically include the libs needed for DXUT 
#ifdef DXUT_AUTOLIB
#pragma comment( lib, "dxerr.lib" )
#pragma comment( lib, "dxguid.lib" )
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment( lib, "d3dx9d.lib" )
#else
#pragma comment( lib, "d3dx9.lib" )
#endif
#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "comctl32.lib" )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <wchar.h>
#include <mmsystem.h>
#include <commctrl.h> // for InitCommonControls() 
#include <shellapi.h> // for ExtractIcon()
#include <new.h>      // for placement new
#include <math.h>      
#include <limits.h>      
#include <stdio.h>
#include <XInput.h> // Header for XInput APIs
#include <string>

// CRT's memory leak detection
#if defined(DEBUG) || defined(_DEBUG)
#include <crtdbg.h>
#endif

// Enable extra D3D debugging in debug builds if using the debug DirectX runtime.  
// This makes D3D objects work well in the debugger watch window, but slows down 
// performance slightly.
#if defined(DEBUG) || defined(_DEBUG)
#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif
#endif

// Direct3D includes
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>

#include <strsafe.h>

#include "win32_dxut.h"
#include "win32_dxut_misc.h"
#include "win32_dxut_enum.h"

#if defined(DEBUG) || defined(_DEBUG)
	#ifndef VERIFY
		#define VERIFY(x)           { hr = x; if( FAILED(hr) ) { win32_d3d9_trace( __FILE__, (DWORD)__LINE__, hr, #x, true ); } }
	#endif
	#ifndef VERIFY_RETURN
		#define VERIFY_RETURN(x)    { hr = x; if( FAILED(hr) ) { return win32_d3d9_trace( __FILE__, (DWORD)__LINE__, hr, #x, true ); } }
	#endif
#else
	#ifndef VERIFY
		#define VERIFY(x)           { hr = x; }
	#endif
	#ifndef VERIFY_RETURN
		#define VERIFY_RETURN(x)    { hr = x; if( FAILED(hr) ) { return hr; } }
	#endif
#endif

#ifndef SAFE_DELETE
	#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=nullptr; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
	#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=nullptr; } }
#endif    
#ifndef SAFE_RELEASE
	#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=nullptr; } }
#endif

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

/*
namespace dx9
{
	struct vec3f_t
	{
		float x;
		float y;
		float z;
	};

	struct vec2f_t
	{
		float x;
		float y;
	};
}
*/
