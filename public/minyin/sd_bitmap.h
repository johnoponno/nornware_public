#pragma once

enum struct sd_operation_t
{
	SET,
	ADD,
	SUB,
	MUL,
	HALF,
};

struct sd_cliprect_t
{
	int32_t x;
	int32_t y;
	int32_t z;
	int32_t w;
};

struct sd_color24_t
{
	uint8_t x;
	uint8_t y;
	uint8_t z;
};

struct sd_bitmap_t
{
	explicit sd_bitmap_t();
	~sd_bitmap_t();

	int32_t width;
	int32_t height;
	uint16_t key;
	uint16_t* pixels;

private:

	explicit sd_bitmap_t(const sd_bitmap_t& other);
	void operator = (const sd_bitmap_t& other);
};

void sd_set_555();
void sd_set_565();
void sd_blend(const sd_operation_t anOperation, const uint16_t aColor, uint16_t* aDst);
uint16_t sd_color_encode(const uint8_t aR, const uint8_t aG, const uint8_t aB);
void sd_add_blend(const uint16_t aOne, const uint16_t aTwo, uint16_t* aDst);
void sd_half_blend(const uint16_t aOne, const uint16_t aTwo, uint16_t* aDst);
void sd_sub_blend(const uint16_t aOne, const uint16_t aTwo, uint16_t* aDst);
void sd_multi_blend(const uint16_t aOne, const uint16_t aTwo, uint16_t* aDst);

bool sd_clip(const sd_bitmap_t& aDst, const sd_cliprect_t* aClipRect, int32_t& aSrcX, int32_t& aSrcY, int32_t& aDstX, int32_t& aDstY, int32_t& aCopyWidth, int32_t& aCopyHeight);

bool sd_bitmap_init(const int32_t aWidth, const int32_t aHeight, const uint16_t aKey, sd_bitmap_t& bm);
void sd_bitmap_relinquish(sd_bitmap_t& bm);
bool sd_bitmap_load_24(const char* aFileName, sd_bitmap_t& bm);
bool sd_bitmap_import_flip_vertical_swap_r_b(const int32_t aWidth, const int32_t aHeight, const sd_color24_t* somePixels, sd_bitmap_t& bm);
bool sd_bitmap_clear_set(sd_bitmap_t& bm, const sd_cliprect_t* aClipRect, const uint16_t aColor, int32_t aDstX = 0, int32_t aDstY = 0, int32_t aClearWidth = 0, int32_t aClearHeight = 0);
void sd_bitmap_cross(sd_bitmap_t& bm, const int32_t x, const int32_t y, const int32_t aSize, const uint16_t aColor);

void sd_bitmap_blit(const sd_bitmap_t& SRC, sd_bitmap_t& aDst, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
bool sd_bitmap_blit_clip(const sd_bitmap_t& SRC, sd_bitmap_t& aDst, const sd_cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void sd_bitmap_blit_key(const sd_bitmap_t& SRC, sd_bitmap_t& aDst, const int32_t aDstX, const int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, const int32_t aSrcX = 0, const int32_t aSrcY = 0);
bool sd_bitmap_blit_key_clip(const sd_bitmap_t& SRC, sd_bitmap_t& aDst, const sd_cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
bool sd_bitmap_blit_key_clip(const sd_bitmap_t& SRC, sd_bitmap_t& aDst, const sd_cliprect_t* aClipRect, const sd_operation_t anOp, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
bool sd_bitmap_blit_key_color_not_black_clip(const sd_bitmap_t& SRC, sd_bitmap_t& aDst, const uint16_t aColor, const sd_cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
bool sd_bitmap_blit_half_key_clip(const sd_bitmap_t& SRC, sd_bitmap_t& aDst, const sd_cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
void sd_bitmap_blit_add(const sd_bitmap_t& SRC, sd_bitmap_t& aDst, const int32_t aDstX, const int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
bool sd_bitmap_blit_add_key_clip(const sd_bitmap_t& SRC, sd_bitmap_t& aDst, const sd_cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);
bool sd_bitmap_blit_sub_key_clip(const sd_bitmap_t& SRC, sd_bitmap_t& aDst, const sd_cliprect_t* aClipRect, int32_t aDstX, int32_t aDstY, int32_t aCopyWidth = 0, int32_t aCopyHeight = 0, int32_t aSrcX = 0, int32_t aSrcY = 0);

extern uint32_t sd_memory_load_current;
extern uint32_t sd_memory_load_high;
extern uint16_t sd_white;
extern uint16_t sd_light_gray;
extern uint16_t sd_gray;
extern uint16_t sd_dark_gray;
extern uint16_t sd_black;
extern uint16_t sd_red;
extern uint16_t sd_dark_red;
extern uint16_t sd_orange;
extern uint16_t sd_green;
extern uint16_t sd_dark_green;
extern uint16_t sd_blue;
extern uint16_t sd_dark_blue;
extern uint16_t sd_light_blue;
extern uint16_t sd_yellow;
extern uint16_t sd_dark_yellow;
extern uint16_t sd_purple;
extern uint16_t sd_that_pink;
