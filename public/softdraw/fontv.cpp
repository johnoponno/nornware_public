//fontv.h

//variable width fonts

//source files must be even width, and fontheight * 6 in height,
//meaning 6 lines of characters
//the supported ASCII characters are 33 to 128

#include "stdafx.h"
#include "fontv.h"

namespace softdraw
{
#define CHARSPERHEIGHT 6

	fontv_t::fontv_t(const int32_t aCharSpacing)
	{
		::memset(&characters, 0, sizeof(characters));
		height = 0;
		char_spacing = aCharSpacing;
		space_width = 0;
	}

	void fontv_print(const fontv_t& FONT, const fontv_blit_mode_t aMode, const int32_t aDstX, const int32_t aDstY, const char* message, sd_bitmap_t& aDst)
	{
		if (!message)
			return;

		const int32_t length = (int32_t)::strlen(message);
		if (!length)
			return;

		int32_t x = aDstX;
		int32_t y = aDstY;
		char ch;

		//draw characters
		for (int32_t i = 0; i < length; ++i)
		{
			ch = message[i];

			//space
			if (ch == ' ')
			{
				x += FONT.space_width + FONT.char_spacing;
			}
			//end line
			else if (ch == '\n')
			{
				x = aDstX;
				y += FONT.height;
			}
			//printable chars
			else if (ch >= fontv_charbegin && ch <= fontv_charend)
			{
				ch -= fontv_charbegin;

				switch (aMode)
				{
				case fontv_blit_mode_t::normal:
					sd_bitmap_blit_clip(FONT.rep, aDst, nullptr, x, y, FONT.characters[ch].w, FONT.characters[ch].h, FONT.characters[ch].s, FONT.characters[ch].t);
					break;

				case fontv_blit_mode_t::key:
					sd_bitmap_blit_key_clip(FONT.rep, aDst, nullptr, x, y, FONT.characters[ch].w, FONT.characters[ch].h, FONT.characters[ch].s, FONT.characters[ch].t);
					break;

				case fontv_blit_mode_t::add:
					sd_bitmap_blit_add_key_clip(FONT.rep, aDst, nullptr, x, y, FONT.characters[ch].w, FONT.characters[ch].h, FONT.characters[ch].s, FONT.characters[ch].t);
					break;

				case fontv_blit_mode_t::sub:
					sd_bitmap_blit_sub_key_clip(FONT.rep, aDst, nullptr, x, y, FONT.characters[ch].w, FONT.characters[ch].h, FONT.characters[ch].s, FONT.characters[ch].t);
					break;
				}

				x += FONT.characters[ch].w + FONT.char_spacing;
			}
		}
	}

	void fontv_print_color_not_black(const fontv_t& FONT, const uint16_t aColor, const int32_t aDstX, const int32_t aDstY, const char* message, sd_bitmap_t& aDst)
	{
		if (!message)
			return;

		const int32_t length = (int32_t)::strlen(message);
		if (!length)
			return;

		int32_t x = aDstX;
		int32_t y = aDstY;
		char ch;

		//draw characters
		for (int32_t i = 0; i < length; ++i)
		{
			ch = message[i];

			//space
			if (ch == ' ')
			{
				x += FONT.space_width + FONT.char_spacing;
			}
			//end line
			else if (ch == '\n')
			{
				x = aDstX;
				y += FONT.height;
			}
			//printable chars
			else if (ch >= fontv_charbegin && ch <= fontv_charend)
			{
				ch -= fontv_charbegin;

				sd_bitmap_blit_key_color_not_black_clip(FONT.rep, aDst, aColor, nullptr, x, y, FONT.characters[ch].w, FONT.characters[ch].h, FONT.characters[ch].s, FONT.characters[ch].t);

				x += FONT.characters[ch].w + FONT.char_spacing;
			}
		}
	}

	int32_t fontv_string_width(const fontv_t& f, const char* aString)
	{
		int32_t length = (int32_t)::strlen(aString);
		int32_t result = 0;
		int32_t index = 0;
		char chr;

		while (index < length)
		{
			chr = aString[index];
			if (chr == '\t')
			{
				result += f.space_width;
			}
			else if (chr == '\n')
			{
				return result;
			}
			else
			{
				assert(chr >= 32 && chr <= 128);
				if (chr == ' ')
					result += f.space_width;
				else
					result += f.characters[chr - fontv_charbegin].w + f.char_spacing;
			}
			++index;
		}

		return result;
	}

	void fontv_print(const fontv_t& f, const int32_t aX, const int32_t aY, const char* aText, sd_bitmap_t& aCanvas)
	{
		fontv_print(f, fontv_blit_mode_t::key, aX, aY, aText, aCanvas);
	}

	bool fontv_load_24(fontv_t& font, const char* aFileName, const bool aBlackKeyFlag)
	{
		if (!sd_bitmap_load_24(aFileName, font.rep))
			return false;

		if (aBlackKeyFlag)
			font.rep.key = 0;

		font.height = font.rep.height / CHARSPERHEIGHT;

		//loop all chars and set txas
		int32_t ch = 0;
		int32_t y = 0;
		while (ch < fontv_numchars && y < CHARSPERHEIGHT)
		{
			//set pos at start of line
			int32_t pos_x = 0;
			int32_t pos_y = y * font.height;
			int32_t char_started = false;

			//loop width
			while (pos_x < font.rep.width)
			{
				//line is clear until proven otherwise
				bool line_clear = true;

				//loop line height
				for (int32_t iny = 0; iny < font.height; ++iny)
				{
					const uint16_t* ptr = font.rep.pixels + pos_x + (pos_y + iny) * font.rep.width;

					if (char_started)
					{
						if (*ptr != font.rep.key)
						{
							line_clear = false;
						}
					}
					else
					{
						if (*ptr != font.rep.key)
						{
							font.characters[ch].s = pos_x;
							char_started = true;
							line_clear = false;
						}
					}
				}

				//check if found end of char
				if (char_started && line_clear)
				{
					font.characters[ch].t = pos_y;
					font.characters[ch].w = pos_x - font.characters[ch].s;
					font.characters[ch].h = font.height;
					char_started = false;
					++ch;	//next char
				}

				//next pixel in x
				++pos_x;

				//if a char was started and we went off the edge of the bitmap,
				//add another char
				if (char_started && pos_x >= font.rep.width)
				{
					font.characters[ch].t = pos_y;
					font.characters[ch].w = pos_x - font.characters[ch].s;
					font.characters[ch].h = font.height;
					char_started = false;

					assert(font.characters[ch].s >= 0 && font.characters[ch].s < font.rep.width);
					assert(font.characters[ch].t >= 0 && font.characters[ch].t < font.rep.height);
					assert(font.characters[ch].w >= 0 && font.characters[ch].w < font.rep.width);
					assert(font.characters[ch].h >= 0 && font.characters[ch].h < font.rep.width);

					++ch;	//next char
				}
			}

			//next row
			++y;
		}

		//space width should be same as '_'
		font.space_width = font.characters['_' - fontv_charbegin].w;

		return true;
	}
}