#include "stdafx.h"

#include "../../dx9/state.h"
#include "tpmn_app.h"

//--------------------------------------------------------------------------------------
// Local variables
//--------------------------------------------------------------------------------------
static tpmn_app_t __app;

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int32_t)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Show the cursor and clip it when in full screen
	dx9::set_cursor_settings(false, true);

	// Initialize DXUT and create the desired Win32 window and Direct3D 
	// device for the application. Calling each of these functions is optional, but they
	// allow you to set several options which control the behavior of the framework.
	dx9::init();
	dx9::create_window("Twisted Pair Man Nightmare (c) 2011-2019 nornware AB");
	{
		const ::HRESULT err = dx9::create_device(D3DADAPTER_DEFAULT, true, ::GetSystemMetrics(SM_CXSCREEN) / 2, ::GetSystemMetrics(SM_CYSCREEN) / 2, &__app);
		if (FAILED(err))
			return false;
	}

	//window is created here, can do sound setup
	if (!tpmn_app_init(__app))
		return -1;

	// Pass control to DXUT for handling the message pump and 
	// dispatching render calls. DXUT will call your FrameMove 
	// and FrameRender callback when there is idle time between handling window messages.
	dx9::main_loop(&__app);

	// Perform any application-level cleanup here. Direct3D device resources are released within the
	// appropriate callback functions and therefore don't require any cleanup code here.
	tpmn_app_shutdown(__app);

	return dx9::state.m_exit_code;
}
