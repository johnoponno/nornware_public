#pragma once

//BEGIN PUBLIC INTERFACE FOR MICRON IMPLEMENTORS
//BEGIN PUBLIC INTERFACE FOR MICRON IMPLEMENTORS
//BEGIN PUBLIC INTERFACE FOR MICRON IMPLEMENTORS
//BEGIN PUBLIC INTERFACE FOR MICRON IMPLEMENTORS

//FIXME: this is game specific!
#define MICRON_CANVAS_WIDTH 600
#define MICRON_CANVAS_HEIGHT 320

//FIXME: this is currently win32 VK_xxx codes and NOT platform agnostic!
#define MICRON_KEY_ESCAPE 0x1B

/*
micron (c) 2025 nornware AB

The idea is a minimalist DATA-ONLY input / output API for writing 8-bit / palettized games.

BASE CONTRACT: Getting the 8-bit framebuffer / canvas to the screen "somehow" using the supplied palette (which maps 8 bit pixel indices to 24-bit colors).
BASE CONTRACT: Supplying screen and canvas mouse cursor coordinates.
BASE CONTRACT: Mapping platform-specific "key-is-down-right-now" state to the micron key constants (including mouse buttons).

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
	//FIXME: double buffering should be inside the library
	struct
	{
		uint8_t down_current : 1;
		uint8_t down_last : 1;
	} keys[256];

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
	uint8_t canvas[MICRON_CANVAS_WIDTH * MICRON_CANVAS_HEIGHT];

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




//BEGIN UTILITY INTEFACE FOR GAMES WRITTEN FOR MICRON
//BEGIN UTILITY INTEFACE FOR GAMES WRITTEN FOR MICRON
//BEGIN UTILITY INTEFACE FOR GAMES WRITTEN FOR MICRON
//BEGIN UTILITY INTEFACE FOR GAMES WRITTEN FOR MICRON

bool micron_key_is_down(const micron_t& in_micron, const int32_t in_key);
bool micron_key_downflank(const micron_t& in_micron, const int32_t in_key);

void micron_canvas_clear(
	micron_t& out_micron,
	const uint8_t aColor, int32_t aDstX = 0, int32_t aDstY = 0, int32_t aClearWidth = 0, int32_t aClearHeight = 0);
void micron_canvas_horizontal_line(
	micron_t& out_micron,
	int32_t x1, int32_t x2, const int32_t y1, const uint8_t aColor);
void micron_canvas_vertical_line(
	micron_t& out_micron,
	const int32_t x1, int32_t y1, int32_t y2, const uint8_t aColor);
void micron_canvas_cross(
	micron_t& out_micron,
	const int32_t x, const int32_t y, const int32_t aSize, const uint8_t aColor);

//this is intended to allocate all required bitmap memory in a contiguous chunk
//this is possible because we are loading 24-bit .tga files, calculating a best-fit palette, and then remapping these 24-bit images to 8-bit versions
//see paletas_t for the loading and remapping process
struct micron_bitmap_memory_t
{
	void* data;
};

struct micron_bitmap_t
{
	uint8_t* pixels;//this is memory from the contiguous chunk in micron_bitmap_memory_t
	uint16_t width;
	uint16_t height;
};

void micron_blit(
	micron_t& out_micron,
	const micron_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void micron_blit_key(
	micron_t& out_micron,
	const uint8_t in_key, const micron_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void micron_blit_key_clip(
	micron_t& out_micron,
	const uint8_t in_key, const micron_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void micron_blit_key2_colorize(
	micron_t& out_micron,
	const uint8_t in_key, const uint8_t in_key2, const micron_bitmap_t& SRC, const uint8_t aColor, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void micron_blit_key2_colorize_clip(
	micron_t& out_micron,
	const uint8_t in_key, const uint8_t in_key2, const micron_bitmap_t& SRC, const uint8_t aColor, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);

struct micron_font_t
{
	explicit micron_font_t(const int32_t in_char_spacing = 1);

	micron_bitmap_t rep;
	struct
	{
		int32_t s;
		int32_t t;
		int32_t w;
		int32_t h;
	} characters[95];
	int32_t height;
	int32_t char_spacing;
	int32_t space_width;	//same as '_'

private:

	explicit micron_font_t(const micron_font_t& in_other) = delete;
	void operator = (const micron_font_t& in_other) = delete;
};

bool micron_font_init(micron_font_t& out_font, const uint8_t in_key);
int32_t micron_font_string_width(const micron_font_t& f, const char* aString);

void micron_print(
	micron_t& out_micron,
	const uint8_t in_key, const micron_font_t& f, const int32_t aX, const int32_t aY, const char* aText);
void micron_print_colorize(
	micron_t& out_micron,
	const uint8_t in_key, const uint8_t in_key2, const micron_font_t& FONT, const uint8_t aColor, const int32_t aDstX, const int32_t aDstY, const char* message);

//END UTILITY INTEFACE FOR GAMES WRITTEN FOR MICRON
//END UTILITY INTEFACE FOR GAMES WRITTEN FOR MICRON
//END UTILITY INTEFACE FOR GAMES WRITTEN FOR MICRON
//END UTILITY INTEFACE FOR GAMES WRITTEN FOR MICRON
