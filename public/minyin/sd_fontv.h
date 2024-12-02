//fontv.h

//variable width fonts

//source files must be even width, and fontheight * 6 in height,
//meaning 6 lines of characters
//the supported ASCII characters are 33 to 128

#pragma once

#include "sd_bitmap.h"

#define SD_FONTV_CHARBEGIN 33
#define SD_FONTV_CHAREND 128

enum struct sd_fontv_blit_mode_t
{
	NORMAL,
	KEY,
	ADD,
	SUB,
};

struct sd_fontv_t
{
	explicit sd_fontv_t(const int32_t aCharSpacing = 1);

	sd_bitmap_t rep;
	struct
	{
		int32_t s;
		int32_t t;
		int32_t w;
		int32_t h;
	} characters[SD_FONTV_CHAREND - SD_FONTV_CHARBEGIN];
	int32_t height;
	int32_t char_spacing;
	int32_t space_width;	//same as '_'
};

bool sd_fontv_load_24(sd_fontv_t& font, const char* aFileName, const bool aBlackChromaFlag = true);

void sd_fontv_print(const sd_fontv_t& f, const int32_t aX, const int32_t aY, const char* aText, sd_bitmap_t& aCanvas);
void sd_fontv_print(const sd_fontv_t& FONT, const sd_fontv_blit_mode_t aMode, const int32_t aDstX, const int32_t aDstY, const char* message, sd_bitmap_t& aDst);
void sd_fontv_print_color_not_black(const sd_fontv_t& FONT, const uint16_t aColor, const int32_t aDstX, const int32_t aDstY, const char* message, sd_bitmap_t& aDst);
int32_t sd_fontv_string_width(const sd_fontv_t& f, const char* aString);
