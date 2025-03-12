#pragma once

#define MICRON_WIDTH 600
#define MICRON_HEIGHT 320
#define MICRON_KEY_ESCAPE 0x1B

struct micron_t
{
	struct
	{
		uint8_t down_current : 1;
		uint8_t down_last : 1;
	} keys[256];

	int32_t screen_cursor_x;
	int32_t screen_cursor_y;
	int32_t canvas_cursor_x;
	int32_t canvas_cursor_y;

	struct
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
	} palette[256];
	uint8_t canvas[MICRON_WIDTH * MICRON_HEIGHT];
};

bool micron_key_is_down(const micron_t& in_micron, const int32_t in_key);
bool micron_key_downflank(const micron_t& in_micron, const int32_t in_key);

struct micron_bitmap_t
{
	explicit micron_bitmap_t();
	~micron_bitmap_t();

	uint8_t* pixels;
	int32_t width;
	int32_t height;

private:

	explicit micron_bitmap_t(const micron_bitmap_t& in_other) = delete;
	void operator = (const micron_bitmap_t& in_other) = delete;
};

bool minyin_bitmap_init(micron_bitmap_t& bm, const int32_t aWidth, const int32_t aHeight);
void minyin_bitmap_relinquish(micron_bitmap_t& bm);

void minyin_clear(
	micron_t& out_micron,
	const uint8_t aColor, int32_t aDstX = 0, int32_t aDstY = 0, int32_t aClearWidth = 0, int32_t aClearHeight = 0);
void minyin_h_line(
	micron_t& out_micron,
	int32_t x1, int32_t x2, const int32_t y1, const uint8_t aColor);
void minyin_v_line(
	micron_t& out_micron,
	const int32_t x1, int32_t y1, int32_t y2, const uint8_t aColor);
void minyin_cross(
	micron_t& out_micron,
	const int32_t x, const int32_t y, const int32_t aSize, const uint8_t aColor);
void minyin_blit(
	micron_t& out_micron,
	const micron_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void minyin_blit_key(
	micron_t& out_micron,
	const uint8_t in_key, const micron_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void minyin_blit_key_clip(
	micron_t& out_micron,
	const uint8_t in_key, const micron_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void minyin_blit_key2_colorize(
	micron_t& out_micron,
	const uint8_t in_key, const uint8_t in_key2, const micron_bitmap_t& SRC, const uint8_t aColor, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void minyin_blit_key2_colorize_clip(
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

bool minyin_font_init(micron_font_t& out_font, const uint8_t in_key);
int32_t minyin_font_string_width(const micron_font_t& f, const char* aString);

void minyin_print(
	micron_t& out_micron,
	const uint8_t in_key, const micron_font_t& f, const int32_t aX, const int32_t aY, const char* aText);
void minyin_print_colorize(
	micron_t& out_micron,
	const uint8_t in_key, const uint8_t in_key2, const micron_font_t& FONT, const uint8_t aColor, const int32_t aDstX, const int32_t aDstY, const char* message);
