#include "stdafx.h"

#include "../../win32/win32_d3d9_state.h"
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
	//win32 / d3d9 init
	if (!win32_d3d9_init("Twisted Pair Man Nightmare (c) 2012-2024 nornware AB", true, ::GetSystemMetrics(SM_CXSCREEN) / 2, ::GetSystemMetrics(SM_CYSCREEN) / 2, __app))
		return -1;

	//application-specific init
	if (!__app.init())
		return -1;

	//enter main loop
	win32_d3d9_main_loop(&__app);

	//application-specific shutdown
	__app.shutdown();

	//return internal exit code
	return win32_d3d9_state.m_exit_code;
}
