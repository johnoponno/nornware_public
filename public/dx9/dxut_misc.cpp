#include "stdafx.h"
#include "state.h"

namespace dx9
{
	//--------------------------------------------------------------------------------------
	// MsgProc for DXUTDisplaySwitchingToREFWarning() dialog box
	//--------------------------------------------------------------------------------------
	static INT_PTR CALLBACK __display_switching_to_ref_warning_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM)
	{
		switch (message)
		{
		case WM_INITDIALOG:
			// Easier to set text here than in the DLGITEMTEMPLATE
			::SetWindowTextA(hDlg, win32_d3d9_state.m_window_title);
			::SendMessage(::GetDlgItem(hDlg, 0x100), STM_SETIMAGE, IMAGE_ICON, (LPARAM)::LoadIcon(0, IDI_QUESTION));
			::SetDlgItemText(hDlg, 0x101, "Switching to the Direct3D reference rasterizer, a software device\nthat implements the entire Direct3D feature set, but runs very slowly.\nDo you wish to continue?");
			::SetDlgItemText(hDlg, IDYES, "&Yes");
			::SetDlgItemText(hDlg, IDNO, "&No");
			::SetDlgItemText(hDlg, IDIGNORE, "&Don't show again");
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDIGNORE: ::CheckDlgButton(hDlg, IDIGNORE, (IsDlgButtonChecked(hDlg, IDIGNORE) == BST_CHECKED) ? BST_UNCHECKED : BST_CHECKED); EnableWindow(GetDlgItem(hDlg, IDNO), (IsDlgButtonChecked(hDlg, IDIGNORE) != BST_CHECKED)); break;
			case IDNO: ::EndDialog(hDlg, (IsDlgButtonChecked(hDlg, IDIGNORE) == BST_CHECKED) ? IDNO | 0x80 : IDNO | 0x00); return TRUE;
			case IDCANCEL:
			case IDYES: ::EndDialog(hDlg, (IsDlgButtonChecked(hDlg, IDIGNORE) == BST_CHECKED) ? IDYES | 0x80 : IDYES | 0x00); return TRUE;
			}
			break;
		}
		return FALSE;
	}

	//--------------------------------------------------------------------------------------
	// Shared code for samples to ask user if they want to use a REF device or quit
	//--------------------------------------------------------------------------------------
	void display_switching_to_ref_warning()
	{
		// Open the appropriate registry key
		DWORD dwSkipWarning = 0;
		HKEY hKey;
		LONG lResult = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\DirectX 9.0 SDK", 0, KEY_READ, &hKey);
		if (ERROR_SUCCESS == lResult)
		{
			DWORD dwType;
			DWORD dwSize = sizeof(DWORD);
			lResult = RegQueryValueEx(hKey, "Skip Warning On REF", nullptr, &dwType, (BYTE*)&dwSkipWarning, &dwSize);
			RegCloseKey(hKey);
		}

		if (dwSkipWarning == 0)
		{
			// Compact code to create a custom dialog box without using a template in a resource file.
			// If this dialog were in a .rc file, this would be a lot simpler but every sample calling this function would
			// need a copy of the dialog in its own .rc file. Also MessageBox API could be used here instead, but 
			// the MessageBox API is simpler to call but it can't provide a "Don't show again" checkbox
			typedef struct { DLGITEMTEMPLATE a; WORD b; WORD c; WORD d; WORD e; WORD f; } DXUT_DLG_ITEM;
			typedef struct { DLGTEMPLATE a; WORD b; WORD c; char d[2]; WORD e; char f[14]; DXUT_DLG_ITEM i1; DXUT_DLG_ITEM i2; DXUT_DLG_ITEM i3; DXUT_DLG_ITEM i4; DXUT_DLG_ITEM i5; } DXUT_DLG_DATA;

			DXUT_DLG_DATA dtp =
			{
				{ WS_CAPTION | WS_POPUP | WS_VISIBLE | WS_SYSMENU | DS_ABSALIGN | DS_3DLOOK | DS_SETFONT | DS_MODALFRAME | DS_CENTER,0,5,0,0,269,82 },0,0," ",8,"MS Sans Serif",
			{ { WS_CHILD | WS_VISIBLE | SS_ICON | SS_CENTERIMAGE,0,7,7,24,24,0x100 },0xFFFF,0x0082,0,0,0 }, // icon
			{ { WS_CHILD | WS_VISIBLE,0,40,7,230,25,0x101 },0xFFFF,0x0082,0,0,0 }, // static text
			{ { WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,0,80,39,50,14,IDYES },0xFFFF,0x0080,0,0,0 }, // Yes button
			{ { WS_CHILD | WS_VISIBLE,0,133,39,50,14,IDNO },0xFFFF,0x0080,0,0,0 }, // No button
			{ { WS_CHILD | WS_VISIBLE | BS_CHECKBOX,0,7,59,70,16,IDIGNORE },0xFFFF,0x0080,0,0,0 }, // checkbox
			};

			int32_t nResult = (int32_t) ::DialogBoxIndirect(win32_d3d9_state.m_hinstance, (DLGTEMPLATE*)&dtp, win32_d3d9_hwnd(), __display_switching_to_ref_warning_proc);

			if ((nResult & 0x80) == 0x80) // "Don't show again" checkbox was checked
			{
				lResult = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\DirectX 9.0 SDK", 0, KEY_WRITE, &hKey);
				if (ERROR_SUCCESS == lResult)
				{
					dwSkipWarning = 1;
					RegSetValueEx(hKey, "Skip Warning On REF", 0, REG_DWORD, (BYTE*)&dwSkipWarning, sizeof(DWORD));
					RegCloseKey(hKey);
				}
			}

			// User choose not to continue
			if ((nResult & 0x0F) == IDNO)
				win32_d3d9_shutdown(1);
		}
	}

	//--------------------------------------------------------------------------------------
	// Multimon API handling for OSes with or without multimon API support
	//--------------------------------------------------------------------------------------
#define DXUT_PRIMARY_MONITOR ((HMONITOR)0x12340042)
	typedef HMONITOR(WINAPI* LPMONITORFROMWINDOW)(HWND, DWORD);
	typedef BOOL(WINAPI* LPGETMONITORINFO)(HMONITOR, LPMONITORINFO);

	BOOL get_monitor_info(HMONITOR hMonitor, LPMONITORINFO lpMonitorInfo)
	{
		static bool s_bInited = false;
		static LPGETMONITORINFO s_pFnGetMonitorInfo = nullptr;
		if (!s_bInited)
		{
			s_bInited = true;
			::HMODULE hUser32 = GetModuleHandle("USER32");
			if (hUser32)
			{
				s_pFnGetMonitorInfo = (LPGETMONITORINFO)GetProcAddress(hUser32, "GetMonitorInfoA");
			}
		}

		if (s_pFnGetMonitorInfo)
			return s_pFnGetMonitorInfo(hMonitor, lpMonitorInfo);

		RECT rcWork;
		if ((hMonitor == DXUT_PRIMARY_MONITOR) && lpMonitorInfo && (lpMonitorInfo->cbSize >= sizeof(MONITORINFO)) && SystemParametersInfoA(SPI_GETWORKAREA, 0, &rcWork, 0))
		{
			lpMonitorInfo->rcMonitor.left = 0;
			lpMonitorInfo->rcMonitor.top = 0;
			lpMonitorInfo->rcMonitor.right = GetSystemMetrics(SM_CXSCREEN);
			lpMonitorInfo->rcMonitor.bottom = GetSystemMetrics(SM_CYSCREEN);
			lpMonitorInfo->rcWork = rcWork;
			lpMonitorInfo->dwFlags = MONITORINFOF_PRIMARY;
			return TRUE;
		}
		return FALSE;
	}

	HMONITOR monitor_from_window(HWND hWnd, DWORD dwFlags)
	{
		static bool s_bInited = false;
		static LPMONITORFROMWINDOW s_pFnGetMonitorFronWindow = nullptr;
		if (!s_bInited)
		{
			s_bInited = true;
			HMODULE hUser32 = GetModuleHandle("USER32");
			if (hUser32) s_pFnGetMonitorFronWindow = (LPMONITORFROMWINDOW)GetProcAddress(hUser32, "MonitorFromWindow");
		}

		if (s_pFnGetMonitorFronWindow)
			return s_pFnGetMonitorFronWindow(hWnd, dwFlags);
		if (dwFlags & (MONITOR_DEFAULTTOPRIMARY | MONITOR_DEFAULTTONEAREST))
			return DXUT_PRIMARY_MONITOR;
		return nullptr;
	}

	//--------------------------------------------------------------------------------------
	// Display error msg box to help debug 
	//--------------------------------------------------------------------------------------
	::HRESULT WINAPI trace(const char* strFile, DWORD dwLine, HRESULT hr, const char* strMsg, bool bPopMsgBox)
	{
		if (bPopMsgBox && true == false)
			bPopMsgBox = false;

		return ::DXTraceA(strFile, dwLine, hr, strMsg, bPopMsgBox);
	}

	/*
	void frustum_from_matrix(const ::D3DXMATRIX& aMatrix, const bool aNormalize, core::frustum_t& aFrustum)
	{
		// Left clipping plane
		aFrustum.planes[core::frustum_t::P_LEFT].normal.x = aMatrix._14 + aMatrix._11;
		aFrustum.planes[core::frustum_t::P_LEFT].normal.y = aMatrix._24 + aMatrix._21;
		aFrustum.planes[core::frustum_t::P_LEFT].normal.z = aMatrix._34 + aMatrix._31;
		aFrustum.planes[core::frustum_t::P_LEFT].determinant = aMatrix._44 + aMatrix._41;

		// Right clipping plane
		aFrustum.planes[core::frustum_t::P_RIGHT].normal.x = aMatrix._14 - aMatrix._11;
		aFrustum.planes[core::frustum_t::P_RIGHT].normal.y = aMatrix._24 - aMatrix._21;
		aFrustum.planes[core::frustum_t::P_RIGHT].normal.z = aMatrix._34 - aMatrix._31;
		aFrustum.planes[core::frustum_t::P_RIGHT].determinant = aMatrix._44 - aMatrix._41;

		// Top clipping plane
		aFrustum.planes[core::frustum_t::P_TOP].normal.x = aMatrix._14 - aMatrix._12;
		aFrustum.planes[core::frustum_t::P_TOP].normal.y = aMatrix._24 - aMatrix._22;
		aFrustum.planes[core::frustum_t::P_TOP].normal.z = aMatrix._34 - aMatrix._32;
		aFrustum.planes[core::frustum_t::P_TOP].determinant = aMatrix._44 - aMatrix._42;

		// Bottom clipping plane
		aFrustum.planes[core::frustum_t::P_BOTTOM].normal.x = aMatrix._14 + aMatrix._12;
		aFrustum.planes[core::frustum_t::P_BOTTOM].normal.y = aMatrix._24 + aMatrix._22;
		aFrustum.planes[core::frustum_t::P_BOTTOM].normal.z = aMatrix._34 + aMatrix._32;
		aFrustum.planes[core::frustum_t::P_BOTTOM].determinant = aMatrix._44 + aMatrix._42;

		// Near clipping plane
		aFrustum.planes[core::frustum_t::P_NEAR].normal.x = aMatrix._13;
		aFrustum.planes[core::frustum_t::P_NEAR].normal.y = aMatrix._23;
		aFrustum.planes[core::frustum_t::P_NEAR].normal.z = aMatrix._33;
		aFrustum.planes[core::frustum_t::P_NEAR].determinant = aMatrix._43;

		// Far clipping plane
		//aFrustum.planes[core::Frustum::P_FAR].myNormal.x = aMatrix._14 - aMatrix._13;
		//aFrustum.planes[core::Frustum::P_FAR].myNormal.y = aMatrix._24 - aMatrix._23;
		//aFrustum.planes[core::Frustum::P_FAR].myNormal.z = aMatrix._34 - aMatrix._33;
		//aFrustum.planes[core::Frustum::P_FAR].myDeterminant = aMatrix._44 - aMatrix._43;

		// Normalize the plane equations, if requested
		if (aNormalize)
		{
			for (uint32_t i = 0; i < core::frustum_t::NUM_PLANES; ++i)
				aFrustum.planes[i] = normalized(aFrustum.planes[i]);
		}
	}
	*/
}