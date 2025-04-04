#pragma once

//BEGIN PUBLIC INTERFACE FOR MICRON IMPLEMENTORS
//BEGIN PUBLIC INTERFACE FOR MICRON IMPLEMENTORS
//BEGIN PUBLIC INTERFACE FOR MICRON IMPLEMENTORS
//BEGIN PUBLIC INTERFACE FOR MICRON IMPLEMENTORS

//FIXME: this is currently win32 VK_xxx codes and NOT platform agnostic!
#define MICRON_KEY_ESCAPE 0x1B
#define MICRON_KEY_RETURN 0x0D
#define MICRON_KEY_LMB 0x01
#define MICRON_KEY_RMB 0x02
#define MICRON_KEY_F1 0x70
#define MICRON_KEY_F2 0x71
#define MICRON_KEY_F3 0x72
#define MICRON_KEY_F4 0x73
#define MICRON_KEY_F5 0x74
#define MICRON_KEY_F6 0x75
#define MICRON_KEY_F7 0x76
#define MICRON_KEY_F8 0x77
#define MICRON_KEY_SHIFT 0x10
#define MICRON_KEY_SPACE 0x20
#define MICRON_KEY_PRIOR 0x21
#define MICRON_KEY_NEXT 0x22
#define MICRON_KEY_END 0x23
#define MICRON_KEY_HOME 0x24
#define MICRON_KEY_LEFT 0x25
#define MICRON_KEY_UP 0x26
#define MICRON_KEY_RIGHT 0x27
#define MICRON_KEY_DOWN 0x28
#define MICRON_KEY_INSERT 0x2D


/*
micron (c) 2025 nornware AB

The idea is a minimalist DATA-ONLY input / output API for writing 8-bit / palettized games.

BASE CONTRACT: Getting the 8-bit framebuffer / canvas to the screen "somehow" using the supplied palette (which maps 8 bit pixel indices to 24-bit colors).
BASE CONTRACT: Supplying screen and canvas mouse cursor coordinates.
BASE CONTRACT: Mapping platform-specific "key-is-down-right-now" state to the micron key constants (including mouse buttons).

OPTIONAL: Gamepad support.
OPTIONAL: Music streaming support.
OPTIONAL: Sound sample playback support.

A small bitmap-manipulation library is emerging, based around "paletas" which takes input 24-bit .tga images (assets) and maps them all to a common /
best-fit 8-bit palette (including memory managemend). Also associated to this are various "blit" operations that can be used to manipulate the resulting
bitmaps and composing the output framebuffer / canvas. This also includes minimal (subset of ASCII) fixed-width font support, which is based on bitmaps
that are formatted in a certain way in order to automatically indentify the mapping of characters to coordinates.

*/

struct micron_t
{
	//INPUT (written by the implementing platform)
	//INPUT (written by the implementing platform)
	//INPUT (written by the implementing platform)
	//INPUT (written by the implementing platform)

	//the "down-right-now" state of the keyboard keys AND mouse keys
	//FIXME: this is currently win32 VK_xxx codes and NOT platform agnostic!
	//FIXME: should double buffering should be inside the library?
	struct
	{
		uint8_t down_current : 1;
		uint8_t down_last : 1;
	} keys[256];

	//gamepad state (modeled after xinput)
	struct
	{
		float left_stick_x;
		uint8_t button_a : 1;
		uint8_t button_b : 1;
		uint8_t button_x : 1;
		uint8_t button_y : 1;
	} pads[4];

	//the position of the mouse cursor in your local / native screen resolution
	int32_t screen_cursor_x;
	int32_t screen_cursor_y;

	//the position of the mouse cursor on the canvas (in case you are doing funky scaling and positioning, since MICRON_CANVAS_WIDTH x MICRON_CANVAS_HEIGHT is pretty small)
	int32_t canvas_cursor_x;
	int32_t canvas_cursor_y;



	//OUTPUT (read by the implementing platform)
	//OUTPUT (read by the implementing platform)
	//OUTPUT (read by the implementing platform)
	//OUTPUT (read by the implementing platform)

	//output: 256-color 24-bit palette for mapping 8-bit canvas contents to 24-bit
	//NOTE: this CAN change on the fly (palette animation), so be robust for that
	struct
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
	} palette[256];

	//the actual pixels / framebuffer, CAN / WILL change on the fly
	//the idea is to map this "somehow" to a modern graphics api (texture? / colored quads? / pixel shader?)
	uint8_t canvas[640 * 360];//max resolution, arbitrary, but basically a wide-screen (16:9) version of 640x480
	int32_t canvas_width;//expected to be set by the game, are signed ints for good interop with various blit functions
	int32_t canvas_height;//expected to be set by the game, are signed ints for good interop with various blit functions

	//the music file/track we currently want to stream
	//it's up to the implementor to handle this (if at all) by loading and caching whatever it needs in order to stream a single music track
	const char* music;

	//each one of these (if the game writes them) represents the request to load a sound asset and have it available for later playback (via id)
	struct sound_load_t
	{
		const char* asset;
		uint32_t id;
	};
	std::vector<sound_load_t> sound_loads;

	//this gets written by the game each tick -> each id is the request to play a sound asset (previously loaded / identified by sound_loads)
	std::vector<uint32_t> sound_plays;
};

//END PUBLIC INTERFACE FOR MICRON IMPLEMENTORS
//END PUBLIC INTERFACE FOR MICRON IMPLEMENTORS
//END PUBLIC INTERFACE FOR MICRON IMPLEMENTORS
//END PUBLIC INTERFACE FOR MICRON IMPLEMENTORS
