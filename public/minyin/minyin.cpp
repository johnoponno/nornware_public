#include "stdafx.h"
#include "minyin.h"

#include "sd_bitmap.h"
#include "fs.h"

#define CHARSPERHEIGHT 6
#define CHARBEGIN 33
#define CHAREND 128
static_assert(_countof(minyin_font_t::characters) == CHAREND - CHARBEGIN, "wtf!?");

static void __min_max(int32_t& aMin, int32_t& aMax)
{
	if (aMin > aMax)
	{
		const int32_t temp = aMin;
		aMin = aMax;
		aMax = temp;
	}
}

static bool __string_has_extension(const char* in_extension, const char* in_string)
{
	const char* LAST_DOT = ::strrchr(in_string, '.');
	return nullptr != LAST_DOT && 0 == ::strcmp(LAST_DOT + 1, in_extension);
}

//from project ninja - for loading 8 bit .tga files "properly" (with palette support)
static struct octamap_t
{
	fs_tga_header_t* header;
	uint8_t* palette;
	uint8_t* pixels;
} __octamap_make(const fs_blob_t& blob)
{
	octamap_t result{};

	result.header = (fs_tga_header_t*)blob.data;
	assert(0 == result.header->id_length);
	assert(1 == result.header->color_map_type);
	assert(1 == result.header->image_type);
	assert(0 == result.header->color_map_origin);
	assert(256 == result.header->color_map_length);
	assert(24 == result.header->color_map_entry_size);
	assert(768 == (uint32_t)result.header->color_map_length * ((uint32_t)result.header->color_map_entry_size / 8));

	result.palette = (uint8_t*)blob.data + sizeof(fs_tga_header_t);
	assert(result.palette + 768 < (uint8_t*)blob.data + blob.size);

	result.pixels = result.palette + 768;
	assert(result.pixels + result.header->image_spec_width * result.header->image_spec_height < (uint8_t*)blob.data + blob.size);

	return result;
}

static bool __clip(

	//these are legacy, but here in case you would want to clip to some other rect than the whole bitmap
	const int32_t in_cliprect_x,
	const int32_t in_cliprect_y,
	const int32_t in_cliprect_z,
	const int32_t in_cliprect_w,
	//these are legacy, but here in case you would want to clip to some other rect than the whole bitmap

	int32_t& out_src_x, int32_t& out_src_y, int32_t& out_dst_x, int32_t& out_dst_y, int32_t& out_width, int32_t& out_height)
{
	//REJECTION TEST
	if ((out_dst_x + out_width) < in_cliprect_x ||
		out_dst_x >= in_cliprect_z ||
		(out_dst_y + out_height) < in_cliprect_y ||
		out_dst_y >= in_cliprect_w)
	{
		return false;
	}

	int32_t clip;

	//CLIP
	//x
	if (out_dst_x < in_cliprect_x)
	{
		clip = out_dst_x - in_cliprect_x;
		out_src_x -= clip;
		out_width += clip;
		out_dst_x = in_cliprect_x;
	}

	//return if no width
	if (out_width <= 0)
		return false;

	//z
	if ((out_dst_x + out_width) >= in_cliprect_z)
		out_width = in_cliprect_z - out_dst_x;

	//return if no width
	if (out_width <= 0)
		return false;

	//y
	if (out_dst_y < in_cliprect_y)
	{
		clip = out_dst_y - in_cliprect_y;
		out_src_y -= clip;
		out_height += clip;
		out_dst_y = in_cliprect_y;
	}

	//return if no height
	if (out_height <= 0)
		return false;

	//w
	if ((out_dst_y + out_height) >= in_cliprect_w)
		out_height = in_cliprect_w - out_dst_y;

	return out_height > 0;
}

//public
//public
//public
//public

minyin_bitmap_t::minyin_bitmap_t()
{
	pixels = nullptr;
	width = 0;
	height = 0;
}

minyin_bitmap_t::~minyin_bitmap_t()
{
	minyin_bitmap_relinquish(*this);
}

minyin_font_t::minyin_font_t(const int32_t in_char_spacing)
{
	::memset(&characters, 0, sizeof(characters));
	height = 0;
	char_spacing = in_char_spacing;
	space_width = 0;
}

bool minyin_key_is_down(const minyin_input_t& in_minyin, const int32_t in_key)
{
	if (in_key >= 0 && in_key < _countof(in_minyin.keys))
		return in_minyin.keys[in_key].down_current;
	return false;
}

bool minyin_key_downflank(const minyin_input_t& in_minyin, const int32_t in_key)
{
	if (in_key >= 0 && in_key < _countof(in_minyin.keys))
		return in_minyin.keys[in_key].down_current && !in_minyin.keys[in_key].down_last;
	return false;
}

bool minyin_bitmap_init(
	minyin_bitmap_t& out_bitmap,
	const int32_t in_width, const int32_t in_height)
{
	//cleanup
	minyin_bitmap_relinquish(out_bitmap);

	//get new memory
	assert(!out_bitmap.pixels);
	out_bitmap.pixels = new uint8_t[in_width * in_height];
	if (!out_bitmap.pixels)
		return false;

	//store new settings
	out_bitmap.width = in_width;
	out_bitmap.height = in_height;

	return true;
}

bool minyin_bitmap_load_8(
	minyin_bitmap_t& out_bm,
	const char* in_filename)
{
	minyin_bitmap_relinquish(out_bm);

	if (!in_filename || 0 == *in_filename)
		return false;

	if (!__string_has_extension("tga", in_filename))
		return false;

	const fs_blob_t BLOB = fs_file_contents(in_filename);
	if (!BLOB.data || !BLOB.size)
		return false;
	const octamap_t OCTAMAP = __octamap_make(BLOB);

	//FIXME: remove?
	{
		struct pe_t
		{
			uint8_t r;
			uint8_t g;
			uint8_t b;
		};
		for (
			uint32_t i = 0;
			i < 256;
			++i
			)
		{
			const pe_t* PE = (pe_t*)OCTAMAP.palette + i;
			minyin_palette[i] = sd_color_encode(PE->b, PE->g, PE->r);
		}
	}

	bool result = false;
	if (minyin_bitmap_init(out_bm, OCTAMAP.header->image_spec_width, OCTAMAP.header->image_spec_height))
	{
		const uint8_t* SRC = OCTAMAP.pixels + (OCTAMAP.header->image_spec_height - 1) * OCTAMAP.header->image_spec_width;
		uint8_t* dst = out_bm.pixels;

		for (int32_t y = 0; y < OCTAMAP.header->image_spec_height; ++y)
		{
			for (int32_t x = 0; x < OCTAMAP.header->image_spec_width; ++x)
			{
				*dst = *SRC;
				++dst;
				++SRC;
			}
			SRC -= OCTAMAP.header->image_spec_width * 2;
		}

		result = true;
	}
	delete[] BLOB.data;
	return result;
}

void minyin_bitmap_relinquish(minyin_bitmap_t& out_bitmap)
{
	const uint32_t SIZE = out_bitmap.width * out_bitmap.height;
	if (SIZE)
	{
		assert(out_bitmap.width && out_bitmap.height && out_bitmap.pixels && (uint32_t)(out_bitmap.width * out_bitmap.height) == SIZE);
		delete[] out_bitmap.pixels;
		out_bitmap.pixels = nullptr;
	}
	else
	{
		assert(!out_bitmap.width && !out_bitmap.height && !out_bitmap.pixels);
	}

	out_bitmap.width = 0;
	out_bitmap.height = 0;
}

void minyin_cross(
	minyin_bitmap_t& out_bitmap,
	const int32_t in_x, const int32_t in_y, const int32_t in_size, const uint8_t in_color)
{
	minyin_h_line(out_bitmap, in_x - in_size / 2, in_x + in_size / 2 + 1, in_y, in_color);
	minyin_v_line(out_bitmap, in_x, in_y - in_size / 2, in_y + in_size / 2 + 1, in_color);
}

void minyin_h_line(
	minyin_bitmap_t& out_bitmap,
	int32_t in_x1, int32_t in_x2, const int32_t in_y1, const uint8_t in_color)
{
	//clip
	if (in_y1 < 0 || in_y1 >= out_bitmap.height)
		return;

	__min_max(in_x1, in_x2);
	in_x1 = __max(0, in_x1);
	in_x2 = __min(out_bitmap.width, in_x2);
	uint8_t* dst = out_bitmap.pixels + in_x1 + in_y1 * out_bitmap.width;

	for (
		int32_t x = in_x1;
		x < in_x2;
		++x
		)
	{
		assert(dst >= out_bitmap.pixels && dst < out_bitmap.pixels + out_bitmap.width * out_bitmap.height);
		*dst++ = in_color;
	}
}

void minyin_v_line(
	minyin_bitmap_t& out_bitmap,
	const int32_t in_x1, int32_t in_y1, int32_t in_y2, const uint8_t in_color)
{
	//clip
	if (in_x1 < 0 || in_x1 >= out_bitmap.width)
		return;

	__min_max(in_y1, in_y2);
	in_y1 = __max(0, in_y1);
	in_y2 = __min(out_bitmap.height, in_y2);
	uint8_t* dst = out_bitmap.pixels + in_x1 + in_y1 * out_bitmap.width;

	for (
		int32_t y = in_y1;
		y < in_y2;
		++y
		)
	{
		assert(dst >= out_bitmap.pixels && dst < out_bitmap.pixels + out_bitmap.width * out_bitmap.height);
		*dst = in_color;
		dst += out_bitmap.width;
	}
}

void minyin_blit(
	minyin_bitmap_t& out_bitmap,
	const minyin_bitmap_t& in_src, int32_t in_dst_x, int32_t in_dst_y, int32_t in_copy_width, int32_t in_copy_height, int32_t in_src_x, int32_t in_src_y)
{
	if (0 == in_copy_width || in_copy_width > in_src.width)
		in_copy_width = in_src.width;
	if (0 == in_copy_height || in_copy_height > in_src.height)
		in_copy_height = in_src.height;

	const uint8_t* BYTE_SRC = in_src.pixels + in_src_x + in_src_y * in_src.width;
	uint8_t* byte_dst = out_bitmap.pixels + in_dst_x + in_dst_y * out_bitmap.width;

	for (
		int32_t y = 0;
		y < in_copy_height;
		++y
		)
	{
		const uint8_t* BYTE_SCAN_SRC = BYTE_SRC;
		uint8_t* byte_scan_dst = byte_dst;

		for (
			int32_t x = 0;
			x < in_copy_width;
			++x
			)
		{
			assert(BYTE_SRC >= in_src.pixels && BYTE_SRC < (in_src.pixels + in_src.width * in_src.height));
			assert(byte_dst >= out_bitmap.pixels && byte_dst < (out_bitmap.pixels + out_bitmap.width * out_bitmap.height));
			*byte_dst = *BYTE_SRC;
			++byte_dst;
			++BYTE_SRC;
		}

		BYTE_SRC = BYTE_SCAN_SRC + in_src.width;
		byte_dst = byte_scan_dst + out_bitmap.width;
	}
}

void minyin_blit_key(
	minyin_bitmap_t& out_bitmap,
	const uint8_t in_key, const minyin_bitmap_t& in_src, int32_t in_dst_x, int32_t in_dst_y, int32_t in_copy_width, int32_t in_copy_height, int32_t in_src_x, int32_t in_src_y)
{
	if (0 == in_copy_width || in_copy_width > in_src.width)
		in_copy_width = in_src.width;
	if (0 == in_copy_height || in_copy_height > in_src.height)
		in_copy_height = in_src.height;

	const uint8_t* BYTE_SRC = in_src.pixels + in_src_x + in_src_y * in_src.width;
	uint8_t* byte_dst = out_bitmap.pixels + in_dst_x + in_dst_y * out_bitmap.width;

	for (
		int32_t y = 0;
		y < in_copy_height;
		++y
		)
	{
		const uint8_t* BYTE_SCAN_SRC = BYTE_SRC;
		uint8_t* byte_scan_dst = byte_dst;

		for (
			int32_t x = 0;
			x < in_copy_width;
			++x
			)
		{
			assert(BYTE_SRC >= in_src.pixels && BYTE_SRC < (in_src.pixels + in_src.width * in_src.height));
			assert(byte_dst >= out_bitmap.pixels && byte_dst < (out_bitmap.pixels + out_bitmap.width * out_bitmap.height));
			if (in_key != *BYTE_SRC)
				*byte_dst = *BYTE_SRC;
			++byte_dst;
			++BYTE_SRC;
		}

		BYTE_SRC = BYTE_SCAN_SRC + in_src.width;
		byte_dst = byte_scan_dst + out_bitmap.width;
	}
}

void minyin_blit_key_clip(
	minyin_bitmap_t& out_bitmap,
	const uint8_t in_key, const minyin_bitmap_t& in_src, int32_t in_dst_x, int32_t in_dst_y, int32_t in_copy_width, int32_t in_copy_height, int32_t in_src_x, int32_t in_src_y)
{
	if (0 == in_copy_width || in_copy_width > in_src.width)
		in_copy_width = in_src.width;
	if (0 == in_copy_height || in_copy_height > in_src.height)
		in_copy_height = in_src.height;

	if (__clip(

		//these are legacy, but here in case you would want to clip to some other rect than the whole bitmap
		0, 0, out_bitmap.width, out_bitmap.height,
		//these are legacy, but here in case you would want to clip to some other rect than the whole bitmap

		in_src_x, in_src_y, in_dst_x, in_dst_y, in_copy_width, in_copy_height))
	{
		minyin_blit_key(out_bitmap, in_key, in_src, in_dst_x, in_dst_y, in_copy_width, in_copy_height, in_src_x, in_src_y);
	}
}

void minyin_blit_key_color_not_black(
	minyin_bitmap_t& out_bitmap,
	const uint8_t in_key, const minyin_bitmap_t& in_src, const uint8_t in_color, int32_t in_dst_x, int32_t in_dst_y, int32_t in_copy_width, int32_t in_copy_height, int32_t in_src_x, int32_t in_src_y)
{
	if (0 == in_copy_width || in_copy_width > in_src.width)
		in_copy_width = in_src.width;
	if (0 == in_copy_height || in_copy_height > in_src.height)
		in_copy_height = in_src.height;

	const uint8_t* BYTE_SRC = in_src.pixels + in_src_x + in_src_y * in_src.width;
	uint8_t* byte_dst = out_bitmap.pixels + in_dst_x + in_dst_y * out_bitmap.width;

	for (
		int32_t y = 0;
		y < in_copy_height;
		++y
		)
	{
		const uint8_t* BYTE_SCAN_SRC = BYTE_SRC;
		uint8_t* byte_scan_dst = byte_dst;

		for (
			int32_t x = 0;
			x < in_copy_width;
			++x
			)
		{
			assert(BYTE_SRC >= in_src.pixels && BYTE_SRC < (in_src.pixels + in_src.width * in_src.height));
			assert(byte_dst >= out_bitmap.pixels && byte_dst < (out_bitmap.pixels + out_bitmap.width * out_bitmap.height));
			if (in_key != *BYTE_SRC)
			{
				if (0 == *BYTE_SRC)
					*byte_dst = in_color;
				else
					*byte_dst = *BYTE_SRC;
			}
			++byte_dst;
			++BYTE_SRC;
		}

		BYTE_SRC = BYTE_SCAN_SRC + in_src.width;
		byte_dst = byte_scan_dst + out_bitmap.width;
	}
}

void minyin_blit_key_color_not_black_clip(
	minyin_bitmap_t& out_bitmap,
	const uint8_t in_key, const minyin_bitmap_t& in_src, const uint8_t in_color, int32_t in_dst_x, int32_t in_dst_y, int32_t in_copy_width, int32_t in_copy_height, int32_t in_src_x, int32_t in_src_y)
{
	if (0 == in_copy_width || in_copy_width > in_src.width)
		in_copy_width = in_src.width;
	if (0 == in_copy_height || in_copy_height > in_src.height)
		in_copy_height = in_src.height;

	if (__clip(

		//these are legacy, but here in case you would want to clip to some other rect than the whole bitmap
		0, 0, out_bitmap.width, out_bitmap.height,
		//these are legacy, but here in case you would want to clip to some other rect than the whole bitmap

		in_src_x, in_src_y, in_dst_x, in_dst_y, in_copy_width, in_copy_height))
	{
		minyin_blit_key_color_not_black(out_bitmap, in_key, in_src, in_color, in_dst_x, in_dst_y, in_copy_width, in_copy_height, in_src_x, in_src_y);
	}
}


bool minyin_font_load_8(minyin_font_t& out_font, const char* in_filename, const uint8_t in_key)
{
	if (!minyin_bitmap_load_8(out_font.rep, in_filename))
		return false;

	out_font.height = out_font.rep.height / CHARSPERHEIGHT;

	//loop all chars and set txas
	int32_t ch = 0;
	int32_t y = 0;
	while (ch < _countof(out_font.characters) && y < CHARSPERHEIGHT)
	{
		//set pos at start of line
		int32_t pos_x = 0;
		int32_t pos_y = y * out_font.height;
		int32_t char_started = false;

		//loop width
		while (pos_x < out_font.rep.width)
		{
			//line is clear until proven otherwise
			bool line_clear = true;

			//loop line height
			for (
				int32_t iny = 0;
				iny < out_font.height;
				++iny
				)
			{
				const uint8_t* ptr = out_font.rep.pixels + pos_x + (pos_y + iny) * out_font.rep.width;

				if (char_started)
				{
					if (*ptr != in_key)
					{
						line_clear = false;
					}
				}
				else
				{
					if (*ptr != in_key)
					{
						out_font.characters[ch].s = pos_x;
						char_started = true;
						line_clear = false;
					}
				}
			}

			//check if found end of char
			if (char_started && line_clear)
			{
				out_font.characters[ch].t = pos_y;
				out_font.characters[ch].w = pos_x - out_font.characters[ch].s;
				out_font.characters[ch].h = out_font.height;
				char_started = false;
				++ch;	//next char
			}

			//next pixel in x
			++pos_x;

			//if a char was started and we went off the edge of the bitmap,
			//add another char
			if (char_started && pos_x >= out_font.rep.width)
			{
				out_font.characters[ch].t = pos_y;
				out_font.characters[ch].w = pos_x - out_font.characters[ch].s;
				out_font.characters[ch].h = out_font.height;
				char_started = false;

				assert(out_font.characters[ch].s >= 0 && out_font.characters[ch].s < out_font.rep.width);
				assert(out_font.characters[ch].t >= 0 && out_font.characters[ch].t < out_font.rep.height);
				assert(out_font.characters[ch].w >= 0 && out_font.characters[ch].w < out_font.rep.width);
				assert(out_font.characters[ch].h >= 0 && out_font.characters[ch].h < out_font.rep.width);

				++ch;	//next char
			}
		}

		//next row
		++y;
	}

	//space width should be same as '_'
	out_font.space_width = out_font.characters['_' - CHARBEGIN].w;

	return true;
}

int32_t minyin_font_string_width(const minyin_font_t& f, const char* aString)
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
				result += f.characters[chr - CHARBEGIN].w + f.char_spacing;
		}
		++index;
	}

	return result;
}

void minyin_print(
	minyin_bitmap_t& out_bitmap,
	const uint8_t in_key, const minyin_font_t& in_font, const int32_t in_dst_x, const int32_t in_dst_y, const char* in_message)
{
	if (!in_message || !*in_message)
		return;

	const int32_t LENGTH = (int32_t)::strlen(in_message);
	assert(LENGTH);

	int32_t x = in_dst_x;
	int32_t y = in_dst_y;
	char ch;

	//draw characters
	for (
		int32_t i = 0;
		i < LENGTH;
		++i
		)
	{
		ch = in_message[i];

		//space
		if (ch == ' ')
		{
			x += in_font.space_width + in_font.char_spacing;
		}
		//end line
		else if (ch == '\n')
		{
			x = in_dst_x;
			y += in_font.height;
		}
		//printable chars
		else if (ch >= CHARBEGIN && ch <= CHAREND)
		{
			ch -= CHARBEGIN;

			minyin_blit_key_clip(out_bitmap, in_key, in_font.rep, x, y, in_font.characters[ch].w, in_font.characters[ch].h, in_font.characters[ch].s, in_font.characters[ch].t);

			x += in_font.characters[ch].w + in_font.char_spacing;
		}
	}
}

void minyin_print_color_not_black(
	minyin_bitmap_t& out_bitmap,
	const uint8_t in_key, const minyin_font_t& in_font, const uint8_t in_color, const int32_t in_dst_x, const int32_t in_dst_y, const char* in_message)
{
	if (!in_message || !*in_message)
		return;

	const int32_t LENGTH = (int32_t)::strlen(in_message);
	assert(LENGTH);

	int32_t x = in_dst_x;
	int32_t y = in_dst_y;
	char ch;

	//draw characters
	for (
		int32_t i = 0;
		i < LENGTH;
		++i
		)
	{
		ch = in_message[i];

		//space
		if (ch == ' ')
		{
			x += in_font.space_width + in_font.char_spacing;
		}
		//end line
		else if (ch == '\n')
		{
			x = in_dst_x;
			y += in_font.height;
		}
		//printable chars
		else if (ch >= CHARBEGIN && ch <= CHAREND)
		{
			ch -= CHARBEGIN;

			minyin_blit_key_color_not_black_clip(out_bitmap, in_key, in_font.rep, in_color, x, y, in_font.characters[ch].w, in_font.characters[ch].h, in_font.characters[ch].s, in_font.characters[ch].t);

			x += in_font.characters[ch].w + in_font.char_spacing;
		}
	}
}

uint16_t minyin_palette[256];
