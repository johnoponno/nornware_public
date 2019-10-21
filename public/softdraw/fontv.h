//fontv.h

//variable width fonts

//source files must be even width, and fontheight * 6 in height,
//meaning 6 lines of characters
//the supported ASCII characters are 33 to 128

#pragma once

#include "bitmap.h"

namespace softdraw
{
	enum struct fontv_blit_mode_t
	{
		normal,
		key,
		add,
		sub,
	};

	enum
	{
		fontv_charbegin = 33,
		fontv_charend = 128,
		fontv_numchars = fontv_charend - fontv_charbegin,
	};

	struct fontv_character_t
	{
		int32_t s;
		int32_t t;
		int32_t w;
		int32_t h;
	};

	struct fontv_t
	{
		//specific interface
		explicit fontv_t(const int32_t aCharSpacing = 1);

		//members
		bitmap_t rep;
		fontv_character_t characters[fontv_numchars];
		int32_t height;
		int32_t char_spacing;
		int32_t space_width;	//same as '_'
	};

	bool fontv_load_24(fontv_t& font, const char* aFileName, const bool aBlackChromaFlag = true);

	void fontv_print(const fontv_t& f, const int32_t aX, const int32_t aY, const char* aText, bitmap_t& aCanvas);
	void fontv_print(const fontv_t& FONT, const fontv_blit_mode_t aMode, const int32_t aDstX, const int32_t aDstY, const char* message, bitmap_t& aDst);
	void fontv_print_color_not_black(const fontv_t& FONT, const uint16_t aColor, const int32_t aDstX, const int32_t aDstY, const char* message, bitmap_t& aDst);
	int32_t fontv_string_width(const fontv_t& f, const char* aString);
}
