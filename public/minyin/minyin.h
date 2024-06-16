#pragma once

struct sd_bitmap_t;
struct sd_fontv_t;

#define MINYIN_KEY_ESCAPE 0x1B

struct minyin_sound_request_t
{
	const char* asset;
	uint32_t id;
};

struct minyin_t
{
	bool key_is_down(const int32_t in_key) const;
	bool key_downflank(const int32_t in_key) const;

	struct
	{
		bool down_current;
		bool down_last;
	} _keys[256];
	int32_t _screen_cursor_x;
	int32_t _screen_cursor_y;
	int32_t _canvas_cursor_x;
	int32_t _canvas_cursor_y;
};

struct minyin_bitmap_t
{
	explicit minyin_bitmap_t();
	~minyin_bitmap_t();

	uint8_t* pixels;
	int32_t width;
	int32_t height;

private:

	explicit minyin_bitmap_t(const minyin_bitmap_t& other);
	void operator = (const minyin_bitmap_t& other);
};

bool minyin_bitmap_init(minyin_bitmap_t& bm, const int32_t aWidth, const int32_t aHeight);
bool minyin_bitmap_load_8(minyin_bitmap_t& bm, const char* aFileName);
void minyin_bitmap_relinquish(minyin_bitmap_t& bm);

void minyin_h_line(minyin_bitmap_t& bm, int32_t x1, int32_t x2, const int32_t y1, const uint8_t aColor);
void minyin_v_line(minyin_bitmap_t& bm, const int32_t x1, int32_t y1, int32_t y2, const uint8_t aColor);
void minyin_cross(minyin_bitmap_t& bm, const int32_t x, const int32_t y, const int32_t aSize, const uint8_t aColor);

void minyin_blit(minyin_bitmap_t& aDst, const minyin_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
//void minyin_blit_add(minyin_bitmap_t& aDst, const sd_bitmap_t& SRC, const int32_t aDstX, const int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void minyin_blit_key(minyin_bitmap_t& aDst, const uint8_t in_key, const minyin_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void minyin_blit_key_clip(minyin_bitmap_t& aDst, const uint8_t in_key, const minyin_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
//void minyin_blit_add_key_clip(minyin_bitmap_t& aDst, const sd_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
//void minyin_blit_half_key_clip(minyin_bitmap_t& aDst, const sd_bitmap_t& SRC, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);

void minyin_print(minyin_bitmap_t& aDst, const sd_fontv_t& f, const int32_t aX, const int32_t aY, const char* aText);
void minyin_print_color_not_black(minyin_bitmap_t& aDst, const sd_fontv_t& FONT, const uint16_t aColor, const int32_t aDstX, const int32_t aDstY, const char* message);

extern uint16_t minyin_palette[256];
