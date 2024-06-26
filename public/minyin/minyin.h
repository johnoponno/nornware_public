#pragma once

#define MINYIN_KEY_ESCAPE 0x1B

struct fs_blob_t;
struct fs_tga_header_t;

struct pixel_t
{
	bool operator < (const pixel_t& in_other) const;
	bool operator >= (const pixel_t& in_other) const;

	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct octamap_t
{
	fs_tga_header_t* header;
	uint8_t* palette;
	uint8_t* pixels;
};

octamap_t octamap_make(const fs_blob_t& blob);

struct minyin_sound_request_t
{
	const char* asset;
	uint32_t id;
};

struct minyin_input_t
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
};

bool minyin_key_is_down(const minyin_input_t& in_minyin, const int32_t in_key);
bool minyin_key_downflank(const minyin_input_t& in_minyin, const int32_t in_key);

struct minyin_bitmap_t
{
	explicit minyin_bitmap_t();
	~minyin_bitmap_t();

	uint8_t* pixels;
	int32_t width;
	int32_t height;

private:

	explicit minyin_bitmap_t(const minyin_bitmap_t& other) = delete;
	void operator = (const minyin_bitmap_t& other) = delete;
};

bool minyin_bitmap_init(minyin_bitmap_t& bm, const int32_t aWidth, const int32_t aHeight);
bool minyin_bitmap_load_8(minyin_bitmap_t& bm, const char* aFileName);
void minyin_bitmap_relinquish(minyin_bitmap_t& bm);

void minyin_clear(minyin_bitmap_t& bm, const uint8_t aColor, int32_t aDstX = 0, int32_t aDstY = 0, int32_t aClearWidth = 0, int32_t aClearHeight = 0);
void minyin_h_line(minyin_bitmap_t& bm, int32_t x1, int32_t x2, const int32_t y1, const uint8_t aColor);
void minyin_v_line(minyin_bitmap_t& bm, const int32_t x1, int32_t y1, int32_t y2, const uint8_t aColor);
void minyin_cross(minyin_bitmap_t& bm, const int32_t x, const int32_t y, const int32_t aSize, const uint8_t aColor);

void minyin_blit(minyin_bitmap_t& aDst, const minyin_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void minyin_blit_key(minyin_bitmap_t& aDst, const uint8_t in_key, const minyin_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void minyin_blit_key_clip(minyin_bitmap_t& aDst, const uint8_t in_key, const minyin_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void minyin_blit_key2_colorize(minyin_bitmap_t& aDst, const uint8_t in_key, const uint8_t in_key2, const minyin_bitmap_t& SRC, const uint8_t aColor, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void minyin_blit_key2_colorize_clip(minyin_bitmap_t& aDst, const uint8_t in_key, const uint8_t in_key2, const minyin_bitmap_t& SRC, const uint8_t aColor, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);

struct minyin_font_t
{
	explicit minyin_font_t(const int32_t in_char_spacing = 1);

	minyin_bitmap_t rep;
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

	explicit minyin_font_t(const minyin_font_t& other) = delete;
	void operator = (const minyin_font_t& other) = delete;
};

bool minyin_font_init(minyin_font_t& out_font, const uint8_t in_key);
int32_t minyin_font_string_width(const minyin_font_t& f, const char* aString);

void minyin_print(minyin_bitmap_t& aDst, const uint8_t in_key, const minyin_font_t& f, const int32_t aX, const int32_t aY, const char* aText);
void minyin_print_colorize(minyin_bitmap_t& aDst, const uint8_t in_key, const uint8_t in_key2, const minyin_font_t& FONT, const uint8_t aColor, const int32_t aDstX, const int32_t aDstY, const char* message);

extern uint16_t minyin_palette[256];
