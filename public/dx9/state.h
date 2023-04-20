#pragma once

struct win32_d3d9_backbuffer_size_t
{
	uint32_t width;
	uint32_t height;
};

struct win32_d3d9_state_t
{
	explicit win32_d3d9_state_t();
	~win32_d3d9_state_t();

	IDirect3D9* m_d3d;                     // the main D3D object
	IDirect3DDevice9* m_d3d_device;               // the D3D rendering device
	win32_d3d9_enumeration_t* m_d3d_enumeration;          // CD3DEnumeration object
	win32_d3d9_device_settings_t* m_device_settings;   // current device settings
	D3DSURFACE_DESC m_backbuffer_surface_desc;   // back buffer surface description
	D3DCAPS9 m_caps;                    // D3D caps for current device
	struct
	{
		HWND focus;                  // the main app focus window
		HWND device_fullscreen;       // the main app device window in fullscreen mode
		HWND device_windowed;         // the main app device window in windowed mode
	} m_hwnd;
	HMONITOR m_adapter_monitor;          // the monitor of the adapter 
	HMENU m_menu;                       // handle to menu
	win32_d3d9_backbuffer_size_t fullscreen_backbuffer_at_mode_change;
	win32_d3d9_backbuffer_size_t windowed_backbuffer_at_mode_change;
	DWORD m_windowed_style_at_mode_change;  // window style
	WINDOWPLACEMENT m_windowed_placement; // record of windowed HWND position/show state/etc
	bool m_topmost_while_windowed;       // if true, the windowed HWND is topmost 
	bool m_minimized;                  // if true, the HWND is minimized
	bool m_maximized;                  // if true, the HWND is maximized
	bool m_minimized_while_fullscreen;   // if true, the HWND is minimized due to a focus switch away when fullscreen mode
	bool m_ignore_size_change;           // if true, DXUT won't reset the device upon HWND size change
	double m_time;                      // current time in seconds
	float m_elapsed_time;                // time elapsed since last frame
	HINSTANCE m_hinstance;              // handle to the app instance
	struct
	{
		STICKYKEYS sticky;     // StickyKey settings upon startup so they can be restored later
		TOGGLEKEYS toggle;     // ToggleKey settings upon startup so they can be restored later
		FILTERKEYS filter;     // FilterKey settings upon startup so they can be restored later
	} m_startup_keys;
	bool m_clip_cursor_when_fullscreen;   // if true, then DXUT will keep the cursor from going outside the window when full screen
	bool m_show_cursor_when_fullscreen;   // if true, then DXUT will show a cursor when full screen
	bool m_auto_change_adapter;          // if true, then the adapter will automatically change if the window is different monitor
	int32_t m_exit_code;                   // the exit code to be returned to the command line
	struct
	{
		bool called;
		bool created;
	} m_init, m_window, m_device;
	struct
	{
		bool created;       // if true, then DeviceCreated callback has been called (if non-nullptr)
		bool reset;         // if true, then DeviceReset callback has been called (if non-nullptr)
	} m_device_objects;
	struct
	{
		bool device_callback;		// if true, then the framework is inside an app device callback
		bool main_loop;				// if true, then the framework is inside the main loop
		bool size_move;				// if true, app is inside a WM_ENTERSIZEMOVE, FIXME: read but not acted upon!
	} m_inside;
	bool m_active;                     // if true, then the app is the active top level window
	int32_t m_pause_rendering_count;        // pause rendering ref count
	int32_t m_pause_time_count;             // pause time ref count
	bool m_device_lost;                 // if true, then the device is lost and needs to be reset
	char m_device_stats[256];                // device stats (description, device type, etc)
	char m_window_title[256];                // window title
	struct
	{
		uint32_t x;
		uint32_t y;
	} desktop_resolution;
	win32_d3d9_app_i* app;
};

extern ::HWND win32_d3d9_hwnd();
extern bool win32_d3d9_is_windowed();
extern bool win32_d3d9_is_rendering_paused();

//2023-04-20 - these are just commented out, use if needed
//extern vec2f_t win32_d3d9_get_backbuffer_size();
//extern float win32_d3d9_get_backbuffer_aspect_ratio();
//extern void win32_d3d9_toggle_wireframe();
//extern bool win32_d3d9_is_wireframe();

extern win32_d3d9_state_t win32_d3d9_state;
