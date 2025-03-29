#pragma once

struct micron_t;

//BEGIN UTILITY INTEFACE FOR GAMES WRITTEN FOR MICRON
//BEGIN UTILITY INTEFACE FOR GAMES WRITTEN FOR MICRON
//BEGIN UTILITY INTEFACE FOR GAMES WRITTEN FOR MICRON
//BEGIN UTILITY INTEFACE FOR GAMES WRITTEN FOR MICRON

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

//keyboard / mouse keys
bool micron_key_is_down(const micron_t& in_micron, const int32_t in_key);
bool micron_key_upflank(const micron_t& in_micron, const int32_t in_key);
bool micron_key_downflank(const micron_t& in_micron, const int32_t in_key);

//canvas
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
void micron_canvas_atascii_print(
	micron_t& out_micron,
	const void* in_string, const uint8_t in_color_hi, const uint8_t in_color_lo, const int32_t in_x, const int32_t in_y);
void micron_canvas_atascii_char_key(
	micron_t& out_micron,
	const uint8_t in_char, const uint8_t in_color_hi, const int32_t in_x, const int32_t in_y);
void micron_canvas_visualize_palette(
	micron_t& out_micron,
	const int32_t in_magnification);

//bitmaps
void micron_blit(
	micron_t& out_micron,
	const micron_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void micron_blit_clip(
	micron_t& out_micron,
	const micron_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);

void micron_blit_key(
	micron_t& out_micron,
	const uint8_t in_key, const micron_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void micron_blit_key_clip(
	micron_t& out_micron,
	const uint8_t in_key, const micron_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void micron_blit_key_clip_horizontal_mirror(
	micron_t& out_micron,
	const uint8_t in_key, const micron_bitmap_t& in_src, int32_t in_dst_x, int32_t in_dst_y, int32_t in_copy_width = 0, int32_t in_copy_height = 0, int32_t in_src_x = 0, int32_t in_src_y = 0);
void micron_blit_key2_colorize(
	micron_t& out_micron,
	const uint8_t in_key, const uint8_t in_key2, const micron_bitmap_t& SRC, const uint8_t aColor, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void micron_blit_key2_colorize_clip(
	micron_t& out_micron,
	const uint8_t in_key, const uint8_t in_key2, const micron_bitmap_t& SRC, const uint8_t aColor, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);

//bitmap font
bool micron_font_init(micron_font_t& out_font, const uint8_t in_key);
int32_t micron_font_string_width(const micron_font_t& f, const char* aString);
void micron_print(
	micron_t& out_micron,
	const uint8_t in_key, const micron_font_t& f, const int32_t aX, const int32_t aY, const char* aText);
void micron_print_colorize(
	micron_t& out_micron,
	const uint8_t in_key, const uint8_t in_key2, const micron_font_t& FONT, const uint8_t aColor, const int32_t aDstX, const int32_t aDstY, const char* message);

//atascii rom
extern const uint8_t MICRON_ATASCII_BITS[256 * 8];

//END UTILITY INTEFACE FOR GAMES WRITTEN FOR MICRON
//END UTILITY INTEFACE FOR GAMES WRITTEN FOR MICRON
//END UTILITY INTEFACE FOR GAMES WRITTEN FOR MICRON
//END UTILITY INTEFACE FOR GAMES WRITTEN FOR MICRON
